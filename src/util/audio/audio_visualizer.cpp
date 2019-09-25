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

#include "audio_visualizer.hpp"
#include "audio_source.hpp"
#include "../../source/visualizer_source.hpp"
#include "fifo.hpp"

namespace audio
{

    audio_visualizer::audio_visualizer(source::config* cfg)
    {
        m_cfg = cfg;

    }

    audio_visualizer::~audio_visualizer()
    {
        if (m_source)
            m_source->clean_up();
        delete m_source;
        m_source = nullptr;
    }

    void audio_visualizer::update()
    {
        if (!m_source || m_cfg->audio_source != m_source_id) {
            m_source_id = m_cfg->audio_source;
            if (m_source)
                delete m_source;
            if (m_cfg->audio_source == 0) {
                m_source = new fifo(m_cfg);
            } else {
                /* TODO: obs audio source processor */
            }
        }
        m_source->update(m_cfg);
    }

    void audio_visualizer::tick(float seconds)
    {
        if (m_source)
            m_data_read = m_source->tick(seconds, m_cfg);
    }
}
