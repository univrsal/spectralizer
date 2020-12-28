/*************************************************************************
 * This file is part of spectralizer
 * github.con/univrsal/spectralizer
 * Copyright 2020 univrsal <uni@vrsal.cf>.
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

#include "circle_bar_visualizer.hpp"
#include "../../source/visualizer_source.hpp"

namespace audio {
circle_bar_visualizer::circle_bar_visualizer(source::config *cfg) : spectrum_visualizer(cfg)
{
    update();
}

void circle_bar_visualizer::render(gs_effect_t *)
{
    // First translate everything to the center, offset for rotation, rotate, undo offset
    auto count = m_bars_left.size() - DEAD_BAR_OFFSET;
    for (size_t i = 0; i < count; i++) { /* Leave the four dead bars the end */
        float pos = float(i) / (count);
        auto w = UTIL_MAX(m_bars_left[i], 1);
        gs_matrix_push();
        {
            gs_matrix_translate3f(m_cfg->cx / 2, m_cfg->cx / 2 + m_radius, 0);

            gs_matrix_translate3f(0, -m_radius, 0);
            gs_matrix_rotaa4f(0, 0, 1, pos * (M_PI * 2 - m_padding) + m_cfg->offset);
            gs_matrix_translate3f(0, m_radius, 0);

            gs_draw_sprite(nullptr, 0, m_cfg->bar_width, w);
        }
        gs_matrix_pop();
    }
}

void circle_bar_visualizer::update()
{
    spectrum_visualizer::update();
    auto count = m_bars_left.size() - DEAD_BAR_OFFSET;
    float spectrum_width = ((m_cfg->bar_width + m_cfg->bar_space) * count);
    m_radius = (spectrum_width * (1 + m_cfg->padding)) / (2 * M_PI);
    m_padding = m_cfg->padding * 2 * M_PI;
    m_cfg->cx = m_radius * 2 + m_cfg->bar_height * 2;
    m_cfg->cy = m_cfg->cx;
}

}
