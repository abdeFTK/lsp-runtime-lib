/*
 * MMIOReader.cpp
 *
 *  Created on: 24 апр. 2020 г.
 *      Author: sadko
 */

#include <lsp-plug.in/common/alloc.h>
#include <lsp-plug.in/common/debug.h>
#include <lsp-plug.in/common/endian.h>
#include <lsp-plug.in/mm/sample.h>
#include <private/mm/MMIOReader.h>

#ifndef USE_LIBSNDFILE
namespace lsp
{
    namespace mm
    {
        MMIOReader::MMIOReader()
        {
            hMMIO       = NULL;
            nReadPos    = 0;
            nFrames     = -1;
            bSeekable   = false;
            pFormat     = NULL;

            ::ZeroMemory(&ckRiff, sizeof(MMCKINFO));
            ::ZeroMemory(&ckData, sizeof(MMCKINFO));
        }
        
        MMIOReader::~MMIOReader()
        {
            close();
        }

        status_t MMIOReader::close(status_t code)
        {
            if (hMMIO != NULL)
            {
                ::mmioClose(hMMIO, 0);
                hMMIO       = NULL;
            }
            if (pFormat != NULL)
            {
                ::free(pFormat);
                pFormat     = NULL;
            }

            nFrames     = -1;
            bSeekable   = false;

            ::ZeroMemory(&ckRiff, sizeof(MMCKINFO));
            ::ZeroMemory(&ckData, sizeof(MMCKINFO));

            return code;
        }

        status_t MMIOReader::open(const LSPString *path)
        {
            MMCKINFO        ckIn;
            WAVEFORMATEX    wfe;
            MMRESULT        error;

            nReadPos        = 0;
            nFrames         = -1;
            bSeekable       = false;

            if ((hMMIO = ::mmioOpenW(const_cast<WCHAR *>(path->get_utf16()), NULL, MMIO_ALLOCBUF | MMIO_READ)) == NULL)
                return STATUS_PERMISSION_DENIED;

            // Lookup for RIFF chunk
            if ((error = ::mmioDescend(hMMIO, &ckRiff, NULL, 0)) != 0)
                return close(STATUS_BAD_FORMAT);

            if ((ckRiff.ckid != FOURCC_RIFF) || (ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E')))
                return close(STATUS_BAD_FORMAT);

            // Reset position
            if (::mmioSeek(hMMIO, ckRiff.dwDataOffset + sizeof(FOURCC), SEEK_SET) < 0)
                return close(STATUS_CORRUPTED_FILE);

            // Lookup for format chunk
            ckIn.ckid           = mmioFOURCC('f', 'm', 't', ' ');
            if ((error = ::mmioDescend(hMMIO, &ckIn, &ckRiff, MMIO_FINDCHUNK)) != 0)
                return close(STATUS_BAD_FORMAT);

            // Expect the 'fmt' chunk to be at least as large as sizeof(PCMWAVEFORMAT)
            if (ckIn.cksize < sizeof(PCMWAVEFORMAT))
                return close(STATUS_CORRUPTED_FILE);

            // Read the 'fmt ' chunk
            size_t bytes = ::mmioRead(hMMIO, reinterpret_cast<HPSTR>(&wfe), sizeof(PCMWAVEFORMAT));
            if (bytes != sizeof(PCMWAVEFORMAT))
                return close(STATUS_BAD_FORMAT);

            // Estimate number of bytes to allocate for the format descriptor
            size_t ftag = LE_TO_CPU(wfe.wFormatTag);
            if ((ftag != WAVE_FORMAT_PCM) && (ftag != WAVE_FORMAT_IEEE_FLOAT))
            {
                // Read in length of extra bytes.
                bytes = ::mmioRead(hMMIO, reinterpret_cast<HPSTR>(&wfe.cbSize), sizeof(WORD));
                if (bytes != sizeof(WORD))
                    return close(STATUS_CORRUPTED_FILE);

                // Allocate memory
                size_t alloc    = sizeof(WAVEFORMATEX) + LE_TO_CPU(wfe.cbSize);
                pFormat         = static_cast<WAVEFORMATEX *>(::malloc(align_size(alloc, DEFAULT_ALIGN)));
                if (pFormat == NULL)
                    return close(STATUS_NO_MEM);

                // Return back to 'fmt ' chunk
                if ((error = ::mmioDescend(hMMIO, &ckIn, &ckRiff, MMIO_FINDCHUNK)) != 0)
                    return close(STATUS_CORRUPTED_FILE);

                // Read the whole structure again
                bytes = ::mmioRead(hMMIO, reinterpret_cast<HPSTR>(pFormat), alloc);
                if (bytes != alloc)
                    return close(STATUS_CORRUPTED_FILE);
            }
            else
            {
                // Set size and copy wave format
                wfe.cbSize      = 0;
                pFormat         = static_cast<WAVEFORMATEX *>(memdup(&wfe, sizeof(WAVEFORMATEX)));
                if (pFormat == NULL)
                    return close(STATUS_NO_MEM);
                bSeekable       = true;
            }

            // Reset position
            if (::mmioSeek(hMMIO, ckRiff.dwDataOffset + sizeof(FOURCC), SEEK_SET) < 0)
                return close(STATUS_CORRUPTED_FILE);

            // Lookup for 'fact' chunk, read number of samples
            ckIn.ckid           = mmioFOURCC('f', 'a', 'c', 't');
            if ((error = ::mmioDescend(hMMIO, &ckIn, &ckRiff, MMIO_FINDCHUNK)) == 0)
            {
                if (LE_TO_CPU(ckIn.cksize) >= sizeof(DWORD))
                {
                    DWORD factlen   = 0;
                    size_t bytes = ::mmioRead(hMMIO, reinterpret_cast<HPSTR>(&factlen), sizeof(DWORD));
                    if (bytes != sizeof(DWORD))
                        return close(STATUS_CORRUPTED_FILE);
                    nFrames     = factlen;
                }
            }

            // Reset position
            if (::mmioSeek(hMMIO, ckRiff.dwDataOffset + sizeof(FOURCC), SEEK_SET) < 0)
                return close(STATUS_CORRUPTED_FILE);

            ckData.ckid     = mmioFOURCC('d', 'a', 't', 'a');
            if ((error = ::mmioDescend(hMMIO, &ckData, &ckRiff, MMIO_FINDCHUNK)) != 0)
                return close(STATUS_CORRUPTED_FILE);

            // Estimate number of frames by the other way (PCM allows this)
            if (nFrames < 0)
            {
                if (LE_TO_CPU(wfe.wFormatTag) == WAVE_FORMAT_PCM)
                {
                    size_t fsize    = LE_TO_CPU(wfe.wBitsPerSample) * LE_TO_CPU(wfe.nChannels);
                    if (fsize & 0x07)   // not multiple of 8?
                        return close(STATUS_UNSUPPORTED_FORMAT);
                    fsize >>= 3;        // divide by 8
                    nFrames     = ckData.cksize / fsize;
                    if ((nFrames * fsize) != ckData.cksize)
                        return close(STATUS_CORRUPTED_FILE);
                }
                else if (LE_TO_CPU(wfe.wFormatTag) == WAVE_FORMAT_IEEE_FLOAT)
                {
                    if (LE_TO_CPU(wfe.wBitsPerSample) != sizeof(f32_t)*8)
                        return close(STATUS_UNSUPPORTED_FORMAT);
                }
            }

            return STATUS_OK;
        }

        wssize_t MMIOReader::seek(wsize_t offset)
        {
            if (!bSeekable)
                return -STATUS_NOT_SUPPORTED;

            if (offset > ckData.cksize)
                offset = ckData.cksize;

            if (::mmioSeek(hMMIO, ckData.dwDataOffset + offset, SEEK_SET) < 0)
                return -STATUS_IO_ERROR;

            return nReadPos = offset;
        }

        ssize_t MMIOReader::read(void *buf, size_t count)
        {
            // Compute how many bytes we can read
            wssize_t can_read = ckData.cksize - nReadPos;
            if (can_read <= 0)
                return -STATUS_EOF;
            if (count > size_t(can_read))
                count = can_read;

            // Perform read
            LONG res    = ::mmioRead(hMMIO, reinterpret_cast<HPSTR>(buf), count);
            if (res < 0)
                return -STATUS_IO_ERROR;
            nReadPos   += res;
            return res;
        }
    
    } /* namespace mm */
} /* namespace lsp */
#endif /* USE_LIBSNDFILE */
