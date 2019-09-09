/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#pragma once

#include <graphics/graphics.h>
#include <fftw3.h>
#include <mutex>
#include <vector>

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
    public:
        explicit audio_source(source::config* cfg);

        virtual ~audio_source();

        /* Setup/Cleanup */
        virtual void clean_up();
        virtual void update(source::config* cfg);
        virtual bool tick(float seconds, source::config* cfg);
    };

}
