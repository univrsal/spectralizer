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
    spectrum_visualizer::spectrum_visualizer(source::config* cfg)
        : audio_visualizer(cfg)
    {
        update();
    }

    void spectrum_visualizer::update()
    {
        audio_visualizer::update();
    }

    void spectrum_visualizer::tick(float seconds)
    {
        audio_visualizer::tick(seconds);
    }

    void spectrum_visualizer::render(gs_effect_t* effect)
    {
        /* TODO: Stereo, right side up mono */
        auto* f = m_processor->get_freqs();
        auto* f_last = m_processor->get_last_freqs();
        int diff, pos_x;

        for (int i = 0; i < m_cfg->detail; i++) {
            diff = UTIL_MAX(5, abs(f[i]));
            if (m_cfg->clamp)
                diff = UTIL_MIN(diff, m_cfg->bar_height);
            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);

            gs_matrix_push();
            gs_matrix_translate3f(pos_x, 0, 0);
            gs_draw_sprite(nullptr, 0, m_cfg->bar_width, diff);
            gs_matrix_pop();
        }

        UNUSED_PARAMETER(effect);
    }
}
