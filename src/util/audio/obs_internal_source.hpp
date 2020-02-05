/*************************************************************************
 * This file is part of spectralizer
 * github.con/univrsal/spectralizer
 * Copyright 2020 univrsal <universailp@web.de>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

#pragma once
#include "audio_source.hpp"
#include <media-io/audio-io.h>
#include <mutex>
#include <obs-module.h>
#include <util/circlebuf.h>

namespace audio {

class obs_internal_source : public audio_source {
    std::string m_capture_name = "";
    obs_weak_source_t* m_capture_source = nullptr;
    size_t m_max_capture_frames = 0;
    uint8_t m_num_channels = 0;
    uint64_t m_capture_check_time = 0;
    circlebuf m_audio_data[2]; /* Left & Right data from capture callback */
    float* m_audio_buf[2] {}; /* Copy of captured audio */
    size_t m_audio_buf_len = 0;
#ifdef LINUX
    /* Used to keep track of last audio capture callback to decide
	 * whether audio playback has stopped to clear the buffer.
	 * This usually is needed when JACK is used
	 */
    uint64_t m_last_capture = 0;
#endif
    void resize_audio_buf(size_t new_len);

public:
    obs_internal_source(source::config* cfg);
    ~obs_internal_source() override;

    bool tick(float seconds) override;
    void update() override;

    void capture(obs_source_t* src, const struct audio_data* data, bool muted);
};

}
