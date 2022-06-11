/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-runtime-lib
 * Created on: 20 апр. 2020 г.
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

#include <lsp-plug.in/mm/OutAudioFileStream.h>
#include <private/mm/MMIOWriter.h>
#include <private/mm/ACMStream.h>

#ifdef USE_LIBSNDFILE
    #if (__SIZEOF_INT__ == 4)
        #define AFS_S32_CPU     mm::SFMT_S32_CPU
    #endif

    #if (__SIZEOF_SHORT__ == 2)
        #define AFS_S16_CPU     mm::SFMT_S16_CPU
    #elif ((__SIZEOF_SHORT__ == 4) && (!defined(AFS_S32_CPU)))
        #define AFS_S32_CPU     mm::SFMT_S32_CPU
    #endif
#endif /* USE_LIBSNDFILE */

namespace lsp
{
    namespace mm
    {
        
        OutAudioFileStream::OutAudioFileStream()
        {
        #ifdef USE_LIBSNDFILE
            hHandle         = NULL;
        #else
            pMMIO           = NULL;
            pACM            = NULL;
            nTotalFrames    = 0;
            pFormat         = NULL;
        #endif /* USE_LIBSNDFILE */

            nCodec          = 0;
            bSeekable       = false;
        }
        
        OutAudioFileStream::~OutAudioFileStream()
        {
            IOutAudioStream::close();
            close_handle();
        }

    #ifdef USE_LIBSNDFILE
        status_t OutAudioFileStream::decode_sf_error(SNDFILE *fd)
        {
            switch (sf_error(fd))
            {
                case SF_ERR_NO_ERROR:
                    return STATUS_OK;
                case SF_ERR_UNRECOGNISED_FORMAT:
                    return STATUS_BAD_FORMAT;
                case SF_ERR_MALFORMED_FILE:
                    return STATUS_CORRUPTED_FILE;
                case SF_ERR_UNSUPPORTED_ENCODING:
                    return STATUS_BAD_FORMAT;
                default:
                    return STATUS_UNKNOWN_ERR;
            }
        }

        bool OutAudioFileStream::select_sndfile_format(SF_INFO *info, audio_stream_t *fmt, size_t codec)
        {
            int res = 0;

            // Choose audio format
            switch (codec & AFMT_MASK)
            {
                case AFMT_WAV:          res |= SF_FORMAT_WAV; break;
                case AFMT_AIFF:         res |= SF_FORMAT_AIFF; break;
                case AFMT_AU:           res |= SF_FORMAT_AU; break;
                case AFMT_RAW:          res |= SF_FORMAT_RAW; break;
                case AFMT_PAF:          res |= SF_FORMAT_PAF; break;
                case AFMT_SVX:          res |= SF_FORMAT_SVX; break;
                case AFMT_NIST:         res |= SF_FORMAT_NIST; break;
                case AFMT_VOC:          res |= SF_FORMAT_VOC; break;
                case AFMT_IRCAM:        res |= SF_FORMAT_IRCAM; break;
                case AFMT_W64:          res |= SF_FORMAT_W64; break;
                case AFMT_MAT4:         res |= SF_FORMAT_MAT4; break;
                case AFMT_MAT5:         res |= SF_FORMAT_MAT5; break;
                case AFMT_PVF:          res |= SF_FORMAT_PVF; break;
                case AFMT_XI:           res |= SF_FORMAT_XI; break;
                case AFMT_HTK:          res |= SF_FORMAT_HTK; break;
                case AFMT_SDS:          res |= SF_FORMAT_SDS; break;
                case AFMT_AVR:          res |= SF_FORMAT_AVR; break;
                case AFMT_WAVEX:        res |= SF_FORMAT_WAVEX; break;
                case AFMT_SD2:          res |= SF_FORMAT_SD2; break;
                case AFMT_FLAC:         res |= SF_FORMAT_FLAC; break;
                case AFMT_CAF:          res |= SF_FORMAT_CAF; break;
                case AFMT_WVE:          res |= SF_FORMAT_WVE; break;
                case AFMT_OGG:          res |= SF_FORMAT_OGG; break;
                case AFMT_MPC2K:        res |= SF_FORMAT_MPC2K; break;
                case AFMT_RF64:         res |= SF_FORMAT_RF64; break;
                default:
                    return false;
            }

            // Choose codec
            switch (codec & CFMT_MASK)
            {
                case CFMT_PCM:
                {
                    // Set sample format
                    switch (sformat_format(fmt->format))
                    {
                        case SFMT_U8:   res |= SF_FORMAT_PCM_U8; break;
                        case SFMT_S8:   res |= SF_FORMAT_PCM_S8; break;
                        case SFMT_U16:
                        case SFMT_S16:
                            res |= SF_FORMAT_PCM_16;
                            break;
                        case SFMT_S24:
                        case SFMT_U24:
                            res |= SF_FORMAT_PCM_24;
                            break;
                        case SFMT_S32:
                        case SFMT_U32:
                            res |= SF_FORMAT_PCM_32;
                            break;

                        case SFMT_F32:  res |= SF_FORMAT_FLOAT; break;
                        case SFMT_F64:  res |= SF_FORMAT_DOUBLE; break;
                        default:
                            return false;
                    }
                }
                break;

                case CFMT_ULAW:         res |= SF_FORMAT_ULAW; break;
                case CFMT_ALAW:         res |= SF_FORMAT_ALAW; break;
                case CFMT_IMA_ADPCM:    res |= SF_FORMAT_IMA_ADPCM; break;
                case CFMT_MS_ADPCM:     res |= SF_FORMAT_MS_ADPCM; break;
                case CFMT_GSM610:       res |= SF_FORMAT_GSM610; break;
                case CFMT_VOX_ADPCM:    res |= SF_FORMAT_VOX_ADPCM; break;
                case CFMT_G721_32:      res |= SF_FORMAT_G721_32; break;
                case CFMT_G723_24:      res |= SF_FORMAT_G723_24; break;
                case CFMT_G723_40:      res |= SF_FORMAT_G723_40; break;
                case CFMT_DWVW_12:      res |= SF_FORMAT_DWVW_12; break;
                case CFMT_DWVW_16:      res |= SF_FORMAT_DWVW_16; break;
                case CFMT_DWVW_24:      res |= SF_FORMAT_DWVW_24; break;
                case CFMT_DWVW_N:       res |= SF_FORMAT_DWVW_N; break;
                case CFMT_DPCM_8:       res |= SF_FORMAT_DPCM_8; break;
                case CFMT_DPCM_16:      res |= SF_FORMAT_DPCM_16; break;
                case CFMT_VORBIS:       res |= SF_FORMAT_VORBIS; break;
            #ifdef LIBSNDFILE_HAS_ALAC
                case CFMT_ALAC_16:      res |= SF_FORMAT_ALAC_16; break;
                case CFMT_ALAC_20:      res |= SF_FORMAT_ALAC_20; break;
                case CFMT_ALAC_24:      res |= SF_FORMAT_ALAC_24; break;
                case CFMT_ALAC_32:      res |= SF_FORMAT_ALAC_32; break;
            #endif /* LIBSNDFILE_HAS_ALAC */

                default:
                    return false;
            }

            // Choose endianess
            switch (sformat_endian(fmt->format))
            {
                case SFMT_DFL:          res |= SF_ENDIAN_FILE; break;
                case SFMT_LE:           res |= SF_ENDIAN_LITTLE; break;
                case SFMT_BE:           res |= SF_ENDIAN_BIG; break;
                default:
                    return false;
            }

            info->frames        = fmt->frames;
            info->samplerate    = fmt->srate;
            info->channels      = fmt->channels;
            info->format        = res;
            info->sections      = 0;
            info->seekable      = 0;

            return true;
        }
    #endif /* USE_LIBSNDFILE */

        status_t OutAudioFileStream::open(const char *path, const audio_stream_t *fmt, size_t codec)
        {
            if (path == NULL)
                return set_error(STATUS_BAD_ARGUMENTS);
            LSPString xpath;
            if (!xpath.set_utf8(path))
                return set_error(STATUS_NO_MEM);
            return open(&xpath, fmt, codec);
        }

        status_t OutAudioFileStream::open(const io::Path *path, const audio_stream_t *fmt, size_t codec)
        {
            if (path == NULL)
                return set_error(STATUS_BAD_ARGUMENTS);
            return open(path->as_string(), fmt, codec);
        }

        status_t OutAudioFileStream::open(const LSPString *path, const audio_stream_t *fmt, size_t codec)
        {
            if (!is_closed())
                return set_error(STATUS_OPENED);
            if (fmt == NULL)
                return set_error(STATUS_BAD_ARGUMENTS);

        #ifdef USE_LIBSNDFILE
            audio_stream_t tmp;
            SF_INFO info;
            tmp     = *fmt;

            if (!select_sndfile_format(&info, &tmp, codec))
                return set_error(STATUS_UNSUPPORTED_FORMAT);

            // Open handle
            SNDFILE *sf = sf_open(path->get_native(), SFM_WRITE, &info);
            if (sf == NULL)
                return -set_error(decode_sf_error(sf));

            // Commit new state
            sFormat             = tmp;
            hHandle             = sf;
            nOffset             = 0;
            bSeekable           = info.seekable;

            return set_error(STATUS_OK);
        #else
            if ((codec & AFMT_MASK) != AFMT_WAV)
                return set_error(STATUS_UNSUPPORTED_FORMAT);

            WAVEFORMATEX rfmt;

            ::ZeroMemory(&rfmt, sizeof(WAVEFORMATEX));
            rfmt.cbSize             = 0;
            rfmt.nChannels          = fmt->channels;
            rfmt.nSamplesPerSec     = fmt->srate;
            rfmt.wBitsPerSample     = sformat_size_of(fmt->format) * 8;
            rfmt.nBlockAlign        = fmt->channels * sformat_size_of(fmt->format);
            rfmt.nAvgBytesPerSec    = fmt->srate * rfmt.nBlockAlign;

            switch (codec & CFMT_MASK)
            {
                case CFMT_PCM:
                    rfmt.wFormatTag  = (sformat_format(fmt->format) == SFMT_F32) ?
                            WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
                    break;

                case CFMT_ULAW:         rfmt.wFormatTag = WAVE_FORMAT_MULAW; break;
                case CFMT_ALAW:         rfmt.wFormatTag = WAVE_FORMAT_ALAW; break;
                case CFMT_MS_ADPCM:     rfmt.wFormatTag = WAVE_FORMAT_ADPCM; break;
                case CFMT_IMA_ADPCM:    rfmt.wFormatTag = WAVE_FORMAT_IMA_ADPCM; break;

                case CFMT_GSM610:       rfmt.wFormatTag = WAVE_FORMAT_GSM610; break;

                case CFMT_G721_32:      rfmt.wFormatTag = WAVE_FORMAT_G721_ADPCM; break;
                case CFMT_G723_24:      rfmt.wFormatTag = WAVE_FORMAT_G723_ADPCM; break;
                case CFMT_G723_40:      rfmt.wFormatTag = WAVE_FORMAT_G723_ADPCM; break;

                default:
                    return set_error(STATUS_UNSUPPORTED_FORMAT);
            }

            // Create MMIO
            status_t res;
            MMIOWriter *mmio = new MMIOWriter();
            if (mmio == NULL)
                return set_error(STATUS_NO_MEM);

            if ((rfmt.wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ||
                (rfmt.wFormatTag == WAVE_FORMAT_PCM))
            {
                // Just open MMIO stream with specified format
                if ((res = mmio->open(path, &rfmt, fmt->frames)) == STATUS_OK)
                {
                    // Update state
                    sFormat         = *fmt;
                    sPcmFmt         = rfmt;
                    pFormat         = &sPcmFmt;
                    pMMIO           = mmio;
                    pACM            = NULL;
                    nCodec          = codec;
                    nOffset         = 0;
                    nTotalFrames    = 0;
                    bSeekable       = pMMIO->seekable();

                    return set_error(STATUS_OK);
                }
            }
            else
            {
                // Create ACM stream first and initialize it
                ACMStream *acm      = new ACMStream();
                if ((acm != NULL) && ((res = acm->write_pcm(&rfmt)) == STATUS_OK))
                {
                    pFormat             = acm->in_format();
                    ssize_t sfmt        = decode_sample_format(pFormat);
                    if (sfmt > 0)
                    {
                        // Now open MMIO with specified output format
                        if ((res = mmio->open(path, acm->out_format(), fmt->frames)) == STATUS_OK)
                        {
                            // All is OK, update state
                            pMMIO               = mmio;
                            pACM                = acm;
                            nCodec              = codec;
                            sFormat.srate       = pFormat->nSamplesPerSec;
                            sFormat.channels    = pFormat->nChannels;
                            sFormat.frames      = fmt->frames;
                            sFormat.format      = sfmt;
                            nOffset             = 0;
                            nTotalFrames        = 0;
                            bSeekable           = false;

                            return set_error(STATUS_OK);
                        }
                    }
                    else
                        res  = STATUS_UNSUPPORTED_FORMAT;
                }

                // Close and delete ACM
                acm->close();
                delete acm;
            }

            mmio->close();
            delete mmio;
            return set_error(res);
        #endif /* USE_LIBSNDFILE */
        }

        status_t OutAudioFileStream::close_handle()
        {
        #ifdef USE_LIBSNDFILE
            if (hHandle == NULL)
                return STATUS_OK;

            int res     = sf_close(hHandle);

            hHandle     = NULL;
            bSeekable   = false;
            nOffset     = -1;       // Mark as closed
            nCodec      = 0;

            return set_error((res == 0) ? STATUS_OK : STATUS_IO_ERROR);
        #else
            status_t res = STATUS_OK;

            // Flush all previously encoded data and close ACM
            if (pACM != NULL)
            {
                flush_internal(true);
                pACM->close();
                delete pACM;
                pACM    = NULL;
            }

            // Close MMIO
            if (pMMIO != NULL)
            {
                pMMIO->set_frames(nTotalFrames);
                res     = pMMIO->close();
                delete pMMIO;
                pMMIO   = NULL;
            }

            pFormat = NULL;

            return set_error(res);
        #endif /* USE_LIBSNDFILE */
        }

        status_t OutAudioFileStream::flush()
        {
            if (is_closed())
                return -set_error(STATUS_CLOSED);

        #ifdef USE_LIBSNDFILE
            sf_write_sync(hHandle);
            return set_error(STATUS_OK);
        #else

            status_t res = flush_internal(false);
            if (res != STATUS_OK)
                return set_error(res);

            return set_error(pMMIO->flush());
        #endif /* USE_LIBSNDFILE */
        }

        status_t OutAudioFileStream::close()
        {
            IOutAudioStream::close();
            return close_handle();
        }

        ssize_t OutAudioFileStream::direct_write(const void *src, size_t nframes, size_t fmt)
        {
        #ifdef USE_LIBSNDFILE
            sf_count_t count;
            status_t res;

            switch (sformat_format(fmt))
            {
                case SFMT_S32:
                    count   = sf_writef_int(hHandle, static_cast<const int *>(src), nframes);
                    break;

                case SFMT_S16:
                    count   = sf_writef_short(hHandle, static_cast<const short *>(src), nframes);
                    break;

                case SFMT_F32:
                    count   = sf_writef_float(hHandle, static_cast<const float *>(src), nframes);
                    break;

                case SFMT_F64:
                    count   = sf_writef_double(hHandle, static_cast<const double *>(src), nframes);
                    break;

                default: // Force SFMT_F32 sample format
                    count   = sf_writef_float(hHandle, static_cast<const float *>(src), nframes);
                    break;
            }

            // Is there data written?
            if (count > 0)
                return count;

            res = decode_sf_error(hHandle);
            return -((res == STATUS_OK) ? STATUS_EOF : res);
        #else
            size_t fsize    = pFormat->nBlockAlign;
            if (pMMIO != NULL)
            {
                if (pACM != NULL)
                    return write_acm_convert(src, nframes);

                ssize_t nwritten    = pMMIO->write(src, fsize * nframes);
                return (nwritten < 0) ? nwritten : nwritten / fsize;
            }

            return -STATUS_NOT_SUPPORTED;
        #endif /* USE_LIBSNDFILE */
        }

    #ifndef USE_LIBSNDFILE
        ssize_t OutAudioFileStream::conv_write(const void *src, size_t nframes, size_t fmt)
        {
            ssize_t res = IOutAudioStream::conv_write(src, nframes, fmt);
            if (size_t(nOffset) > nTotalFrames)
                nTotalFrames    = nOffset;
            return res;
        }

        ssize_t OutAudioFileStream::decode_sample_format(WAVEFORMATEX *wfe)
        {
            if (wfe->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
                return SFMT_F32_LE;

            if (wfe->wFormatTag == WAVE_FORMAT_PCM)
            {
                switch (wfe->wBitsPerSample)
                {
                    case 8: return SFMT_U8_CPU;
                    case 16: return SFMT_S16_LE;
                    case 24: return SFMT_S24_LE;
                    case 32: return SFMT_S32_LE;
                    default:
                        break;
                }
            }

            return -1;
        }

        ssize_t OutAudioFileStream::write_acm_convert(const void *src, size_t nframes)
        {
            size_t nwritten     = 0;
            void *dptr;
            const uint8_t *sptr = static_cast<const uint8_t *>(src);
            size_t fsize        = sformat_size_of(sFormat.format) * sFormat.channels;
            nframes            *= fsize;

            while (nwritten < nframes)
            {
                // Perform push if possible
                ssize_t count       = pACM->push(&dptr);
                if (count > 0) // We're still able to push data
                {
                    // Copy data to buffer
                    ssize_t can_push    = nframes - nwritten;
                    if (count > can_push)
                        count   = can_push;
                    ::memcpy(dptr, sptr, count);

                    // Update pointers
                    sptr       += count;
                    nwritten   += count;
                    pACM->commit(count);
                    continue;
                }
                else if (count < 0) // Error occurred?
                {
                    if (nwritten > 0)
                        break;
                    return -set_error(STATUS_UNKNOWN_ERR);
                }

                // Need to perform flush
                status_t res    = flush_internal(false);
                if (res != STATUS_OK)
                {
                    if (nwritten > 0)
                        break;
                    return -set_error(res);
                }
            }

            return nwritten / fsize;
        }

        status_t OutAudioFileStream::flush_internal(bool eof)
        {
            void *dptr;

            while (true)
            {
                // Perform pull of portion of buffer
                ssize_t count       = pACM->pull(&dptr, IO_BUF_SIZE, eof);
                if (count < 0)
                    return -count;
                else if (count == 0)
                    break;

                // Write pulled data to underlying storage
                count               = pMMIO->write(dptr, count);
                if (count < 0)
                    return count;
            }

            return STATUS_OK;
        }
    #endif /* USE_LIBSNDFILE */

        size_t OutAudioFileStream::select_format(size_t rfmt)
        {
        #ifdef USE_LIBSNDFILE
            switch (sformat_format(rfmt))
            {
            #ifdef AFS_S32_CPU
                case SFMT_S32:
                case SFMT_U32:
                case SFMT_S24:
                case SFMT_U24:
                    return SFMT_S32_CPU;
            #endif

            #ifdef AFS_S16_CPU
                case SFMT_S16:
                case SFMT_U16:
                case SFMT_S8:
                case SFMT_U8:
                    return SFMT_S16_CPU;
            #endif

                case SFMT_F32:
                    return SFMT_F32_CPU;

                case SFMT_F64:
                    return SFMT_F64_CPU;

                default:
                    break;
            }

            return SFMT_F32_CPU;
        #else
            if (pFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
                return SFMT_F32_LE;

            if (pFormat->wFormatTag == WAVE_FORMAT_PCM)
            {
                switch (pFormat->wBitsPerSample)
                {
                    case 8: return SFMT_U8_CPU;
                    case 16: return SFMT_S16_LE;
                    case 24: return SFMT_S24_LE;
                    case 32: return SFMT_S32_LE;
                    default:
                        break;
                }
            }

            return -1;
        #endif
        }

        wssize_t OutAudioFileStream::seek(wsize_t nframes)
        {
            if (is_closed())
                return -set_error(STATUS_CLOSED);

        #ifdef USE_LIBSNDFILE
            sf_count_t offset = sf_seek(hHandle, nframes, SEEK_SET);
            if (offset < 0)
                return -set_error(decode_sf_error(hHandle));

            set_error(STATUS_OK);
            return nOffset = offset;
        #else
            size_t fsize    = pFormat->nBlockAlign;
            if ((pMMIO != NULL) && (bSeekable))
            {
                wssize_t res = pMMIO->seek(nframes * fsize);
                if (res < 0)
                {
                    set_error(-res);
                    return res;
                }

                set_error(STATUS_OK);
                return res / fsize;
            }

            return -set_error(STATUS_NOT_IMPLEMENTED);
        #endif /* USE_LIBSNDFILE */
        }
    } /* namespace mm */
} /* namespace lsp */
