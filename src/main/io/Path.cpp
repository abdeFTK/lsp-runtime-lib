/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-runtime-lib
 * Created on: 8 февр. 2019 г.
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

#include <lsp-plug.in/io/Path.h>
#include <lsp-plug.in/io/File.h>
#include <lsp-plug.in/io/Dir.h>
#include <lsp-plug.in/stdlib/string.h>

#if defined(PLATFORM_WINDOWS)
    #include <windows.h>
    #include <shlwapi.h>
#else
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <errno.h>
#endif /* defined(PLATFORM_WINDOWS) */

namespace lsp
{
    namespace io
    {
        Path::Path()
        {
        }
        
        Path::~Path()
        {
        }

        inline void Path::fixup_path()
        {
#ifdef PLATFORM_WINDOWS
            sPath.replace_all('/', FILE_SEPARATOR_C);
#else
            sPath.replace_all('\\', FILE_SEPARATOR_C);
#endif /* PLATFORM_WINDOWS */
        }

        Path *Path::clone() const
        {
            Path *res = new Path();

            if ((res != NULL) && (res->set(this) == STATUS_OK))
                return res;

            delete res;
            return NULL;
        }

        status_t Path::set_native(const char *path, const char *charset)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            if (!sPath.set_native(path, charset))
                return STATUS_NO_MEM;
            fixup_path();
            return STATUS_OK;
        }

        status_t Path::set(const char *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            if (!sPath.set_utf8(path))
                return STATUS_NO_MEM;
            fixup_path();
            return STATUS_OK;
        }

        status_t Path::set(const LSPString *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            if (!sPath.set(path))
                return STATUS_NO_MEM;
            fixup_path();
            return STATUS_OK;
        }

        status_t Path::set(const Path *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            if (!sPath.set(&path->sPath))
                return STATUS_NO_MEM;
            fixup_path();
            return STATUS_OK;
        }

        status_t Path::set(const char *path, const char *child)
        {
            Path tmp;
            status_t res = tmp.set(path);
            if (res == STATUS_OK)
                res = tmp.append_child(child);
            if (res == STATUS_OK)
                swap(&tmp);
            return res;
        }

        status_t Path::set(const char *path, const LSPString *child)
        {
            Path tmp;
            status_t res = tmp.set(path);
            if (res == STATUS_OK)
                res = tmp.append_child(child);
            if (res == STATUS_OK)
                swap(&tmp);
            return res;
        }

        status_t Path::set(const char *path, const Path *child)
        {
            Path tmp;
            status_t res = tmp.set(path);
            if (res == STATUS_OK)
                res = tmp.append_child(child);
            if (res == STATUS_OK)
                swap(&tmp);
            return res;
        }

        status_t Path::set(const LSPString *path, const char *child)
        {
            Path tmp;
            status_t res = tmp.set(path);
            if (res == STATUS_OK)
                res = tmp.append_child(child);
            if (res == STATUS_OK)
                swap(&tmp);
            return res;
        }

        status_t Path::set(const LSPString *path, const LSPString *child)
        {
            Path tmp;
            status_t res = tmp.set(path);
            if (res == STATUS_OK)
                res = tmp.append_child(child);
            if (res == STATUS_OK)
                swap(&tmp);
            return res;
        }

        status_t Path::set(const LSPString *path, const Path *child)
        {
            Path tmp;
            status_t res = tmp.set(path);
            if (res == STATUS_OK)
                res = tmp.append_child(child);
            if (res == STATUS_OK)
                swap(&tmp);
            return res;
        }

        status_t Path::set(const Path *path, const char *child)
        {
            Path tmp;
            status_t res = tmp.set(path);
            if (res == STATUS_OK)
                res = tmp.append_child(child);
            if (res == STATUS_OK)
                swap(&tmp);
            return res;
        }

        status_t Path::set(const Path *path, const LSPString *child)
        {
            Path tmp;
            status_t res = tmp.set(path);
            if (res == STATUS_OK)
                res = tmp.append_child(child);
            if (res == STATUS_OK)
                swap(&tmp);
            return res;
        }

        status_t Path::set(const Path *path, const Path *child)
        {
            Path tmp;
            status_t res = tmp.set(path);
            if (res == STATUS_OK)
                res = tmp.append_child(child);
            if (res == STATUS_OK)
                swap(&tmp);
            return res;
        }

        status_t Path::get(char *path, size_t maxlen) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            const char *utf8 = sPath.get_utf8();
            if (utf8 == NULL)
                return STATUS_NO_MEM;

            size_t len = ::strlen(utf8);
            if (len >= maxlen)
                return STATUS_TOO_BIG;

            ::memcpy(path, utf8, len+1); // Copy including '\0' character
            return STATUS_OK;
        }

        status_t Path::get(LSPString *path) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            return (path->set(&sPath)) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::get(Path *path) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            return (path->sPath.set(&sPath)) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::set_last(const char *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            else if (strlen(path) == 0)
                return remove_last();

            ssize_t len     = sPath.length();
            ssize_t idx     = sPath.rindex_of(FILE_SEPARATOR_C);
            idx             = (idx < 0) ? 0 : idx + 1;
            sPath.set_length(idx);
            if (sPath.append_utf8(path))
            {
                fixup_path();
                return STATUS_OK;
            }

            sPath.set_length(len);
            return STATUS_NO_MEM;
        }

        status_t Path::set_last(const LSPString *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            else if (path->length() <= 0)
                return remove_last();

            ssize_t len     = sPath.length();
            ssize_t idx     = sPath.rindex_of(FILE_SEPARATOR_C);
            idx             = (idx < 0) ? 0 : idx + 1;
            sPath.set_length(idx);
            if (sPath.append(path))
            {
                fixup_path();
                return STATUS_OK;
            }

            sPath.set_length(len);
            return STATUS_NO_MEM;
        }

        status_t Path::set_last(const Path *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            else if (path->sPath.length() <= 0)
                return remove_last();

            ssize_t len     = sPath.length();
            ssize_t idx     = sPath.rindex_of(FILE_SEPARATOR_C);
            idx             = (idx < 0) ? 0 : idx + 1;
            sPath.set_length(idx);
            if (sPath.append(&path->sPath))
            {
                fixup_path();
                return STATUS_OK;
            }

            sPath.set_length(len);
            return STATUS_NO_MEM;
        }

        status_t Path::get_last(char *path, size_t maxlen) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            ssize_t idx = sPath.rindex_of(FILE_SEPARATOR_C);
            idx     = (idx < 0) ? 0 : idx + 1;

            const char *utf8 = sPath.get_utf8(idx);
            if (utf8 == NULL)
                return STATUS_NO_MEM;

            size_t len = ::strlen(utf8);
            if (len >= maxlen)
                return STATUS_TOO_BIG;

            ::memcpy(path, utf8, len + 1);
            return STATUS_OK;
        }

        status_t Path::get_last(LSPString *path) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            ssize_t idx = sPath.rindex_of(FILE_SEPARATOR_C);
            idx     = (idx < 0) ? 0 : idx + 1;

            return (path->set(&sPath, idx)) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::get_last(Path *path) const
        {
            return (path != NULL) ? get_last(&path->sPath) : STATUS_BAD_ARGUMENTS;
        }

        status_t Path::pop_last(char *path, size_t maxlen)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            ssize_t idx = sPath.rindex_of(FILE_SEPARATOR_C);
            idx     = (idx < 0) ? 0 : idx + 1;

            const char *utf8 = sPath.get_utf8(idx);
            if (utf8 == NULL)
                return STATUS_NO_MEM;

            size_t len = ::strlen(utf8);
            if (len >= maxlen)
                return STATUS_TOO_BIG;

            ::memcpy(path, utf8, len + 1);
            sPath.set_length((idx > 0) ? idx - 1 : 0);

            return STATUS_OK;
        }

        status_t Path::pop_last(LSPString *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            ssize_t idx = sPath.rindex_of(FILE_SEPARATOR_C);
            idx     = (idx < 0) ? 0 : idx + 1;

            if (!path->set(&sPath, idx))
                return STATUS_NO_MEM;
            sPath.set_length((idx > 0) ? idx - 1 : 0);

            return STATUS_OK;
        }

        status_t Path::pop_last(Path *path)
        {
            return (path != NULL) ? pop_last(&path->sPath) : STATUS_BAD_ARGUMENTS;
        }

        status_t Path::get_first(char *path, size_t maxlen) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            ssize_t idx         = sPath.index_of(FILE_SEPARATOR_C);
            if (idx < 0)
            {
                if (sPath.is_empty())
                    return STATUS_NOT_FOUND;
                idx         = sPath.length();
            }
            else if (is_absolute())
                idx        += 1;

            const char *utf8    = sPath.get_utf8(0, idx);
            if (utf8 == NULL)
                return STATUS_NO_MEM;

            size_t len = ::strlen(utf8);
            if (len >= maxlen)
                return STATUS_TOO_BIG;

            ::memcpy(path, utf8, len + 1);
            return STATUS_OK;
        }

        status_t Path::get_first(LSPString *path) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            ssize_t idx         = sPath.index_of(FILE_SEPARATOR_C);
            if (idx < 0)
            {
                if (sPath.is_empty())
                    return STATUS_NOT_FOUND;
                idx         = sPath.length();
            }
            else if (is_absolute())
                idx        += 1;

            return (path->set(&sPath, 0, idx)) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::get_first(Path *path) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            ssize_t idx         = sPath.index_of(FILE_SEPARATOR_C);
            if (idx < 0)
                return STATUS_NOT_FOUND;
            if (is_absolute())
                ++idx;

            return path->sPath.set(&sPath, 0, idx);
        }

        status_t Path::pop_first(char *path, size_t maxlen)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            size_t tail;
            ssize_t idx         = sPath.index_of(FILE_SEPARATOR_C);
            if (idx < 0)
            {
                if (sPath.is_empty())
                    return STATUS_NOT_FOUND;
                tail        = sPath.length();
                idx         = tail;
            }
            else
            {
                tail        = (is_absolute()) ? idx + 1 : idx;
                idx        += 1;
            }

            const char *utf8    = sPath.get_utf8(0, tail);
            if (utf8 == NULL)
                return STATUS_NO_MEM;

            size_t len = ::strlen(utf8);
            if (len >= maxlen)
                return STATUS_TOO_BIG;

            ::memcpy(path, utf8, len + 1);
            sPath.remove(0, idx);

            return STATUS_OK;
        }

        status_t Path::pop_first(LSPString *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            size_t tail;
            ssize_t idx         = sPath.index_of(FILE_SEPARATOR_C);
            if (idx < 0)
            {
                if (sPath.is_empty())
                    return STATUS_NOT_FOUND;
                tail        = sPath.length();
                idx         = tail;
            }
            else
            {
                tail        = (is_absolute()) ? idx + 1 : idx;
                idx        += 1;
            }

            if (!path->set(&sPath, 0, tail))
                return STATUS_NO_MEM;

            sPath.remove(0, idx);
            return STATUS_OK;
        }

        status_t Path::pop_first(Path *path)
        {
            return (path != NULL) ? pop_first(&path->sPath) : STATUS_BAD_ARGUMENTS;
        }

        status_t Path::get_ext(char *path, size_t maxlen) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            ssize_t start, next;

            start   = sPath.rindex_of(FILE_SEPARATOR_C);
            start   = (start < 0) ? 0 : start + 1;

            // Lookup for last dot
            if ((next = sPath.index_of(start, '.')) >= 0)
            {
                start       = next+1;
                while ((next = sPath.index_of(start, '.')) >= 0)
                    start       = next+1;
            }
            else
                start   = sPath.length();

            // Copy data to output
            const char *utf8 = sPath.get_utf8(start);
            if (utf8 == NULL)
                return STATUS_NO_MEM;

            size_t len = ::strlen(utf8);
            if (len >= maxlen)
                return STATUS_TOO_BIG;

            ::memcpy(path, utf8, len + 1);
            return STATUS_OK;
        }

        status_t Path::get_ext(LSPString *path) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            ssize_t start, next;

            start   = sPath.rindex_of(FILE_SEPARATOR_C);
            start   = (start < 0) ? 0 : start + 1;

            // Lookup for last dot
            if ((next = sPath.index_of(start, '.')) >= 0)
            {
                start       = next+1;
                while ((next = sPath.index_of(start, '.')) >= 0)
                    start       = next+1;
            }
            else
                start   = sPath.length();

            return (path->set(&sPath, start)) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::get_ext(Path *path) const
        {
            return (path != NULL) ? get_ext(&path->sPath) : STATUS_BAD_ARGUMENTS;
        }

        status_t Path::get_noext(char *path, size_t maxlen) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            ssize_t start, next, end;

            start   = sPath.rindex_of(FILE_SEPARATOR_C);
            start   = (start < 0) ? 0 : start + 1;

            // Lookup for last dot
            end     = sPath.index_of(start, '.');
            if (end >= 0)
            {
                while ((next = sPath.index_of(end + 1, '.')) >= 0)
                    end     = next;
            }
            if (end < 0)
                end = sPath.length();

            // Copy data to output
            const char *utf8 = sPath.get_utf8(start, end);
            if (utf8 == NULL)
                return STATUS_NO_MEM;

            size_t len = ::strlen(utf8);
            if (len >= maxlen)
                return STATUS_TOO_BIG;

            ::memcpy(path, utf8, len + 1);
            return STATUS_OK;
        }

        status_t Path::get_noext(LSPString *path) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            ssize_t start, next, end;

            start   = sPath.rindex_of(FILE_SEPARATOR_C);
            start   = (start < 0) ? 0 : start + 1;

            // Lookup for last dot
            end     = sPath.index_of(start, '.');
            if (end >= 0)
            {
                while ((next = sPath.index_of(end + 1, '.')) >= 0)
                    end     = next;
            }
            if (end < 0)
                end = sPath.length();

            return (path->set(&sPath, start, end)) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::get_noext(Path *path) const
        {
            return (path != NULL) ? get_noext(&path->sPath) : STATUS_BAD_ARGUMENTS;
        }

        status_t Path::get_parent(char *path, size_t maxlen) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            else if (is_root())
                return STATUS_NOT_FOUND;

            ssize_t idx = sPath.rindex_of(FILE_SEPARATOR_C);
            if (idx < 0)
                return STATUS_NOT_FOUND;

            const char *utf8 = sPath.get_utf8(0, idx);
            if (utf8 == NULL)
                return STATUS_NO_MEM;

            size_t len = ::strlen(utf8);
            if (len >= maxlen)
                return STATUS_TOO_BIG;

            ::memcpy(path, utf8, len + 1);
            return STATUS_OK;
        }

        status_t Path::get_parent(LSPString *path) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            else if (is_root())
                return STATUS_NOT_FOUND;

            ssize_t idx = sPath.rindex_of(FILE_SEPARATOR_C);
            if (idx < 0)
                return STATUS_NOT_FOUND;

            return (path->set(&sPath, 0, idx)) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::get_parent(Path *path) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            else if (is_root())
                return STATUS_NOT_FOUND;

            ssize_t idx = sPath.rindex_of(FILE_SEPARATOR_C);
            if (idx < 0)
                return STATUS_NOT_FOUND;

            return (path->sPath.set(&sPath, 0, idx)) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::set_parent(const char *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            else if (is_root())
                return STATUS_BAD_STATE;

            LSPString tmp;
            if (!tmp.set_utf8(path))
                return STATUS_NO_MEM;
            while (tmp.ends_with(FILE_SEPARATOR_C))
                tmp.set_length(tmp.length() - 1);

            bool success = tmp.append(FILE_SEPARATOR_C);
            if (success)
                success = tmp.append(&sPath);
            if (success)
            {
                sPath.swap(&tmp);
                fixup_path();
            }
            return (success) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::set_parent(const LSPString *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            else if (is_root())
                return STATUS_BAD_STATE;

            LSPString tmp;
            if (!tmp.set(path))
                return STATUS_NO_MEM;
            while (tmp.ends_with(FILE_SEPARATOR_C))
                tmp.set_length(tmp.length() - 1);

            bool success = tmp.append(FILE_SEPARATOR_C);
            if (success)
                success = tmp.append(&sPath);
            if (success)
            {
                sPath.swap(&tmp);
                fixup_path();
            }
            return (success) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::set_parent(const Path *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            else if (is_root())
                return STATUS_BAD_STATE;

            LSPString tmp;
            if (!tmp.set(&path->sPath))
                return STATUS_NO_MEM;
            while (tmp.ends_with(FILE_SEPARATOR_C))
                tmp.set_length(tmp.length() - 1);

            bool success = tmp.append(FILE_SEPARATOR_C);
            if (success)
                success = tmp.append(&sPath);
            if (success)
            {
                sPath.swap(&tmp);
                fixup_path();
            }
            return (success) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::concat(const char *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            if (!sPath.append_utf8(path))
                return STATUS_NO_MEM;
            fixup_path();
            return STATUS_OK;
        }

        status_t Path::concat(const LSPString *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            if (!sPath.append(path))
                return STATUS_NO_MEM;
            fixup_path();
            return STATUS_OK;
        }

        status_t Path::concat(const Path *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            if (!sPath.append(&path->sPath))
                return STATUS_NO_MEM;
            fixup_path();
            return STATUS_OK;
        }

        status_t Path::append_child(const char *path)
        {
            Path tmp;
            status_t res = tmp.set(path);
            if (res != STATUS_OK)
                return res;
            else if (tmp.is_empty())
                return STATUS_OK;
            else if (tmp.is_absolute())
                return STATUS_INVALID_VALUE;

            size_t len = sPath.length();
            bool success = ((len <= 0) || (sPath.ends_with(FILE_SEPARATOR_C))) ? true : sPath.append(FILE_SEPARATOR_C);
            if (success)
                success = sPath.append(&tmp.sPath);
            if (success)
                fixup_path();
            else
                sPath.set_length(len);

            return (success) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::append_child(const LSPString *path)
        {
            Path tmp;
            status_t res = tmp.set(path);
            if (res != STATUS_OK)
                return res;
            else if (tmp.is_empty())
                return STATUS_OK;
            else if (tmp.is_absolute())
                return STATUS_INVALID_VALUE;

            size_t len = sPath.length();
            bool success = ((len <= 0) || (sPath.ends_with(FILE_SEPARATOR_C))) ? true : sPath.append(FILE_SEPARATOR_C);
            if (success)
                success = sPath.append(&tmp.sPath);
            if (success)
                fixup_path();
            else
                sPath.set_length(len);

            return (success) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::append_child(const Path *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            else if (path->is_empty())
                return STATUS_OK;
            else if (path->is_absolute())
                return STATUS_INVALID_VALUE;

            size_t len = sPath.length();
            bool success = ((len <= 0) || (sPath.ends_with(FILE_SEPARATOR_C))) ? true : sPath.append(FILE_SEPARATOR_C);
            if (success)
                success = sPath.append(&path->sPath);
            if (success)
                fixup_path();
            else
                sPath.set_length(len);
            return (success) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::append(const char *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            LSPString tmp;
            if (!tmp.set_utf8(path))
                return STATUS_NO_MEM;
            return append(&tmp);
        }

        status_t Path::append(const LSPString *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            if (!sPath.append(path))
                return STATUS_NO_MEM;
            fixup_path();
            return STATUS_OK;
        }

        status_t Path::append(const Path *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            return append(&path->sPath);
        }

        status_t Path::remove_last()
        {
            if (is_root())
                return STATUS_OK;

            ssize_t idx     = sPath.rindex_of(FILE_SEPARATOR_C);
            if (is_relative())
            {
                if (idx < 0)
                    idx             = 0;
                sPath.set_length(idx);
            }
            else if (idx >= 0)
            {
                ssize_t idx2    = (idx > 0) ? sPath.rindex_of(idx - 1, FILE_SEPARATOR_C) : -1;
                if (idx2 < 0)
                    idx             = idx + 1;
                sPath.set_length(idx);
            }
            return STATUS_OK;
        }

        status_t Path::remove_last(char *path, size_t maxlen) const
        {
            Path tmp;
            status_t res = tmp.set(&this->sPath);
            if (res == STATUS_OK)
                res         = tmp.remove_last();
            if (res == STATUS_OK)
                res         = tmp.get(path, maxlen);
            return res;
        }

        status_t Path::remove_last(LSPString *path) const
        {
            Path tmp;
            status_t res = tmp.set(&this->sPath);
            if (res == STATUS_OK)
                res         = tmp.remove_last();
            if (res == STATUS_OK)
                res         = tmp.get(path);
            return res;
        }

        status_t Path::remove_last(Path *path) const
        {
            Path tmp;
            status_t res = tmp.set(&this->sPath);
            if (res == STATUS_OK)
                res         = tmp.remove_last();
            if (res == STATUS_OK)
                res         = tmp.get(path);
            return res;
        }

        status_t Path::remove_first()
        {
            ssize_t idx     = sPath.index_of(FILE_SEPARATOR_C);
            if (is_relative())
            {
                if (idx < 0)
                    idx             = 0;
                return (sPath.remove(0, idx + 1)) ? STATUS_OK : STATUS_NO_MEM;
            }
            else if (idx < 0)
                return STATUS_NOT_FOUND;

            return (sPath.remove(0, idx + 1)) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::remove_first(char *path, size_t maxlen) const
        {
            Path tmp;
            status_t res = tmp.set(&this->sPath);
            if (res == STATUS_OK)
                res         = tmp.remove_first();
            if (res == STATUS_OK)
                res         = tmp.get(path, maxlen);
            return res;
        }

        status_t Path::remove_first(LSPString *path) const
        {
            Path tmp;
            status_t res = tmp.set(&this->sPath);
            if (res == STATUS_OK)
                res         = tmp.remove_first();
            if (res == STATUS_OK)
                res         = tmp.get(path);
            return res;
        }

        status_t Path::remove_first(Path *path) const
        {
            Path tmp;
            status_t res = tmp.set(&this->sPath);
            if (res == STATUS_OK)
                res         = tmp.remove_first();
            if (res == STATUS_OK)
                res         = tmp.get(path);
            return res;
        }

        status_t Path::root()
        {
            if (is_relative())
                return STATUS_BAD_STATE;
            else if (is_root())
                return STATUS_OK;
#if defined(PLATFORM_WINDOWS)
            ssize_t idx = sPath.index_of(FILE_SEPARATOR_C);
            if (idx < 0)
                return STATUS_BAD_STATE;
            sPath.set_length(idx+1);
            return STATUS_OK;
#else
            return (sPath.set(FILE_SEPARATOR_C)) ? STATUS_OK : STATUS_NO_MEM;
#endif
        }

        status_t Path::remove_root()
        {
            if (!is_absolute())
                return STATUS_OK;

            ssize_t idx = sPath.index_of(FILE_SEPARATOR_C);
            if (idx < 0)
            {
                sPath.set_length(0);
                return STATUS_OK;
            }

            return (sPath.remove(idx + 1)) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t Path::parent()
        {
            if (is_root())
                return STATUS_OK;
            ssize_t idx = sPath.rindex_of(FILE_SEPARATOR_C);
            if (idx < 0)
                idx = 0;
            sPath.set_length(idx);
            return STATUS_OK;
        }

        status_t Path::remove_base(const char *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;

            LSPString tmp;
            if (!tmp.set_utf8(path))
                return STATUS_NO_MEM;
            return remove_base(&tmp);
        }

        status_t Path::remove_base(const LSPString *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            if (!sPath.starts_with(path))
                return false;
            size_t index = path->length(), max = sPath.length();
            if (index >= max)
            {
                sPath.clear();
                return STATUS_OK;
            }

            size_t removed = 0;
            while (index < max)
            {
                if (sPath.char_at(index) != FILE_SEPARATOR_C)
                    break;
                ++removed;
                ++index;
            }
            if (removed <= 0)
                return STATUS_INVALID_VALUE;

            LSPString tmp;
            if (!tmp.set(&sPath, index, max))
                return STATUS_NO_MEM;
            sPath.swap(&tmp);
            return STATUS_OK;
        }

        status_t Path::remove_base(const Path *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            return remove_base(&path->sPath);
        }

        status_t Path::remove_base()
        {
            ssize_t idx = sPath.rindex_of(FILE_SEPARATOR_C);
            if (idx < 0)
                return STATUS_OK;
            return (sPath.remove(0, idx + 1)) ? STATUS_OK : STATUS_NO_MEM;
        }

        bool Path::is_absolute() const
        {
            if (sPath.length() <= 0)
                return false;
#if defined(PLATFORM_WINDOWS)
            return !PathIsRelativeW(reinterpret_cast<LPCWSTR>(sPath.get_native_utf16()));
#else
            return (sPath.first() == FILE_SEPARATOR_C);
#endif
        }

        bool Path::is_relative() const
        {
            if (sPath.length() <= 0)
                return true;
#if defined(PLATFORM_WINDOWS)
            return ::PathIsRelativeW(reinterpret_cast<LPCWSTR>(sPath.get_native_utf16()));
#else
            return (sPath.first() != FILE_SEPARATOR_C);
#endif
        }

        bool Path::is_canonical() const
        {
            enum state_t
            {
                S_SEEK,
                S_SEPARATOR,
                S_DOT,
                S_DOTDOT
            };

            if (is_root())
                return true;

            lsp_wchar_t c;
            size_t len              = sPath.length();
            const lsp_wchar_t *p    = sPath.characters();
            const lsp_wchar_t *e    = &p[len];
            state_t state           = S_SEEK;

            while (p < e)
            {
                c  = *p++;

                switch (state)
                {
                    case S_SEEK:
                        if (c == FILE_SEPARATOR_C)
                            state       = S_SEPARATOR;
                        else if (c == '.')
                            state       = S_DOT;
                        break;
                    case S_SEPARATOR:
                        if (c == FILE_SEPARATOR_C)
                            return false;
                        else if (c == '.')
                            state       = S_DOT;
                        else
                            state       = S_SEEK;
                        break;
                    case S_DOT:
                        if (c == FILE_SEPARATOR_C)
                            return false;
                        else if (c == '.')
                            state       = S_DOTDOT;
                        else
                            state       = S_SEEK;
                        break;
                    case S_DOTDOT:
                        if (c == FILE_SEPARATOR_C)
                            return false;
                        else
                            state       = S_SEEK;
                        break;
                }
            }

            return state == S_SEEK;
        }

        bool Path::is_root() const
        {
#if defined(PLATFORM_WINDOWS)
            return ::PathIsRootW(reinterpret_cast<LPCWSTR>(sPath.get_native_utf16()));
#else
            return (sPath.length() == 1) &&
                    (sPath.first() == FILE_SEPARATOR_C);
#endif
        }

        status_t Path::canonicalize()
        {
            enum state_t
            {
                S_SEEK,
                S_SEPARATOR,
                S_DOT,
                S_DOTDOT
            };

            lsp_wchar_t c;
            size_t len              = sPath.length();
            lsp_wchar_t *s          = const_cast<lsp_wchar_t *>(sPath.characters());
            lsp_wchar_t *e          = &s[len];
            state_t state           = S_SEEK;

            if (is_absolute())
            {
                while (*(s++) != FILE_SEPARATOR_C)
                    /* loop */ ;
                state               = S_SEPARATOR;
            }

            lsp_wchar_t *p          = s;
            lsp_wchar_t *w          = s;

            while (p < e)
            {
                c  = *p++;

                switch (state)
                {
                    case S_SEEK:
                        if (c == FILE_SEPARATOR_C)
                        {
                            state       = S_SEPARATOR;
                            *w++        = c;
                        }
                        else if (c == '.')
                            state       = S_DOT;
                        else
                            *w++    = c;
                        break;
                    case S_SEPARATOR:
                        if (c == FILE_SEPARATOR_C)
                            break;
                        else if (c == '.')
                            state       = S_DOT;
                        else
                        {
                            *w++    = c;
                            state   = S_SEEK;
                        }
                        break;
                    case S_DOT:
                        if (c == FILE_SEPARATOR_C)
                            state       = S_SEPARATOR;
                        else if (c == '.')
                            state       = S_DOTDOT;
                        else
                        {
                            *w++        = '.';
                            *w++        = c;
                            state       = S_SEEK;
                        }
                        break;
                    case S_DOTDOT:
                        if (c == FILE_SEPARATOR_C)
                        {
                            state       = S_SEPARATOR;
                            do // Roll-back path
                            {
                                if (w <= s)
                                    break;
                                --w;
                            }
                            while (w[-1] != FILE_SEPARATOR_C);
                        }
                        else
                        {
                            *w++        = '.';
                            *w++        = '.';
                            *w++        = c;
                            state       = S_SEEK;
                        }
                        break;
                }
            }

            while ((w > s) && (w[-1] == FILE_SEPARATOR_C))
                --w;

            sPath.set_length(w - const_cast<lsp_wchar_t *>(sPath.characters()));

            return STATUS_OK;
        }

        status_t Path::get_canonical(char *path, size_t maxlen) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            Path tmp;
            status_t res = tmp.set(&sPath);
            if (res == STATUS_OK)
                res     = tmp.canonicalize();
            if (res == STATUS_OK)
                res     = tmp.get(path, maxlen);
            return res;
        }

        status_t Path::get_canonical(LSPString *path) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            Path tmp;
            status_t res = tmp.set(&sPath);
            if (res == STATUS_OK)
                res     = tmp.canonicalize();
            if (res == STATUS_OK)
                tmp.sPath.swap(path);
            return res;
        }

        status_t Path::get_canonical(Path *path) const
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            Path tmp;
            status_t res = tmp.set(&sPath);
            if (res == STATUS_OK)
                res     = tmp.canonicalize();
            if (res == STATUS_OK)
                tmp.swap(path);
            return res;
        }

        bool Path::equals(const Path *path) const
        {
            return (path != NULL) ? sPath.equals(&path->sPath) : false;
        }

        bool Path::equals(const LSPString *path) const
        {
            return (path != NULL) ? sPath.equals(path) : false;
        }

        bool Path::equals(const char *path) const
        {
            if (path == NULL)
                return false;

            LSPString tmp;
            return (tmp.set_utf8(path)) ? tmp.equals(&sPath) : false;
        }

        status_t Path::stat(fattr_t *attr) const
        {
            return File::stat(&sPath, attr);
        }

        status_t Path::sym_stat(fattr_t *attr) const
        {
            return File::sym_stat(&sPath, attr);
        }

        wssize_t Path::size() const
        {
            fattr_t attr;
            status_t res = File::stat(&sPath, &attr);
            return (res != STATUS_OK) ? attr.size : -res;
        }

        bool Path::exists() const
        {
            fattr_t attr;
            status_t res = File::stat(&sPath, &attr);
            return res == STATUS_OK;
        }

        bool Path::is_reg() const
        {
            fattr_t attr;
            status_t res = File::sym_stat(&sPath, &attr);
            return (res == STATUS_OK) && (attr.type == fattr_t::FT_REGULAR);
        }

        bool Path::is_dir() const
        {
            fattr_t attr;
            status_t res = File::sym_stat(&sPath, &attr);
            return (res == STATUS_OK) && (attr.type == fattr_t::FT_DIRECTORY);
        }

        bool Path::is_block_dev() const
        {
            fattr_t attr;
            status_t res = File::sym_stat(&sPath, &attr);
            return (res == STATUS_OK) && (attr.type == fattr_t::FT_BLOCK);
        }

        bool Path::is_char_dev() const
        {
            fattr_t attr;
            status_t res = File::sym_stat(&sPath, &attr);
            return (res == STATUS_OK) && (attr.type == fattr_t::FT_CHARACTER);
        }

        bool Path::is_fifo() const
        {
            fattr_t attr;
            status_t res = File::sym_stat(&sPath, &attr);
            return (res == STATUS_OK) && (attr.type == fattr_t::FT_FIFO);
        }

        bool Path::is_symlink() const
        {
            fattr_t attr;
            status_t res = File::stat(&sPath, &attr);
            return (res == STATUS_OK) && (attr.type == fattr_t::FT_SYMLINK);
        }

        bool Path::is_socket() const
        {
            fattr_t attr;
            status_t res = File::sym_stat(&sPath, &attr);
            return (res == STATUS_OK) && (attr.type == fattr_t::FT_SOCKET);
        }

        status_t Path::mkdir() const
        {
            return Dir::create(&sPath);
        }

        status_t Path::mkdir(bool recursive) const
        {
            // Try to create directory
            status_t res = Dir::create(&sPath);
            if ((res == STATUS_OK) || (!recursive))
                return res;

            // No success?
            // First, canonicalize path
            Path path;
            path.set(this);
            res = path.canonicalize();
            if (res != STATUS_OK)
                return res;

            // Prepare the loopp
            LSPString tmp;
            ssize_t off = path.sPath.index_of(0, FILE_SEPARATOR_C);
            if (off < 0)
                return STATUS_INVALID_VALUE;
            else if (path.is_absolute())
            {
                off = path.sPath.index_of(off+1, FILE_SEPARATOR_C);
                if (off < 0) // Tried to create root directory?
                    return STATUS_OK;
            }

            // Perform iterative directory creation
            while (off >= 0)
            {
                if (!tmp.set(&path.sPath, 0, off))
                    return STATUS_NO_MEM;

                res = Dir::create(&tmp);
                if (res != STATUS_OK)
                    return res;

                // Lookup for next separator
                off     = path.sPath.index_of(off+1, FILE_SEPARATOR_C);
            }

            return Dir::create(&sPath);
        }

        status_t Path::mkparent() const
        {
            io::Path tmp;
            status_t res;
            if ((res = tmp.set(this)) != STATUS_OK)
                return res;
            if ((res = tmp.remove_last()) != STATUS_OK)
                return res;

            return tmp.mkdir();
        }

        status_t Path::mkparent(bool recursive) const
        {
            io::Path tmp;
            status_t res;
            if ((res = tmp.set(this)) != STATUS_OK)
                return res;
            if ((res = tmp.remove_last()) != STATUS_OK)
                return res;

            return tmp.mkdir(recursive);
        }

        status_t Path::remove() const
        {
            status_t res = File::remove(&sPath);
            if (res == STATUS_IS_DIRECTORY)
                res = Dir::remove(&sPath);
            return (res == STATUS_NOT_DIRECTORY) ? STATUS_IO_ERROR : res;
        }

        status_t Path::rename(const char *dst) const
        {
            return io::File::rename(&sPath, dst);
        }

        status_t Path::rename(const LSPString *dst) const
        {
            return io::File::rename(&sPath, dst);
        }

        status_t Path::rename(const io::Path *dst) const
        {
            return io::File::rename(&sPath, dst);
        }

        void Path::take(LSPString *src)
        {
            sPath.take(src);
            fixup_path();
        }

        status_t Path::current()
        {
            status_t res = Dir::get_current(&sPath);
            if (res == STATUS_OK)
                fixup_path();
            return res;
        }

        ssize_t Path::fmt(const char *fmt...)
        {
            va_list list;
            va_start(list, fmt);
            ssize_t res = sPath.vfmt_utf8(fmt, list);
            va_end(list);
            if (res > 0)
                fixup_path();
            return res;
        }

        ssize_t Path::fmt(const LSPString *fmt...)
        {
            va_list list;
            va_start(list, fmt);
            ssize_t res = sPath.vfmt_utf8(fmt->get_utf8(), list);
            va_end(list);
            if (res > 0)
                fixup_path();
            return res;
        }

        ssize_t Path::vfmt(const char *fmt, va_list args)
        {
            ssize_t res = sPath.vfmt_utf8(fmt, args);
            if (res > 0)
                fixup_path();
            return res;
        }

        ssize_t Path::vfmt(const LSPString *fmt, va_list args)
        {
            ssize_t res = sPath.vfmt_utf8(fmt->get_utf8(), args);
            if (res > 0)
                fixup_path();
            return res;
        }


        bool Path::is_dot() const
        {
            size_t len = sPath.length();
            if (len < 1)
                return false;

            const lsp_wchar_t *wc = sPath.characters();
            if (len == 1)
                return wc[0] == '.';

            return (wc[len-2] == FILE_SEPARATOR_C) &&
                    (wc[len-1] == '.');
        }

        bool Path::is_dot(const io::Path *path)
        {
            return (path != NULL) && (path->is_dot());
        }

        bool Path::is_dot(const LSPString *path)
        {
            if (path == NULL)
                return false;

            size_t len = path->length();
            if (len < 1)
                return false;

            const lsp_wchar_t *wc = path->characters();
            if (len == 1)
                return wc[0] == '.';

            return (wc[len-2] == FILE_SEPARATOR_C) &&
                    (wc[len-1] == '.');
        }

        bool Path::is_dot(const char *path)
        {
            if (path == NULL)
                return false;

            ssize_t len = strlen(path);
            if (len < 1)
                return false;
            else if (len == 1)
                return path[0] == '.';

            path = &path[len - 2];
            return (path[0] == FILE_SEPARATOR_C) &&
                    (path[1] == '.');
        }

        bool Path::is_dotdot() const
        {
            size_t len = sPath.length();
            if (len < 2)
                return false;

            const lsp_wchar_t *wc = sPath.characters();
            if (len == 2)
                return (wc[0] == '.') &&
                        (wc[1] == '.');

            return (wc[len-3] == FILE_SEPARATOR_C) &&
                    (wc[len-2] == '.') &&
                    (wc[len-1] == '.');
        }

        bool Path::is_dotdot(const io::Path *path)
        {
            return (path != NULL) && (path->is_dotdot());
        }

        bool Path::is_dotdot(const LSPString *path)
        {
            if (path == NULL)
                return false;

            size_t len = path->length();
            if (len < 2)
                return false;

            const lsp_wchar_t *wc = path->characters();
            if (len == 2)
                return (wc[0] == '.') &&
                        (wc[1] == '.');

            return (wc[len-3] == FILE_SEPARATOR_C) &&
                    (wc[len-2] == '.') &&
                    (wc[len-1] == '.');
        }

        bool Path::is_dotdot(const char *path)
        {
            if (path == NULL)
                return false;

            ssize_t len = strlen(path);
            if (len < 2)
                return false;
            else if (len == 2)
                return (path[0] == '.') &&
                        (path[1] == '.');

            return (path[len-3] == FILE_SEPARATOR_C) &&
                    (path[len-2] == '.') &&
                    (path[len-1] == '.');
        }

        bool Path::is_dots() const
        {
            ssize_t len = sPath.length();
            if ((len--) <= 0)
                return false;

            const lsp_wchar_t *wc = sPath.characters();
            if (wc[len] != '.')
                return false;
            if ((len--) <= 0)
                return true;

            if (wc[len] == FILE_SEPARATOR_C)
                return true;
            else if (wc[len] != '.')
                return false;
            if ((len--) <= 0)
                return true;

            return wc[len] == FILE_SEPARATOR_C;
        }

        bool Path::is_dots(const Path *path)
        {
            return (path != NULL) && (path->is_dots());
        }

        bool Path::is_dots(const char *path)
        {
            if (path == NULL)
                return false;

            ssize_t len = strlen(path);
            if ((len--) <= 0)
                return false;

            if (path[len] != '.')
                return false;
            if ((len--) <= 0)
                return true;

            if (path[len] == FILE_SEPARATOR_C)
                return true;
            else if (path[len] != '.')
                return false;
            if ((len--) <= 0)
                return true;

            return path[len] == FILE_SEPARATOR_C;
        }

        bool Path::is_dots(const LSPString *path)
        {
            if (path == NULL)
                return false;

            ssize_t len = path->length();
            if ((len--) <= 0)
                return false;

            const lsp_wchar_t *wc = path->characters();
            if (wc[len] != '.')
                return false;
            if ((len--) <= 0)
                return true;

            if (wc[len] == FILE_SEPARATOR_C)
                return true;
            else if (wc[len] != '.')
                return false;
            if ((len--) <= 0)
                return true;

            return wc[len] == FILE_SEPARATOR_C;
        }

        bool Path::valid_file_name(const LSPString *fname)
        {
            if (fname == NULL)
                return false;
            size_t n = fname->length();
            if (n <= 0)
                return false;

            const lsp_wchar_t *chars = fname->characters();
            for (size_t i=0; i<n; ++i)
            {
                lsp_wchar_t ch = *(chars++);
                if ((ch == '*') || (ch == '?'))
                    return false;
                if ((ch == FILE_SEPARATOR_C) || (ch == '\0'))
                    return false;
                #ifdef PLATFORM_WINDOWS
                if ((ch == '/') || (ch == ':') || (ch== '|') || (ch == '<') || (ch == '>'))
                    return false;
                #endif /* PLATFORM_WINDOWS */
            }

            return true;
        }

        bool Path::valid_path_name(const LSPString *fname)
        {
            if (fname == NULL)
                return false;
            size_t n = fname->length();
            if (n <= 0)
                return false;

            const lsp_wchar_t *chars = fname->characters();
            for (size_t i=0; i<n; ++i)
            {
                lsp_wchar_t ch = *(chars++);
                if ((ch == '*') || (ch == '?'))
                    return false;
                if (ch == '\0')
                    return false;
                #ifdef PLATFORM_WINDOWS
                if ((ch == '/') || (ch== '|') || (ch == '<') || (ch == '>'))
                    return false;
                #endif /* PLATFORM_WINDOWS */
            }

            #ifdef PLATFORM_WINDOWS
            ssize_t semicolon = fname->index_of(':');
            if (semicolon > 0)
            {
                ssize_t next = fname->index_of(semicolon + 1, ':');
                if (next >= 0)
                    return false;
                if (size_t(semicolon + 1) < fname->length()) // Should be "?:\"
                {
                    if (fname->char_at(semicolon + 1) != FILE_SEPARATOR_C)
                        return false;
                }

                // Check disk name
                const lsp_wchar_t *chars = fname->characters();
                while ((semicolon--) > 0)
                {
                    lsp_wchar_t ch = *(chars++);
                    if ((ch >= 'a') && (ch <= 'z'))
                        continue;
                    if ((ch >= 'A') && (ch <= 'Z'))
                        continue;
                    return false;
                }
            }
            else if (semicolon == 0)
                return false;
            #endif /* PLATFORM_WINDOWS */

            return true;
        }

        status_t Path::as_relative(const char *path)
        {
            status_t res;
            io::Path child, base;
            if ((res = base.set(path)) != STATUS_OK)
                return res;
            if ((res = child.set(this)) != STATUS_OK)
                return res;

            if ((res = child.compute_relative(&base)) == STATUS_OK)
                sPath.swap(&child.sPath);
            return res;
        }

        status_t Path::as_relative(const LSPString *path)
        {
            status_t res;
            io::Path child, base;
            if ((res = base.set(path)) != STATUS_OK)
                return res;
            if ((res = child.set(this)) != STATUS_OK)
                return res;

            if ((res = child.compute_relative(&base)) == STATUS_OK)
                sPath.swap(&child.sPath);
            return res;
        }

        status_t Path::as_relative(const Path *path)
        {
            status_t res;
            io::Path child, base;
            if ((res = base.set(path)) != STATUS_OK)
                return res;
            if ((res = child.set(this)) != STATUS_OK)
                return res;

            if ((res = child.compute_relative(&base)) == STATUS_OK)
                sPath.swap(&child.sPath);
            return res;
        }

        status_t Path::compute_relative(Path *base)
        {
            // Canonicalize both paths
            status_t res;
            if ((res = canonicalize()) != STATUS_OK)
                return res;
            if ((res = base->canonicalize()) != STATUS_OK)
                return res;

            // Check that beginning of paths matches
            size_t matched = sPath.match(&base->sPath);
            if (matched <= 0)
                return STATUS_NOT_FOUND;

            // The length matches the base file?
            ssize_t idx1, idx2;
            if (matched == base->length())
            {
                if (sPath.length() == base->length())
                {
                    // Lengths match - clear
                    sPath.clear();
                    return STATUS_OK;
                }
                else if (sPath.char_at(matched) == FILE_SEPARATOR_C)
                {
                    // Just remove file base
                    sPath.remove(0, matched+1);
                    return STATUS_OK;
                }

                // Find last matchind file separator
                idx1 = sPath.rindex_of(matched, FILE_SEPARATOR_C);
                idx2 = base->sPath.rindex_of(matched, FILE_SEPARATOR_C);
                if ((idx1 < 0) || (idx2 != idx1))
                    return STATUS_NOT_FOUND;
            }
            else if (matched == sPath.length())
            {
                if (base->sPath.char_at(matched) != FILE_SEPARATOR_C)
                    return STATUS_NOT_FOUND;

                // All is OK, we're just at the end of the child path
                idx1 = matched - 1;
                idx2 = matched;
            }
            else
            {
                // Find last matchind file separator
                idx1 = sPath.rindex_of(matched, FILE_SEPARATOR_C);
                idx2 = base->sPath.rindex_of(matched, FILE_SEPARATOR_C);
                if ((idx1 < 0) || (idx2 != idx1))
                    return STATUS_NOT_FOUND;
            }

            // Add the necessary amount of '../' references
            LSPString tmp;
            while (true)
            {
                idx2        = base->sPath.index_of(idx2 + 1, FILE_SEPARATOR_C);
                if (!tmp.append_ascii(".." FILE_SEPARATOR_S))
                    return STATUS_NO_MEM;
                if (idx2 < 0)
                    break;
            }

            // Append the rest path
            if (!tmp.append(&sPath, idx1 + 1))
                return STATUS_NO_MEM;
            // Remove the trailing '/' character if present
            if (tmp.ends_with(FILE_SEPARATOR_C))
                tmp.remove_last();

            sPath.swap(&tmp);

            return STATUS_OK;
        }
    }

    namespace lltl
    {
        size_t hash_spec<io::Path>::hash_func(const void *ptr, size_t size)
        {
            return (static_cast<const io::Path *>(ptr))->hash();
        }

        ssize_t compare_spec<io::Path>::cmp_func(const void *a, const void *b, size_t size)
        {
            const io::Path *sa = static_cast<const io::Path *>(a);
            const io::Path *sb = static_cast<const io::Path *>(b);
            return sa->compare_to(sb);
        }

        void *allocator_spec<io::Path>::clone_func(const void *src, size_t size)
        {
            return (static_cast<const io::Path *>(src))->clone();
        }

        void allocator_spec<io::Path>::free_func(void *ptr)
        {
            delete (static_cast<io::Path *>(ptr));
        }
    }
} /* namespace lsp */
