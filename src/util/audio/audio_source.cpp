/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#include "audio_source.hpp"
#include "../../source/visualizer_source.hpp"

/*
    Most processing is reused from glava
    https://github.com/wacossusca34/glava
 */

namespace audio
{
    audio_source::audio_source(source::config* cfg)
    {
        update(cfg);
    }

    audio_source::~audio_source()
    {
        clean_up();
    }

    void audio_source::update(source::config* cfg)
    {

    }

    void audio_source::clean_up()
    {

    }

    bool audio_source::tick(float seconds, source::config* cfg)
    {
    }
} /* namespace audio */
