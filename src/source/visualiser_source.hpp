/**
 * This file is part of spectraliser
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectraliser
 */
#pragma once

#include <cstdint>
#include <obs-module.h>
#include "../util/audio/spectrum_visualiser.hpp"
#include "../util/util.hpp"

namespace source {

struct config {
    visual_mode mode = VISUAL_BARS;
    uint32_t detail = 32, cx = 50, cy = 50;
    uint16_t fps = 30;
    float refresh_rate = 1.f / fps;
    float refresh_counter = 0.f;
    bool stereo = false;
    obs_source_t* source = nullptr;
    obs_data_t* settings = nullptr;
    uint32_t sample_rate = 44100;
    uint32_t color;
    uint16_t audio_source = 0; /* Audio source id, 0 is fifo */
    bar_filter filter_mode = BAR_FILTER_NONE;

    const char* fifo_path = nullptr;

    /* Bar visualiser_source settings */
    float gravity = 1.f, integral = 10.f / 100, sens = 100.f;
    uint16_t bar_space = 2;
    uint16_t bar_width = 5;
    uint16_t bar_height = 100;
    double bar_filter_arg = 0; /* Used in monstercat filter */
};

class visualiser_source
{
    config m_config;
    spectrum_visualiser m_renderer;
public:
    visualiser_source(obs_source_t* source, obs_data_t* settings);
    ~visualiser_source();

    inline void update(obs_data_t* settings);
    inline void tick(float seconds);
    inline void render(gs_effect_t* effect);

    uint32_t get_width() const
    { return m_config.cx; }

    uint32_t get_height() const
    { return m_config.cy; }
};

/* Util for registering the source */
static obs_properties_t* get_properties_for_visualiser(void* data);

void register_visualiser();
void cleanup();
}