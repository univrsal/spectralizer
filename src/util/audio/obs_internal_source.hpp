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

#pragma once
#include "audio_source.hpp"
#include <util/circlebuf.h>

namespace audio {

class obs_internal_source : public audio_source
{
    circlebuf m_audio_buf[2];
public:
    obs_internal_source(source::config *cfg);
    ~obs_internal_source() override;

    bool tick(float seconds) override;
    void clean_up() override;
    void update() override;
};

}
