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
        m_file_path = cfg->fifo_path;
        if (open_fifo() && !m_thread_state) {
            m_thread_state = pthread_create(&m_read_thread, nullptr, read_thread_method, this) == 0;
            if (!m_thread_state)
                blog(LOG_ERROR, "[spectralizer] Failed to create fifo read thread");
        }
    }

    void fifo::clean_up()
    {
        audio_processor::clean_up();
        m_data_mutex.lock();

        m_thread_state = false;
        if (m_fifo_handle)
            close(m_fifo_handle);
        m_fifo_handle = 0;

        m_data_mutex.unlock();
    }

    bool fifo::running() const
    {
        return m_thread_state;
    }

    bool fifo::open_fifo()
    {
        m_data_mutex.lock();

        if (m_fifo_handle)
            close(m_fifo_handle);

        if (m_file_path && strlen(m_file_path) > 0) {
            m_fifo_handle = open(m_file_path, O_RDONLY);

            if (m_fifo_handle < 0) {
                blog(LOG_ERROR, "[spectralizer] Failed to open fifo '%s'", m_file_path);
            } else {
                /* Set to non blocking reading (doesn't really work I think) */
                int flags = fcntl(m_fifo_handle, F_GETFL, 0);
                fcntl(m_fifo_handle, F_SETFL, flags | O_NONBLOCK);
                return true;
            }
        }
        m_data_mutex.unlock();
        return false;
    }

    void* read_thread_method(void* arg)
    {
        auto fifo_instance = static_cast<fifo*>(arg);
        int n = 0,      /* audio_out array index */
                i,          /* Loop index */
                c = 0,      /* Buffer clean counter */
                bytes = 0;  /* Bytes read from fifo */
        int16_t buffer[BUFFER_SIZE];

        while (fifo_instance->running()) {
            fifo_instance->m_data_mutex.lock();
            bytes = read(fifo_instance->m_fifo_handle, buffer, sizeof(buffer));
            if (bytes < 1) {
                os_sleep_ms(10);
                c++; /* haha */
                if (c > 10) {
                    bzero(fifo_instance->m_audio_out_l, sizeof(int16_t) * AUDIO_SIZE);
                    bzero(fifo_instance->m_audio_out_r, sizeof(int16_t) * AUDIO_SIZE);
                    c = 0;
                    /* Reopen fifo */
                    if (!fifo_instance->open_fifo())
                        break; /* Exit loop if opening the fifo doesn't work */
                }
            } else {
                c = 0;

                for (i = 0; i < BUFFER_SIZE / 2; i += 2) {

                    if (fifo_instance->get_channels() == 1) {
                        fifo_instance->m_audio_out_l[n] = (buffer[i] + buffer[i + 1]) / 2;
                    } else {
                        fifo_instance->m_audio_out_l[n] = buffer[i];
                        fifo_instance->m_audio_out_r[n] = buffer[i + 1];
                    }

                    n++;
                    if (n == fifo_instance->get_buffer_size() - 1)
                        n = 0;
                }
            }

            fifo_instance->m_data_mutex.unlock();
        }

        return 0;
    }

} /* namespace audio */