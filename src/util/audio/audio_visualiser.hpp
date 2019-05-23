#pragma once

#include <graphics/graphics.h>

namespace source
{
    struct config;
}

namespace audio
{
    class audio_processor;

    class audio_visualiser
    {
    protected:
        audio::audio_processor* m_processor = nullptr;
    public:

        virtual void update(source::config* cfg) = 0;

        virtual void tick(source::config* cfg, float seconds) = 0;

        virtual void render(source::config* cfg, gs_effect_t* effect) = 0;
    };
}