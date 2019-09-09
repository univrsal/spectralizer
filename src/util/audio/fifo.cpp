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

#define MAX_READ_ATTEMPTS   100
#define READ_ATTEMPT_SLEEP  1L * 1000000L

namespace audio
{
    fifo::fifo(source::config* cfg) : audio_source(cfg)
    {
        open_fifo();
    }

    void fifo::update(source::config* cfg)
    {
        audio_source::update(cfg);
        m_file_path = cfg->fifo_path;
        open_fifo();
    }

    bool fifo::tick(float seconds, source::config* cfg)
    {
        if (m_fifo_fd < 0 && !open_fifo())
            return false;
        auto buffer_size_bytes =
                static_cast<size_t>(sizeof(pcm_stereo_sample) * cfg->sample_size);
        size_t bytes_left = buffer_size_bytes;
        auto attempts = 0;
        memset(cfg->buffer, 0, buffer_size_bytes);

        while (bytes_left > 0) {
            int64_t bytes_read = read(m_fifo_fd, cfg->buffer, bytes_left);

            if (bytes_read == 0) {
                debug("Could not read any bytes");
                return false;
            } else if (bytes_read == -1) {
                if (errno == EAGAIN) {
                    if (attempts > MAX_READ_ATTEMPTS) {
                        debug("Couldn't finish reading buffer, bytes read: %d,"
                             "buffer size: %d", bytes_read, buffer_size_bytes);
                        memset(cfg->buffer, 0, buffer_size_bytes);
                        close(m_fifo_fd);
                        m_fifo_fd = -1;
                        return false;
                    }
                    /* TODO: Sleep? Would delay thread */
                    ++attempts;
                } else {
                    debug("Error reading file: %d %s", errno, strerror(errno));
                }
            } else {
                bytes_left -= (size_t) bytes_read;
            }
        }
        audio_source::tick(seconds, cfg);
        return true;
    }

    void fifo::clean_up()
    {
        audio_source::clean_up();
        if (m_fifo_fd)
            close(m_fifo_fd);
        m_fifo_fd = 0;
    }


    bool fifo::open_fifo()
    {
        if (m_fifo_fd)
            close(m_fifo_fd);

        if (m_file_path && strlen(m_file_path) > 0) {
            m_fifo_fd = open(m_file_path, O_RDONLY);

            if (m_fifo_fd < 0) {
                warn("Failed to open fifo '%s'", m_file_path);
            } else {
                auto flags = fcntl(m_fifo_fd, F_GETFL, 0);
                auto ret = fcntl(m_fifo_fd, F_SETFL, flags | O_NONBLOCK);
                if (ret < 0)
                    warn("Failed to set fifo flags!");
                return ret >= 0;
            }
        }
        return false;
    }
} /* namespace audio */
