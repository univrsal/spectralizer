/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#include "fifo.hpp"
#include "../../source/visualizer_source.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <util/platform.h>

namespace audio
{
    void fifo::update(source::config* cfg)
    {
        audio_processor::update(cfg);
        m_file_path = cfg->fifo_path;
        if (!open_fifo())
            warn("Failed to open fifo");
        else
            debug("Opened fifo@%s:%i", m_file_path, m_fifo_handle);
    }

    void fifo::tick(float seconds, source::config* cfg)
    {
        if (!m_fifo_handle) {
            m_retry_timer += seconds;
            if (m_retry_timer < 5) {
                return;
            } else {
                m_retry_timer = 0.f;
                if (!open_fifo())
                    return;
            }
        }

        bytes = read(m_fifo_handle, buffer, sizeof(buffer));
        if (bytes < 1) {
            os_sleep_ms(10);
            c++; /* haha */
            if (c > 10) {
                bzero(m_audio_out_l, sizeof(int16_t) * AUDIO_SIZE);
                bzero(m_audio_out_r, sizeof(int16_t) * AUDIO_SIZE);
                c = 0;
                /* Reopen fifo */
                m_data_mutex.unlock();
                if (!open_fifo())
                    return;
            }
        } else {
            c = 0;

            for (i = 0; i < BUFFER_SIZE / 2; i += 2) {

                if (get_channels() == 1) {
                    m_audio_out_l[n] = (buffer[i] + buffer[i + 1]) / 2;
                } else {
                    m_audio_out_l[n] = buffer[i];
                    m_audio_out_r[n] = buffer[i + 1];
                }
                n++;
                if (n >= get_buffer_size() - 1)
                    n = 0;
            }
        }

        audio_processor::tick(seconds, cfg);
    }

    void fifo::clean_up()
    {
        audio_processor::clean_up();
        if (m_fifo_handle)
            close(m_fifo_handle);
        m_fifo_handle = 0;
    }


    bool fifo::open_fifo()
    {
        if (m_fifo_handle)
            close(m_fifo_handle);

        if (m_file_path && strlen(m_file_path) > 0) {
            m_fifo_handle = open(m_file_path, O_RDONLY);

            if (m_fifo_handle < 0) {
                warn("Failed to open fifo '%s'", m_file_path);
            } else {
                /* Set to non blocking reading (doesn't really work I think) */
                int flags = fcntl(m_fifo_handle, F_GETFL, 0);
                fcntl(m_fifo_handle, F_SETFL, flags | O_NONBLOCK);
                return true;
            }
        }
        return false;
    }

    fifo::fifo(source::config* cfg) : audio_processor(cfg)
    {
        /* NO-OP */
    }
} /* namespace audio */
