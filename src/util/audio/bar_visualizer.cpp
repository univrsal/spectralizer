/*************************************************************************
 * This file is part of spectralizer
 * github.con/univrsal/spectralizer
 * Copyright 2020 univrsal <universailp@web.de>.
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

#include "bar_visualizer.hpp"
#include "../../source/visualizer_source.hpp"

namespace audio {

bar_visualizer::bar_visualizer(source::config *cfg) : spectrum_visualizer(cfg) {}

void bar_visualizer::render(gs_effect_t *effect)
{
	if (m_cfg->stereo) {
		size_t i = 0, pos_x = 0;
		uint32_t height_l, height_r;
		uint offset = m_cfg->stereo_space / 2;
		uint center = m_cfg->bar_height / 2 + offset;

		/* Just in case */
		if (m_bars_left.size() != m_cfg->detail + DEAD_BAR_OFFSET)
			m_bars_left.resize(m_cfg->detail + DEAD_BAR_OFFSET, 0.0);
		if (m_bars_right.size() != m_cfg->detail + DEAD_BAR_OFFSET)
			m_bars_right.resize(m_cfg->detail + DEAD_BAR_OFFSET, 0.0);

		for (; i < m_bars_left.size() - DEAD_BAR_OFFSET; i++) { /* Leave the four dead bars the end */
			double bar_left = (m_bars_left[i] > 1.0 ? m_bars_left[i] : 1.0);
			double bar_right = (m_bars_right[i] > 1.0 ? m_bars_right[i] : 1.0);

			height_l = UTIL_MAX(static_cast<uint32_t>(round(bar_left)), 1);
			height_l = UTIL_MIN(height_l, (m_cfg->bar_height / 2));
			height_r = UTIL_MAX(static_cast<uint32_t>(round(bar_right)), 1);
			height_r = UTIL_MIN(height_r, (m_cfg->bar_height / 2));

			pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);

			/* Top */
			gs_matrix_push();
			gs_matrix_translate3f(pos_x, (center - height_l) - offset, 0);
			gs_draw_sprite(nullptr, 0, m_cfg->bar_width, height_l);
			gs_matrix_pop();

			/* Bottom */
			gs_matrix_push();
			gs_matrix_translate3f(pos_x, center + offset, 0);
			gs_draw_sprite(nullptr, 0, m_cfg->bar_width, height_r);
			gs_matrix_pop();
		}
	} else {
		size_t i = 0, pos_x = 0;
		uint32_t height;
		for (; i < m_bars_left.size() - DEAD_BAR_OFFSET; i++) { /* Leave the four dead bars the end */
			auto val = m_bars_left[i] > 1.0 ? m_bars_left[i] : 1.0;
			height = UTIL_MAX(static_cast<uint32_t>(round(val)), 1);
			height = UTIL_MIN(height, m_cfg->bar_height);

			pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
			gs_matrix_push();
			gs_matrix_translate3f(pos_x, (m_cfg->bar_height - height), 0);
			gs_draw_sprite(nullptr, 0, m_cfg->bar_width, height);
			gs_matrix_pop();
		}
	}
	UNUSED_PARAMETER(effect);
}
}
