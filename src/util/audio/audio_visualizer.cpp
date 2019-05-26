/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#include "audio_visualizer.hpp"
#include "audio_processor.hpp"
#include "../../source/visualizer_source.hpp"
#include "fifo.hpp"

namespace audio {

    audio_visualizer::audio_visualizer(source::config* cfg)
    {
        m_cfg = cfg;
        if (cfg->audio_source == 0) {
            m_processor = new fifo(cfg);
        } else {
            /* TODO: obs audio source processor */
        }
    }

    audio_visualizer::~audio_visualizer()
    {
        m_processor->clean_up();
        delete m_processor;
        m_processor = nullptr;
    }

    void audio_visualizer::update()
    {
        if (m_processor)
            m_processor->update(m_cfg);
    }

    void audio_visualizer::tick(float seconds)
    {
        if (m_processor)
            m_processor->tick(seconds, m_cfg);
    }
}
