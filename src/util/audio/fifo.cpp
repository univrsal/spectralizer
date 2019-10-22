/*************************************************************************
 * This file is part of spectralizer
 * github.con/univrsal/spectralizer
 * Copyright 2019 univrsal <universailp@web.de>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

#ifdef LINUX
#include "fifo.hpp"
#include "../../source/visualizer_source.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <util/platform.h>

#define MAX_READ_ATTEMPTS 100
#define READ_ATTEMPT_SLEEP 1L * 1000000L

namespace audio {

fifo::fifo(source::config *cfg) : audio_source(cfg)
{
	update();
}

fifo::~fifo()
{
	if (m_fifo_fd)
		close(m_fifo_fd);
	m_fifo_fd = 0;
}

void fifo::update()
{
	m_file_path = m_cfg->fifo_path;
	open_fifo();
}

bool fifo::tick(float seconds)
{
	if (m_fifo_fd < 0 && !open_fifo())
		return false;

	auto buffer_size_bytes = static_cast<size_t>(sizeof(pcm_stereo_sample) * m_cfg->sample_size);
	size_t bytes_left = buffer_size_bytes;
	auto attempts = 0;
	memset(m_cfg->buffer, 0, buffer_size_bytes);

	while (bytes_left > 0) {
		int64_t bytes_read = read(m_fifo_fd, m_cfg->buffer, bytes_left);

		if (bytes_read == 0) {
			debug("Could not read any bytes");
			return false;
		} else if (bytes_read == -1) {
			if (errno == EAGAIN) {
				if (attempts > MAX_READ_ATTEMPTS) {
					debug("Couldn't finish reading buffer, bytes read: %d,"
						  "buffer size: %d",
						  bytes_read, buffer_size_bytes);
					memset(m_cfg->buffer, 0, buffer_size_bytes);
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
			bytes_left -= (size_t)bytes_read;
		}
	}

	return true;
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
#endif /* LINUX */
