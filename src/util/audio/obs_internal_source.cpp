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

#define DEFAULT_AUDIO_BUF_MS	10
#define MS_IN_S					100

namespace audio {

static void audio_capture(void *param, obs_source_t *src,
                          const struct audio_data *data, bool muted)
{
	obs_internal_source* s = reinterpret_cast<obs_internal_source *>(param);
	if (s)
		s->capture(src, data, muted);
}

obs_internal_source::obs_internal_source(source::config *cfg)
    : audio_source(cfg),
      m_buffer_index(0)
{
}

obs_internal_source::~obs_internal_source()
{
	clean_up();
}

void obs_internal_source::capture(obs_source_t *src, const struct audio_data *data, bool muted)
{
	std::lock_guard<std::mutex> lock(m_cfg->value_mutex);
	float *l = (float *) data->data[0];
	float *r = (float *) data->data[1];

	if (!muted && data->frames > 0) {
		for (int i = 0; i < data->frames; i++) {
			m_cfg->buffer[m_buffer_index + i].l = l[i] * (UINT16_MAX / 2);
			m_cfg->buffer[m_buffer_index + i].r = r[i] * (UINT16_MAX / 2);
			m_buffer_index++;

			if (m_buffer_index >= m_cfg->sample_size) {
				m_buffer_index = 0;
				m_data_ready; /* TODO: Break here? */
			}
		}
	}
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
		obs_weak_source_t *weak_capture = capture ?
		                          obs_source_get_weak_source(capture) : NULL;

		if (!m_capture_name.empty() && new_name == m_capture_name) {
			m_capture_source = weak_capture;
			weak_capture = nullptr;
		}

		if (capture) {
			obs_source_add_audio_capture_callback(capture, audio_capture, this);
			obs_weak_source_release(weak_capture);
			obs_source_release(capture);
		}
	}

	/* copy & convert data */
//    bool cpy = m_data_ready;
//    if (m_data_ready)
//        m_data_ready = false;
//    return cpy;
	return true;
}

void obs_internal_source::update()
{
    m_cfg->sample_rate = audio_output_get_sample_rate(obs_get_audio());
    m_cfg->sample_size = m_cfg->sample_rate * DEFAULT_AUDIO_BUF_MS / MS_IN_S;
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
            obs_source_remove_audio_capture_callback(old_source, audio_capture,
                                                     this);
            obs_source_release(old_source);
        }
        obs_weak_source_release(old);
    }
}

void obs_internal_source::clean_up()
{
}

}
