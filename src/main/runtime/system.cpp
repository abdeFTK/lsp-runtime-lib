/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-runtime-lib
 * Created on: 17 мар. 2019 г.
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

#include <lsp-plug.in/runtime/system.h>
#include <lsp-plug.in/io/Dir.h>
#include <lsp-plug.in/ipc/Process.h>

#ifdef PLATFORM_WINDOWS
    #include <shellapi.h>
    #include <synchapi.h>
    #include <sysinfoapi.h>
    #include <winbase.h>
#else
    #include <stdlib.h>
    #include <errno.h>
    #include <time.h>
    #include <sys/time.h>
#endif /* PLATFORM_WINDOWS */

namespace lsp
{
    namespace system
    {
        status_t get_env_var(const LSPString *name, LSPString *dst)
        {
            if (name == NULL)
                return STATUS_BAD_ARGUMENTS;

#ifdef PLATFORM_WINDOWS
            const lsp_utf16_t *nname = name->get_native_utf16();
            if (nname == NULL)
                return STATUS_NO_MEM;

            DWORD bufsize = ::GetEnvironmentVariableW(nname, NULL, 0);
            if (bufsize == 0)
            {
                if (::GetLastError() == ERROR_ENVVAR_NOT_FOUND)
                    return STATUS_NOT_FOUND;
                return STATUS_UNKNOWN_ERR;
            }
            else if (dst == NULL)
                return STATUS_OK;

            lsp_utf16_t *buf = reinterpret_cast<lsp_utf16_t *>(::malloc(bufsize * sizeof(lsp_utf16_t)));
            if (buf == NULL)
                return STATUS_NO_MEM;
            bufsize = ::GetEnvironmentVariableW(nname, buf, bufsize);
            if (bufsize == 0)
            {
                ::free(buf);
                return STATUS_UNKNOWN_ERR;
            }

            bool res = dst->set_native_utf16(buf, bufsize);
            ::free(buf);
            return (res) ? STATUS_OK : STATUS_NO_MEM;
#else
            const char *nname = name->get_native();
            if (nname == NULL)
                return STATUS_NO_MEM;

#ifdef _GNU_SOURCE
            char *var = secure_getenv(nname);
#else
            char *var = getenv(nname);
#endif
            if (var == NULL)
                return STATUS_NOT_FOUND;
            if (dst != NULL)
            {
                if (!dst->set_native(var))
                    return STATUS_NO_MEM;
            }
            return STATUS_OK;
#endif /* PLATFORM_WINDOWS */
        }

        status_t get_env_var(const char *name, LSPString *dst)
        {
            if (name == NULL)
                return STATUS_BAD_ARGUMENTS;
            LSPString sname;
            if (!sname.set_utf8(name))
                return STATUS_NO_MEM;
            return get_env_var(&sname, dst);
        }

        status_t set_env_var(const LSPString *name, const LSPString *value)
        {
#ifdef PLATFORM_WINDOWS
            const lsp_utf16_t *nname = name->get_native_utf16();
            if (nname == NULL)
                return STATUS_NO_MEM;

            if (value != NULL)
            {
                const lsp_utf16_t *nvalue = value->get_native_utf16();
                if (nvalue == NULL)
                    return STATUS_NO_MEM;
                if (::SetEnvironmentVariableW(nname, nvalue))
                    return STATUS_OK;
            }
            else
            {
                if (::SetEnvironmentVariableW(nname, NULL))
                    return STATUS_OK;
            }
            return STATUS_UNKNOWN_ERR;
#else
            const char *nname = name->get_native();
            if (nname == NULL)
                return STATUS_NO_MEM;

            int res;
            if (value != NULL)
            {
                const char *nvalue = value->get_native();
                if (nvalue == NULL)
                    return STATUS_NO_MEM;
                res = ::setenv(nname, nvalue, 1);
            }
            else
                res = ::unsetenv(nname);

            if (res == 0)
                return STATUS_OK;
            switch (res)
            {
                case EINVAL: return STATUS_INVALID_VALUE;
                case ENOMEM: return STATUS_NO_MEM;
                default: break;
            }
            return STATUS_UNKNOWN_ERR;
#endif /* PLATFORM_WINDOWS */
        }

        status_t set_env_var(const char *name, const char *value)
        {
            if (name == NULL)
                return STATUS_BAD_ARGUMENTS;
            LSPString sname;
            if (!sname.set_utf8(name))
                return STATUS_NO_MEM;
            if (value == NULL)
                return set_env_var(&sname, NULL);

            LSPString svalue;
            if (!svalue.set_utf8(value))
                return STATUS_NO_MEM;
            return set_env_var(&sname, &svalue);
        }

        status_t set_env_var(const char *name, const LSPString *value)
        {
            if (name == NULL)
                return STATUS_BAD_ARGUMENTS;
            LSPString sname;
            if (!sname.set_utf8(name))
                return STATUS_NO_MEM;
            return set_env_var(&sname, value);
        }

        status_t remove_env_var(const LSPString *name)
        {
            const char *nname = name->get_native();
            if (nname == NULL)
                return STATUS_NO_MEM;

#ifdef PLATFORM_WINDOWS
            if (SetEnvironmentVariable(nname, NULL))
                return STATUS_OK;
            return STATUS_UNKNOWN_ERR;
#else
            int res = ::unsetenv(nname);
            if (res == 0)
                return STATUS_OK;
            switch (res)
            {
                case EINVAL: return STATUS_INVALID_VALUE;
                case ENOMEM: return STATUS_NO_MEM;
                default: break;
            }
            return STATUS_UNKNOWN_ERR;
#endif /* PLATFORM_WINDOWS */
        }

        status_t remove_env_var(const char *name)
        {
            if (name == NULL)
                return STATUS_BAD_ARGUMENTS;
            LSPString sname;
            if (!sname.set_utf8(name))
                return STATUS_NO_MEM;
            return remove_env_var(&sname);
        }

        status_t get_home_directory(LSPString *homedir)
        {
            if (homedir == NULL)
                return STATUS_BAD_ARGUMENTS;
#ifdef PLATFORM_WINDOWS
            LSPString drv, path;
            status_t res = get_env_var("HOMEDRIVE", &drv);
            if (res != STATUS_OK)
                return res;
            res = get_env_var("HOMEPATH", &path);
            if (res != STATUS_OK)
                return res;
            if (!drv.append(&path))
                return STATUS_NO_MEM;

            homedir->take(&drv);
            return STATUS_OK;
#else
            return get_env_var("HOME", homedir);
#endif /* PLATFORM_WINDOWS */
        }

        status_t get_home_directory(io::Path *homedir)
        {
            if (homedir == NULL)
                return STATUS_BAD_ARGUMENTS;
            LSPString path;
            status_t res = get_home_directory(&path);
            if (res != STATUS_OK)
                return res;
            return homedir->set(&path);
        }

        status_t get_user_config_path(LSPString *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            LSPString upath;

#ifdef PLATFORM_WINDOWS
            status_t res = get_env_var("LOCALAPPDATA", &upath);
            if (res != STATUS_OK)
            {
                res = get_env_var("APPDATA", &upath);
                if (res != STATUS_OK)
                {
                    res = get_home_directory(&upath);
                    if (res != STATUS_OK)
                        return res;
                    if (!upath.append_ascii(FILE_SEPARATOR_S ".config"))
                        return STATUS_NO_MEM;
                }
            }
#else
            status_t res = get_env_var("HOME", &upath);
            if (res != STATUS_OK)
                return res;
            if (!upath.append_ascii(FILE_SEPARATOR_S ".config"))
                return STATUS_NO_MEM;
#endif /* PLATFORM_WINDOWS */

            path->swap(&upath);
            return STATUS_OK;
        }

        status_t get_user_config_path(io::Path *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            LSPString upath;
            status_t res = get_user_config_path(&upath);
            if (res != STATUS_OK)
                return res;
            return path->set(&upath);
        }

#ifdef PLATFORM_WINDOWS
        void get_time(time_t *time)
        {
            FILETIME t;
            ::GetSystemTimeAsFileTime(&t);
            uint64_t itime  = (uint64_t(t.dwHighDateTime) << 32) | t.dwLowDateTime;

            time->seconds   = itime / 10000000;
            time->nanos     = (itime % 10000000) * 100;
        }

        void get_localtime(localtime_t *local, const time_t *time)
        {
            SYSTEMTIME t;

            // Get system file time into tmp
            if (time != NULL)
            {
                FILETIME tmp, stime;
                uint64_t itime      = (time->seconds * 10000000) | (time->nanos / 100);
                tmp.dwHighDateTime  = DWORD(itime >> 32);
                tmp.dwLowDateTime   = DWORD(itime);

                ::FileTimeToLocalFileTime(&tmp, &stime);
                ::FileTimeToSystemTime(&stime, &t);
            }
            else
                ::GetLocalTime(&t);

            // Decode
            local->year     = t.wYear;
            local->month    = t.wMonth;
            local->mday     = t.wDay;
            local->wday     = t.wDayOfWeek;
            local->hour     = t.wHour;
            local->min      = t.wMinute;
            local->sec      = t.wSecond;
            local->nanos    = time->nanos;
        }

        status_t sleep_msec(size_t delay)
        {
            while (delay > 0)
            {
                size_t period   = lsp_min(0x10000000, delay);
                Sleep(DWORD(period));
                delay -= period;
            }

            return STATUS_OK;
        }
#else
        void get_time(time_t *time)
        {
            struct timespec t;
            ::clock_gettime(CLOCK_REALTIME, &t);

            time->seconds   = t.tv_sec;
            time->nanos     = t.tv_nsec;
        }

        void get_localtime(localtime_t *local, const time_t *time)
        {
            // Store actual time to timespec struct
            struct timespec stime;

            if (time != NULL)
            {
                stime.tv_sec        = time->seconds;
                stime.tv_nsec       = time->nanos;
            }
            else
                ::clock_gettime(CLOCK_REALTIME, &stime);

            // Now convert the struct to local time
            struct tm *t;
        #if defined(_POSIX_C_SOURCE) || defined(_BSD_SOURCE) || defined(_SVID_SOURCE)
            struct tm ctime;
            t               = localtime_r(&stime.tv_sec, &ctime);
        #else
            t               = localtime(&stime.tv_sec);
        #endif

            local->year     = t->tm_year + 1900;
            local->month    = t->tm_mon + 1;
            local->mday     = t->tm_mday + 1;
            local->wday     = t->tm_wday + 1;
            local->hour     = t->tm_hour;
            local->min      = t->tm_min;
            local->sec      = t->tm_sec;
            local->nanos    = stime.tv_nsec;
        }

        status_t sleep_msec(size_t delay)
        {
            if (delay <= 0)
                return STATUS_OK;;

            struct timespec req, rem;
            req.tv_nsec = (delay % 1000) * 1000000;
            req.tv_sec  = delay / 1000;

            while ((req.tv_nsec > 0) && (req.tv_sec > 0))
            {
                int res = ::nanosleep(&req, &rem);
                if (res != 0)
                {
                    switch (errno)
                    {
                        case EFAULT:
                        case EINVAL:
                            return STATUS_UNKNOWN_ERR;
                        case EINTR:
                        default:
                            break;
                    }
                }
                else
                    req = rem;
            }

            return STATUS_OK;
        }
#endif /* PLATFORM_WINDOWS */

#ifdef PLATFORM_WINDOWS
        status_t get_temporary_dir(LSPString *path)
        {
            if (get_env_var("TEMP", path) == STATUS_OK)
                return STATUS_OK;
            if (get_env_var("TMP", path) == STATUS_OK)
                return STATUS_OK;
            return (path->set_ascii("tmp")) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t get_temporary_dir(io::Path *path)
        {
            LSPString tmp;
            if (get_env_var("TEMP", &tmp) == STATUS_OK)
                return STATUS_OK;
            if (get_env_var("TMP", &tmp) == STATUS_OK)
                return STATUS_OK;
            if (!tmp.set_ascii("tmp"))
                return STATUS_NO_MEM;
            return path->set(&tmp);
        }
#else
        status_t get_temporary_dir(LSPString *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            return (path->set_ascii("/tmp")) ? STATUS_OK : STATUS_NO_MEM;
        }

        status_t get_temporary_dir(io::Path *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            return path->set("/tmp");
        }
#endif /* PLATFORM_WINDOWS */

        status_t get_current_dir(LSPString *path)
        {
            return io::Dir::get_current(path);
        }

        status_t get_current_dir(io::Path *path)
        {
            return io::Dir::get_current(path);
        }

        status_t follow_url(const char *url)
        {
            LSPString turl;
            if (!turl.set_utf8(url))
                return STATUS_NO_MEM;
            return follow_url(&turl);
        }

        status_t follow_url(const LSPString *url)
        {
        #ifdef PLATFORM_WINDOWS
            ::ShellExecuteW(
                NULL,               // Not associated with window
                L"open",            // Open hyperlink
                url->get_native_utf16(),   // The file to execute
                NULL,               // Parameters
                NULL,               // Directory
                SW_SHOWNORMAL       // Show command
            );
        #else
            status_t res;
            ipc::Process p;

            if ((res = p.set_command("xdg-open")) != STATUS_OK)
                return STATUS_OK;
            if ((res = p.add_arg(url)) != STATUS_OK)
                return STATUS_OK;
            if ((res = p.launch()) != STATUS_OK)
                return STATUS_OK;
            p.wait();
        #endif /* PLATFORM_WINDOWS */

            return STATUS_OK;
        }
    }
}


