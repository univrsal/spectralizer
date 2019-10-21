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

#include "obs_internal_source.hpp"
#include "../../source/visualizer_source.hpp"
#include <util/platform.h>

#define DEFAULT_AUDIO_BUF_MS 10
#define MS_IN_S 100

namespace audio {

static void audio_capture(void *param, obs_source_t *src, const struct audio_data *data, bool muted)
{
	obs_internal_source *s = reinterpret_cast<obs_internal_source *>(param);
	if (s)
		s->capture(src, data, muted);
}

obs_internal_source::obs_internal_source(source::config *cfg) : audio_source(cfg)
{
	circlebuf_init(&m_audio_data[0]);
	circlebuf_init(&m_audio_data[1]);
	update();
}

obs_internal_source::~obs_internal_source()
{
	if (m_capture_source) {
		obs_source_t *source = obs_weak_source_get_source(m_capture_source);
		if (source) {
            info("Removed audio capture from '%s'", obs_source_get_name(source));
            obs_source_remove_audio_capture_callback(source, audio_capture, this);
			obs_source_release(source);
		}
		obs_weak_source_release(m_capture_source);
	}

	for (size_t i = 0; i < 2; i++) {
		circlebuf_free(&m_audio_data[i]);
		bfree(m_audio_buf[i]);
	}
}

void obs_internal_source::capture(obs_source_t *src, const struct audio_data *data, bool muted)
{
	m_cfg->value_mutex.lock();

    if (m_max_capture_frames < data->frames)
        m_max_capture_frames = data->frames;

	size_t expected = m_max_capture_frames * sizeof(float);

	if (expected) {
		if (m_audio_data[0].size > expected * 2) {
			for (auto &buf : m_audio_data) {
				circlebuf_pop_front(&buf, nullptr, expected);
			}
		}

		if (muted) {
			for (size_t i = 0; i < UTIL_MIN(m_num_channels, 2); i++) {
				circlebuf_push_back_zero(&m_audio_data[i], data->frames * sizeof(float));
			}
		} else {
			for (size_t i = 0; i < UTIL_MIN(m_num_channels, 2); i++) {
				circlebuf_push_back(&m_audio_data[i], data->data[i], data->frames * sizeof(float));
            }
		}
	}

#ifdef LINUX
	if (m_cfg->auto_clear)
		m_last_capture = os_gettime_ns();
#endif
	m_cfg->value_mutex.unlock();
}

bool obs_internal_source::tick(float seconds)
{
    /* Audio capturing is done in separate callback
     * and is technically only done, once the circle buffer is
     * filled, but we'll just assume that's always the case */

	/* Update / refresh audio capturing */
	std::string new_name = "";
	if (!m_capture_name.empty() && !m_capture_source) {
		uint64_t t = os_gettime_ns();

		if (t - m_capture_check_time > 3000000000) {
			new_name = m_capture_name;
			m_capture_check_time = t;
		}
	}

	if (!new_name.empty()) {
		obs_source_t *capture = obs_get_source_by_name(new_name.c_str());
		obs_weak_source_t *weak_capture = capture ? obs_source_get_weak_source(capture) : NULL;

		if (!m_capture_name.empty() && new_name == m_capture_name) {
			m_capture_source = weak_capture;
			weak_capture = nullptr;
		}

        if (capture) {
            info("Added audio capture to '%s'", obs_source_get_name(capture));
			obs_source_add_audio_capture_callback(capture, audio_capture, this);
			obs_weak_source_release(weak_capture);
			obs_source_release(capture);
		}
	}

	/* Copy captured data */
	size_t data_size = m_audio_buf_len * sizeof(float);
    if (!data_size) {
        debug("Buffer is empty");
        return false;
    }

	if (m_audio_data[0].size < data_size) {
		/* Clear buffers */
		memset(m_audio_buf[0], 0, data_size);
        memset(m_audio_buf[1], 0, data_size);
        debug("No Data in circle buffer");
        return false;
	} else {
		/* Otherwise copy & convert */
		for (size_t i = 0; i < UTIL_MIN(m_num_channels, 2); i++) {
			circlebuf_pop_front(&m_audio_data[i], m_audio_buf[i], data_size);
		}

		/* Convert to int16 */
		for (size_t chan = 0; chan < UTIL_MIN(m_num_channels, 2); chan++) {
			if (!m_audio_buf[chan])
				continue;

			for (uint32_t i = 0; i < m_audio_buf_len; i++) {
				if (chan == 0) {
					m_cfg->buffer[i].l = static_cast<int16_t>(m_audio_buf[chan][i] * (UINT16_MAX / 2));
				} else {
					m_cfg->buffer[i].r = static_cast<int16_t>(m_audio_buf[chan][i] * (UINT16_MAX / 2));
				}
			}
		}
	}

	return true;
}

void obs_internal_source::resize_audio_buf(size_t new_len)
{
	m_audio_buf_len = new_len;
	m_audio_buf[0] = static_cast<float *>(brealloc(m_audio_buf[0], new_len * sizeof(float)));
	m_audio_buf[1] = static_cast<float *>(brealloc(m_audio_buf[1], new_len * sizeof(float)));
}

void obs_internal_source::update()
{
    m_cfg->sample_rate = audio_output_get_sample_rate(obs_get_audio());
    /* Usually the frame rate is used as a divisor, but
     * it seems that rates below 60 result in a sample size that's too large
     * and therefore will break the visualizer so I'll just use 60 as a constant here
     */
    m_cfg->sample_size = m_cfg->sample_rate / 60;
	m_num_channels = audio_output_get_channels(obs_get_audio());
	obs_weak_source_t *old = nullptr;

	if (m_cfg->audio_source_name.empty()) {
		if (m_capture_source) {
			old = m_capture_source;
			m_capture_source = nullptr;
		}
		m_capture_name = "";
	} else {
		if (m_capture_name.empty() || m_capture_name != m_cfg->audio_source_name) {
			if (m_capture_source) {
				old = m_capture_source;
				m_capture_source = nullptr;
			}
			m_capture_name = m_cfg->audio_source_name;
			m_capture_check_time = os_gettime_ns() - 3000000000;
		}
	}

	if (old) {
		obs_source_t *old_source = obs_weak_source_get_source(old);
        if (old_source) {
            info("Removed audio capture from '%s'", obs_source_get_name(old_source));
			obs_source_remove_audio_capture_callback(old_source, audio_capture, this);
			obs_source_release(old_source);
		}
		obs_weak_source_release(old);
	}

    if (m_audio_buf_len != m_cfg->sample_size)
		resize_audio_buf(m_cfg->sample_size);
}

}
