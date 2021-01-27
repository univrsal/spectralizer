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

#include "util.hpp"

/* clang-format off */
namespace defaults {
const bool stereo                                         = false;
const visual_mode visual                                  = VM_BARS;
const smooting_mode smoothing                             = SM_NONE;
const uint32_t color                                      = 0xffffffff;

const bool log_freq_scale = false;
const log_freq_qual log_freq_quality                      = LFQ_FAST;
const double log_freq_start                               = 40.0;
const bool log_freq_use_hpf                               = true;
const double log_freq_hpf_curve                           = 20.0;

/* constants for log_freq-related options */
const double log_freq_hpf_curve_max                       = 100.0;
const uint32_t log_freq_quality_fast_detail_mul           = 2;
const uint32_t log_freq_quality_precise_detail_mul        = 8;

const uint16_t detail                                     = 32,
               cx                                         = 50,
               cy                                         = 50,
               fps                                        = 30;

const uint32_t sample_rate                                = 44100,
               sample_size                                = sample_rate / fps;

const double lfreq_cut                                    = 30,
             hfreq_cut                                    = 22050,
             falloff_weight                               = .95,
             gravity                                      = .8;

const uint32_t sgs_points                                 = 3, /* Should be a odd number */
               sgs_passes                                 = 2;

const double mcat_smooth                                  = 1.5;

const uint16_t bar_space                                  = 2,
               bar_width                                  = 5,
               bar_height                                 = 100,
               bar_min_height                             = 5,
               corner_points                              = 5;

const uint16_t wire_thickness                             = 5;
const enum wire_mode wire_mode                            = WM_THIN;

const char *fifo_path                                     = "/tmp/mpd.fifo";
const char *audio_source                                  = "none";

const bool use_auto_scale                                 = true;
const double scale_boost                                  = 0.0;
const double scale_size                                   = 1.0;
}

namespace constants {
const int auto_scale_span                                 = 30;
const double auto_scaling_reset_window                    = 0.1;
const double auto_scaling_erase_percent                   = 0.75;
/* Amount of deviation needed between short term and long
 * term moving max height averages to trigger an autoscaling reset */
const double deviation_amount_to_reset                    = 1.0;
}
/* clang-format on */
