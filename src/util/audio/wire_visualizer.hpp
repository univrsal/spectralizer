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

#pragma once
#include "spectrum_visualizer.hpp"

namespace audio {
class wire_visualizer : public spectrum_visualizer {
    gs_vertbuffer_t *make_thin(channel_mode cm);
    gs_vertbuffer_t *make_thick(channel_mode cm);
    gs_vertbuffer_t *make_filled(channel_mode cm);
    gs_vertbuffer_t *make_filled_inverted(channel_mode cm);

public:
    explicit wire_visualizer(source::config *cfg);

    void render(gs_effect_t *e) override;
};
}
