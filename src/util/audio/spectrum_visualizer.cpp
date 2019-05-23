/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#include "spectrum_visualizer.hpp"
#include "../../source/visualizer_source.hpp"
#include "audio_processor.hpp"

namespace audio
{

    spectrum_visualizer::~spectrum_visualizer()
    {
        m_processor->clean_up();
        delete m_processor;
        m_processor = nullptr;
    }

    void spectrum_visualizer::update(source::config* cfg)
    {
        m_processor->update(cfg);
    }

    void spectrum_visualizer::tick(source::config* cfg, float seconds)
    {

    }

    void spectrum_visualizer::render(source::config* cfg, gs_effect_t* effect)
    {

    }
}
