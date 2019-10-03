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
#include <limits>
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

obs_internal_source::~obs_internal_source() {
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
			}
		}
	}
}

bool obs_internal_source::tick(float seconds)
{
    /* Audio capturing is done in separate callback
     * and is technically only done, once the circle buffer is
     * filled, but we'll just assume that's always the case */

    /* copy & convert data */
    return true;
}

void obs_internal_source::update()
{

}

void obs_internal_source::clean_up()
{
}

}
