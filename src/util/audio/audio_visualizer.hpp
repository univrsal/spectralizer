/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#pragma once

#include <graphics/graphics.h>

namespace source
{
    struct config;
}

namespace audio
{
    class audio_source;

    class audio_visualizer
    {
    protected:
        audio::audio_source* m_source = nullptr;
        source::config* m_cfg = nullptr;
        uint16_t m_source_id = 0; /* where to read audio from */
        bool m_data_read = false; /* Audio source will return false if reading failed */

    public:
        audio_visualizer(source::config* cfg);
        virtual ~audio_visualizer();

        virtual void update();

        virtual void tick(float seconds);

        virtual void render(gs_effect_t* effect) = 0;
    };
}
