/*
 * OutSequence.h
 *
 *  Created on: 18 июн. 2018 г.
 *      Author: sadko
 */

#ifndef LSP_PLUG_IN_IO_OUTSEQUENCE_H_
#define LSP_PLUG_IN_IO_OUTSEQUENCE_H_

#include <lsp-plug.in/runtime/version.h>
#include <lsp-plug.in/common/types.h>
#include <lsp-plug.in/io/charset.h>
#include <lsp-plug.in/io/Path.h>
#include <lsp-plug.in/io/File.h>
#include <lsp-plug.in/io/CharsetEncoder.h>
#include <lsp-plug.in/io/IOutStream.h>
#include <lsp-plug.in/io/IOutSequence.h>
#include <lsp-plug.in/stdlib/stdio.h>

namespace lsp
{
    namespace io
    {
        class OutSequence: public IOutSequence
        {
            protected:
                IOutStream     *pOS;
                size_t          nWrapFlags;
                CharsetEncoder  sEncoder;
    
            protected:
                status_t        flush_buffer_internal(bool force);

            private:
                OutSequence & operator = (const OutSequence &);

            public:
                explicit OutSequence();
                virtual ~OutSequence();

            public:
                using IOutSequence::write;
                using IOutSequence::write_ascii;

            public:
                status_t wrap(FILE *fd, bool close, const char *charset = NULL);

                status_t wrap_native(fhandle_t fd, bool close, const char *charset = NULL);

                status_t wrap(File *fd, size_t flags, const char *charset = NULL);

                status_t wrap(IOutStream *os, size_t flags, const char *charset = NULL);

                status_t open(const char *path, size_t mode, const char *charset = NULL);

                status_t open(const LSPString *path, size_t mode, const char *charset = NULL);

                status_t open(const Path *path, size_t mode, const char *charset = NULL);

                virtual status_t write(lsp_wchar_t c);

                virtual status_t write(const lsp_wchar_t *c, size_t count);

                virtual status_t write_ascii(const char *s, size_t count);

                virtual status_t flush();

                virtual status_t close();
        };
    }
} /* namespace lsp */

#endif /* LSP_PLUG_IN_IO_OUTSEQUENCE_H_ */
