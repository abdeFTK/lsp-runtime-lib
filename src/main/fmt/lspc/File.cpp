/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-runtime-lib
 * Created on: 14 янв. 2018 г.
 *
 * lsp-runtime-lib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * lsp-runtime-lib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with lsp-runtime-lib. If not, see <https://www.gnu.org/licenses/>.
 */

#include <lsp-plug.in/common/endian.h>
#include <lsp-plug.in/stdlib/string.h>
#include <lsp-plug.in/fmt/lspc/lspc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <lsp-plug.in/fmt/lspc/File.h>

namespace lsp
{
    namespace lspc
    {
        File::File()
        {
            pFile       = NULL;
            bWrite      = false;
            nHdrSize    = 0;
        }

        File::~File()
        {
            close();
        }

        Resource *File::create_resource(fhandle_t fd)
        {
            Resource *res = new Resource;
            if (res == NULL)
                return NULL;

            res->fd         = fd;
            res->refs       = 1;
            res->bufsize    = 0x10000;
            res->chunk_id   = 0;
            res->length     = 0;

            return res;
        }
    
        status_t File::open(const char *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            LSPString fpath;
            if (!fpath.set_utf8(path))
                return STATUS_NO_MEM;
            return open(&fpath);
        }

        status_t File::open(const io::Path *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            LSPString fpath;
            status_t res = path->get(&fpath);
            if (res == STATUS_OK)
                res  = open(&fpath);
            return res;
        }
    
        status_t File::open(const LSPString *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            if (pFile != NULL)
                return STATUS_BAD_STATE;

            root_header_t hdr;

    #if defined(PLATFORM_WINDOWS)
            fhandle_t fd = CreateFileW(
                    path->get_native_utf16(), GENERIC_READ,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                    NULL
                );
            if (fd == INVALID_HANDLE_VALUE)
                return STATUS_IO_ERROR;

            Resource *res   = create_resource(fd);
            if (res == NULL)
            {
                CloseHandle(fd);
                return STATUS_NO_MEM;
            }
    #else
            fhandle_t fd        = ::open(path->get_utf8(), O_RDONLY);
            if (fd < 0)
                return STATUS_IO_ERROR;

            Resource *res   = create_resource(fd);
            if (res == NULL)
            {
                ::close(fd);
                return STATUS_NO_MEM;
            }
    #endif /* PLATFORM_WINDOWS */

            ssize_t bytes = res->read(0, &hdr, sizeof(root_header_t));

            if ((bytes < ssize_t(sizeof(root_header_t))) ||
                (BE_TO_CPU(hdr.size) < sizeof(root_header_t)) ||
                (BE_TO_CPU(hdr.magic) != LSPC_ROOT_MAGIC) ||
                (BE_TO_CPU(hdr.version) != 1))
            {
                res->release();
                delete res;
                return STATUS_BAD_FORMAT;
            }

            nHdrSize            = BE_TO_CPU(hdr.size);
            pFile               = res;
            bWrite              = false;

            return STATUS_OK;
        }

        status_t File::create(const char *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            LSPString fpath;
            if (!fpath.set_utf8(path))
                return STATUS_NO_MEM;
            return create(&fpath);
        }

        status_t File::create(const io::Path *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            LSPString fpath;
            status_t res = path->get(&fpath);
            if (res == STATUS_OK)
                res = create(&fpath);
            return res;
        }

        status_t File::create(const LSPString *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            if (pFile != NULL)
                return STATUS_BAD_STATE;

    #if defined(PLATFORM_WINDOWS)
            fhandle_t fd = CreateFileW(
                    path->get_native_utf16(), GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                    NULL
                );
            if (fd == INVALID_HANDLE_VALUE)
                return STATUS_IO_ERROR;

            Resource *res   = create_resource(fd);
            if (res == NULL)
            {
                CloseHandle(fd);
                return STATUS_NO_MEM;
            }
    #else
            fhandle_t fd        = ::open(path->get_utf8(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd < 0)
                return STATUS_IO_ERROR;

            Resource *res   = create_resource(fd);
            if (res == NULL)
            {
                ::close(fd);
                return STATUS_NO_MEM;
            }
    #endif /* PLATFORM_WINDOWS */

            root_header_t hdr;
            ::memset(&hdr, 0, sizeof(root_header_t));
            hdr.magic       = LSPC_ROOT_MAGIC;
            hdr.version     = 1;
            hdr.size        = sizeof(hdr);

            hdr.magic       = CPU_TO_BE(hdr.magic);
            hdr.version     = CPU_TO_BE(hdr.version);
            hdr.size        = CPU_TO_BE(hdr.size);

            status_t io_res = res->write(&hdr, sizeof(root_header_t));
            if (io_res != STATUS_OK)
            {
                res->release();
                delete res;
                return io_res;
            }

            res->length         = sizeof(hdr);
            pFile               = res;
            bWrite              = true;

            return STATUS_OK;
        }

        status_t File::close()
        {
            if (pFile == NULL)
                return STATUS_BAD_STATE;
            status_t res = pFile->release();
            if (pFile->refs <= 0)
                delete pFile;
            pFile   = NULL;
            return res;
        }

        ChunkWriter *File::write_chunk(uint32_t magic)
        {
            if ((pFile == NULL) || (!bWrite))
                return NULL;

            ChunkWriter *wr = new ChunkWriter(pFile, magic);
            return wr;
        }

        ChunkReader *File::read_chunk(uint32_t uid)
        {
            if ((pFile == NULL) || (bWrite))
                return NULL;

            // Find the initial position of the chunk in file
            chunk_header_t hdr;
            wsize_t pos         = nHdrSize;
            while (true)
            {
                ssize_t res = pFile->read(pos, &hdr, sizeof(chunk_header_t));
                if (res != sizeof(chunk_header_t))
                    return NULL;
                pos        += sizeof(chunk_header_t);

                hdr.magic   = BE_TO_CPU(hdr.magic);
                hdr.uid     = BE_TO_CPU(hdr.uid);
                hdr.flags   = BE_TO_CPU(hdr.flags);
                hdr.size    = BE_TO_CPU(hdr.size);

    //            lsp_trace("chunk header uid=%x, magic=%x, flags=%x, search_uid=%x, size=%llx",
    //                    int(hdr.uid), int(hdr.magic), int(hdr.flags), int(uid), (long long)(hdr.size));
                if (hdr.uid == uid)
                    break;
                pos        += hdr.size;
            }

            // Create reader
            ChunkReader *rd = new ChunkReader(pFile, hdr.magic, uid);
            if (rd == NULL)
                return NULL;
            rd->nFileOff        = pos;
            rd->nUnread         = hdr.size;
            return rd;
        }

        ChunkReader *File::read_chunk(uint32_t uid, uint32_t magic)
        {
            if ((pFile == NULL) || (bWrite))
                return NULL;

            // Find the initial position of the chunk in file
            chunk_header_t hdr;
            wsize_t pos         = nHdrSize;
            while (true)
            {
                ssize_t res = pFile->read(pos, &hdr, sizeof(chunk_header_t));
                if (res != sizeof(chunk_header_t))
                    return NULL;
                pos        += sizeof(chunk_header_t);

                hdr.magic   = BE_TO_CPU(hdr.magic);
                hdr.uid     = BE_TO_CPU(hdr.uid);
                hdr.flags   = BE_TO_CPU(hdr.flags);
                hdr.size    = BE_TO_CPU(hdr.size);

                if ((hdr.uid == uid) && (hdr.magic == magic))
                    break;
                pos        += hdr.size;
            }

            // Create reader
            ChunkReader *rd = new ChunkReader(pFile, hdr.magic, uid);
            if (rd == NULL)
                return NULL;
            rd->nFileOff        = pos;
            rd->nUnread         = hdr.size;
            return rd;
        }

        ChunkReader *File::find_chunk(uint32_t magic, uint32_t *id, uint32_t start_id)
        {
            if ((pFile == NULL) || (bWrite))
                return NULL;

            // Find the initial position of the chunk in file
            chunk_header_t hdr;
            wsize_t pos         = nHdrSize;
            while (true)
            {
                ssize_t res = pFile->read(pos, &hdr, sizeof(chunk_header_t));
                if (res != sizeof(chunk_header_t))
                    return NULL;
                pos        += sizeof(chunk_header_t);

                hdr.magic   = BE_TO_CPU(hdr.magic);
                hdr.uid     = BE_TO_CPU(hdr.uid);
                hdr.flags   = BE_TO_CPU(hdr.flags);
                hdr.size    = BE_TO_CPU(hdr.size);

                if ((hdr.uid >= start_id) && (hdr.magic == magic))
                    break;
                pos        += hdr.size;
            }

            // Create reader
            ChunkReader *rd = new ChunkReader(pFile, hdr.magic, hdr.uid);
            if (rd == NULL)
                return NULL;
            if (id != NULL)
                *id                 = rd->unique_id();
            rd->nFileOff        = pos;
            rd->nUnread         = hdr.size;
            return rd;
        }
    }
} /* namespace lsp */
