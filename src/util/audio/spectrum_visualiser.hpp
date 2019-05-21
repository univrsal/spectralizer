/**
 * This file is part of spectraliser
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectraliser
 */

#pragma once
#include "audio_visualiser.hpp"

class spectrum_visualiser : public audio_visualiser
{
public:
    spectrum_visualiser() = default;
    ~spectrum_visualiser();

    void update(source::config* cfg) override;
    void tick(source::config* cfg, float) override;
    void render(source::config* cfg, gs_effect_t* effect) override;
};
