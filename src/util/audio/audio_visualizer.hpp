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
    class audio_processor;

    class audio_visualizer
    {
    protected:
        audio::audio_processor* m_processor = nullptr;
        source::config* m_cfg = nullptr;
    public:
        audio_visualizer(source::config* cfg);
        virtual ~audio_visualizer();

        virtual void update();

        virtual void tick(float seconds);

        virtual void render(gs_effect_t* effect) = 0;
    };
}
