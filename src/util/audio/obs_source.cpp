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

#include "obs_source.hpp"
#include "../../source/visualizer_source.hpp"

namespace audio {

static void audio_capture(void *param, obs_source_t *src,
                          const struct audio_data *data, bool muted)
{

}

obs_source::obs_source(source::config *cfg)
    : audio_source(cfg)
{
    circlebuf_init(m_audio_buf);
    circlebuf_init(&m_audio_buf[1]);

    circlebuf_reserve(m_audio_buf, cfg->buffer_size);
    circlebuf_reserve(&m_audio_buf[1], cfg->buffer_size);

}

obs_source::~obs_source()
{
    clean_up();
}

bool obs_source::tick(float seconds)
{
    /* Audio capturing is done in separate callback
     * and is technically only done, once the circle buffer is
     * filled, but we'll just assume that's always the case */
    return true;
}

void obs_source::update()
{

}

void obs_source::clean_up()
{
    circlebuf_free(m_audio_buf);
    circlebuf_free(&m_audio_buf[1]);
}

}
