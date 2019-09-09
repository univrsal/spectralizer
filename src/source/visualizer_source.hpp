/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */
#pragma once

#include <cstdint>
#include <obs-module.h>
#include <mutex>
#include "../util/util.hpp"

namespace audio {
    class audio_visualizer;
}

namespace source {

struct config {
    std::mutex value_mutex;

    /* obs source stuff */
    obs_source_t* source = nullptr;
    obs_data_t* settings = nullptr;

    /* Misc */
    const char* fifo_path = defaults::fifo_path;

    pcm_stereo_sample* buffer = nullptr;
    double ignore = 0; /* Cut off lower peaks */

    /* Appearance settings */
    channel_mode channel = defaults::channel;
    visual_mode visual = defaults::visual;
    smooting_mode smoothing = defaults::smoothing;
    uint32_t color = defaults::color;
    uint16_t detail = defaults::detail,
             cx = defaults::cx, cy = defaults::cy;
    uint16_t fps = defaults::fps;
    float refresh_rate = 1.f / fps,
          refresh_counter = 0.f;

    /* Audio settings */
    uint32_t sample_rate = defaults::sample_rate;
    uint32_t sample_size = defaults::sample_size;
    uint16_t buffer_size = 8192;

    uint16_t audio_source = defaults::audio_source;
    double low_cutoff_freq = defaults::lfreq_cut;
    double high_cutoff_freq = defaults::hfreq_cut;

    /* smoothing */
    uint32_t sgs_points = defaults::sgs_points,
             sgs_passes = defaults::sgs_passes;

    double mcat_smoothing_factor = defaults::mcat_smooth;
    uint32_t mcat_bar_width = defaults::mcat_bar_width;
    uint32_t mcat_bar_space = defaults::mcat_bar_space;

    /* Bar visualizer_source settings */
    uint16_t bar_space = defaults::bar_space;
    uint16_t bar_width = defaults::bar_width;
    uint16_t bar_height = defaults::bar_height;
    uint16_t bar_min_height = defaults::bar_min_height;
    double falloff_weight = defaults::falloff_weight;
    bool clamp = true;
};

class visualizer_source
{
    config m_config;
    audio::audio_visualizer* m_visualizer = nullptr;
public:
    visualizer_source(obs_source_t* source, obs_data_t* settings);
    ~visualizer_source();

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
