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

#include "wire_visualizer.hpp"
#include "../../source/visualizer_source.hpp"
#include <graphics/matrix4.h>

namespace audio {
wire_visualizer::wire_visualizer(source::config *cfg) : spectrum_visualizer(cfg) {}

gs_vertbuffer_t *wire_visualizer::make_thin(channel_mode cm)
{
    gs_render_start(true);
    size_t i = 0, pos_x = 0;
    int32_t height = 0;
    int32_t offset = 0;
    int32_t center = 0;

    if (cm != CM_BOTH) {
        offset = m_cfg->stereo_space / 2;
        center = m_cfg->bar_height / 2 + offset;
    }

    if (cm == CM_RIGHT) {
        for (; i < UTIL_MIN(m_cfg->detail + 1, m_bars_right.size()); i++) {
            auto val = m_bars_right[i];
            height = UTIL_MAX(static_cast<int32_t>(round(val)), 1);

            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
            gs_vertex2f(pos_x, center + offset + height);
        }
    } else if (cm == CM_LEFT) {
        for (; i < UTIL_MIN(m_cfg->detail + 1, m_bars_left.size()); i++) {
            auto val = m_bars_left[i];
            height = UTIL_MAX(static_cast<int32_t>(round(val)), 1);

            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
            gs_vertex2f(pos_x, center - offset - height);
        }
    } else {
        for (; i < UTIL_MIN(m_cfg->detail + 1, m_bars_left.size()); i++) {
            auto val = m_bars_left[i];
            height = UTIL_MAX(static_cast<int32_t>(round(val)), 1);

            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
            gs_vertex2f(pos_x, m_cfg->bar_height - height);
        }
    }

    return gs_render_save();
}

gs_vertbuffer_t *wire_visualizer::make_thick(channel_mode cm)
{
    gs_render_start(true);
    size_t i = 0, pos_x = 0;
    int32_t height = 0;
    int32_t offset = 0;
    int32_t center = 0;

    if (cm != CM_BOTH) {
        offset = m_cfg->stereo_space / 2;
        center = m_cfg->bar_height / 2 + offset;
    }

    if (cm == CM_RIGHT) {
        for (; i < UTIL_MIN(m_cfg->detail + 1, m_bars_right.size()); i++) {
            auto val = m_bars_right[i];
            height = UTIL_MAX(static_cast<int32_t>(round(val)), 1);

            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
            gs_vertex2f(pos_x, center + offset + height);
            gs_vertex2f(pos_x, center + offset + height - m_cfg->wire_thickness);
        }
    } else if (cm == CM_LEFT) {
        for (; i < UTIL_MIN(m_cfg->detail + 1, m_bars_left.size()); i++) {
            auto val = m_bars_left[i];
            height = UTIL_MAX(static_cast<int32_t>(round(val)), 1);

            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
            gs_vertex2f(pos_x, center - offset - height);
            gs_vertex2f(pos_x, center - offset - height + m_cfg->wire_thickness);
        }
    } else {
        for (; i < UTIL_MIN(m_cfg->detail + 1, m_bars_left.size()); i++) {
            auto val = m_bars_left[i];
            height = UTIL_MAX(static_cast<int32_t>(round(val)), 1);

            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
            gs_vertex2f(pos_x, m_cfg->bar_height - height);
            gs_vertex2f(pos_x, m_cfg->bar_height - height + m_cfg->wire_thickness);
        }
    }
    return gs_render_save();
}

gs_vertbuffer_t *wire_visualizer::make_filled(channel_mode cm)
{

    gs_render_start(true);
    size_t i = 0, pos_x = 0;
    int32_t height = 0;
    int32_t offset = 0;
    int32_t center = 0;

    if (cm != CM_BOTH) {
        offset = m_cfg->stereo_space / 2;
        center = m_cfg->bar_height / 2 + offset;
    }

    if (cm == CM_RIGHT) {
        for (; i < UTIL_MIN(m_cfg->detail + 1, m_bars_right.size()); i++) {
            auto val = m_bars_right[i];
            height = UTIL_MAX(static_cast<int32_t>(round(val)), 1);

            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
            gs_vertex2f(pos_x, center + offset + height);
            gs_vertex2f(pos_x, center + offset);
        }
    } else if (cm == CM_LEFT) {
        for (; i < UTIL_MIN(m_cfg->detail + 1, m_bars_left.size()); i++) {
            auto val = m_bars_left[i];
            height = UTIL_MAX(static_cast<int32_t>(round(val)), 1);

            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
            gs_vertex2f(pos_x, center - offset - height);
            gs_vertex2f(pos_x, center - offset);
        }
    } else {
        for (; i < UTIL_MIN(m_cfg->detail + 1, m_bars_left.size()); i++) {
            auto val = m_bars_left[i];
            height = UTIL_MAX(static_cast<int32_t>(round(val)), 1);

            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
            gs_vertex2f(pos_x, m_cfg->bar_height - height);
            gs_vertex2f(pos_x, m_cfg->bar_height);
        }
    }
    return gs_render_save();
}

gs_vertbuffer_t *wire_visualizer::make_filled_inverted(channel_mode cm)
{
    gs_render_start(true);
    size_t i = 0, pos_x = 0;
    uint32_t height = 0;
    int32_t offset = 0;
    int32_t center = 0;

    if (cm != CM_BOTH) {
        offset = m_cfg->stereo_space / 2;
        center = m_cfg->bar_height / 2 + offset;
    }

    if (cm == CM_RIGHT) {
        for (; i < m_bars_left.size() - DEAD_BAR_OFFSET; i++) {
            auto val = m_bars_right[i];
            height = UTIL_MAX(static_cast<uint32_t>(round(val)), 1);

            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
            gs_vertex2f(pos_x, center + offset + height);
            gs_vertex2f(pos_x, m_cfg->cx);
        }
    } else if (cm == CM_LEFT) {
        for (; i < m_bars_left.size() - DEAD_BAR_OFFSET; i++) {
            auto val = m_bars_left[i];
            height = UTIL_MAX(static_cast<uint32_t>(round(val)), 1);

            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
            gs_vertex2f(pos_x, center - offset - height);
            gs_vertex2f(pos_x, 0);
        }
    } else {
        for (; i < m_bars_left.size() - DEAD_BAR_OFFSET; i++) {
            auto val = m_bars_left[i];
            height = UTIL_MAX(static_cast<uint32_t>(round(val)), 1);

            pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
            gs_vertex2f(pos_x, m_cfg->bar_height - height);
            gs_vertex2f(pos_x, 0);
        }
    }
    return gs_render_save();
}

void wire_visualizer::render(gs_effect_t *e)
{
    gs_vertbuffer_t *vb_left = nullptr, *vb_right = nullptr;
    enum gs_draw_mode m = GS_TRISTRIP;
    uint32_t num_verts = 0;
    channel_mode main = m_cfg->stereo ? CM_LEFT : CM_BOTH;

    switch (m_cfg->wire_mode) {
    case WM_THIN:
        vb_left = make_thin(main);
        if (m_cfg->stereo)
            vb_right = make_thin(CM_RIGHT);
        m = GS_LINESTRIP;
        num_verts = m_cfg->detail;
        break;
    case WM_THICK:
        vb_left = make_thick(main);
        if (m_cfg->stereo)
            vb_right = make_thick(CM_RIGHT);
        num_verts = m_cfg->detail * 2;
        break;
    case WM_FILL_INVERTED:
        vb_left = make_filled_inverted(main);
        if (m_cfg->stereo)
            vb_right = make_filled_inverted(CM_RIGHT);
        num_verts = m_cfg->detail * 2;
        break;
    case WM_FILL:
        vb_left = make_filled(main);
        if (m_cfg->stereo)
            vb_right = make_filled(CM_RIGHT);
        num_verts = m_cfg->detail * 2;
        break;
    }

    gs_load_vertexbuffer(vb_left);
    gs_draw(m, 0, num_verts);

    if (vb_right) {
        gs_load_vertexbuffer(vb_right);
        gs_draw(m, 0, num_verts);
    }

    gs_vertexbuffer_destroy(vb_left);
    gs_vertexbuffer_destroy(vb_right);
}
}
