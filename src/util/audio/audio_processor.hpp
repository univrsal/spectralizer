/**
 * This file is part of spectraliser
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectraliser
 */

#pragma once

#include <graphics/graphics.h>
#define AUDIO_SIZE  65536
#define BUFFER_SIZE 1024

namespace source {
    struct config;
}

class audio_processor
{
public:
    int32_t m_buf_size;
    int16_t m_audio_out_r[AUDIO_SIZE];
    int16_t m_audio_out_l[AUDIO_SIZE];
    int m_channels;

    virtual void update(source::config* cfg) = 0;
    virtual void close_processor() = 0;
};
