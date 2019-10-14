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

#define BUFFER_SIZE 1024

namespace source
{
    struct config;
}

namespace audio
{
    /* Base class for audio reading */
    class audio_source
    {
    protected:
        source::config *m_cfg;
    public:
        explicit audio_source(source::config *cfg) : m_cfg(cfg) { }

        virtual ~audio_source() {}

        /* obs_source methods */
        virtual void update() = 0;
        virtual bool tick(float seconds) = 0;
    };
}
