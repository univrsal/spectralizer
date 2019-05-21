#pragma once

#include <graphics/graphics.h>

class audio_processor;

namespace source {
    struct config;
}

class audio_visualiser {
    audio_processor* m_procesor = nullptr;
public:

    virtual void update(source::config* cfg) = 0;
    virtual void tick(source::config* cfg) = 0;
    virtual void render(source::config* cfg, gs_effect_t* effect) = 0;
};
