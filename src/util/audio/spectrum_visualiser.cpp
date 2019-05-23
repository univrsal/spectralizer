/**
 * This file is part of spectraliser
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectraliser
 */

#include "spectrum_visualiser.hpp"
#include "../../source/visualiser_source.hpp"
#include "audio_processor.hpp"

namespace audio
{

    spectrum_visualiser::~spectrum_visualiser()
    {
        m_processor->clean_up();
        delete m_processor;
        m_processor = nullptr;
    }

    void spectrum_visualiser::update(source::config* cfg)
    {
        m_processor->update(cfg);
    }

    void spectrum_visualiser::tick(source::config* cfg, float seconds)
    {

    }

    void spectrum_visualiser::render(source::config* cfg, gs_effect_t* effect)
    {

    }
}
