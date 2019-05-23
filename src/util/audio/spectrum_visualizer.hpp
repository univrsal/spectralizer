/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#pragma once

#include "audio_visualizer.hpp"

namespace audio
{

    class spectrum_visualizer : public audio_visualizer
    {
    public:
        spectrum_visualizer() = default;

        ~spectrum_visualizer();

        void update(source::config* cfg) override;

        void tick(source::config* cfg, float) override;

        void render(source::config* cfg, gs_effect_t* effect) override;
    };

}