/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#include "audio_visualizer.hpp"
#include "audio_source.hpp"
#include "../../source/visualizer_source.hpp"
#include "fifo.hpp"

namespace audio {

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
