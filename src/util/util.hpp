/**
 * This file is part of spectraliser
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectraliser
 */

#pragma once

#include <obs-module.h>

#define UTIL_MAX(a, b)                  (((a) > (b)) ? (a) : (b))
#define UTIL_MIN(a, b)                  (((a) < (b)) ? (a) : (b))
#define UTIL_CLAMP(lower, x, upper) (UTIL_MIN(upper, UTIL_MAX(x, lower)))
#define T_(v)                           obs_module_text(v)

#define T_SOURCE                        T_("Spectraliser.Source")
#define T_SOURCE_MODE                   T_("Spectraliser.Mode")
#define T_MODE_BARS                     T_("Spectraliser.Mode.Bars")
#define T_MODE_WIRE                     T_("Spectraliser.Mode.Wire")
#define T_STEREO                        T_("Spectraliser.Stereo")
#define T_DETAIL                        T_("Spectraliser.Detail")
#define T_REFRESH_RATE                  T_("Spectraliser.RefreshRate")
#define T_AUDIO_SOURCE                  T_("Spectraliser.AudioSource")
#define T_SOURCE_MPD                    T_("Spectraliser.Source.MPD.Fifo")
#define T_FIFO_PATH                     T_("Spectraliser.Source.MPD.Fifo.Path")
#define T_BAR_WIDTH                     T_("Spectraliser.Bar.Width")
#define T_BAR_HEIGHT                    T_("Spectraliser.Bar.Height")
#define T_SAMPLE_RATE                   T_("Spectraliser.SampleRate")
#define T_BAR_SPACING                   T_("Spectraliser.Bar.Space")
#define T_COLOR                         T_("Spectraliser.Color")
#define T_BAR_FILTER_MODE               T_("Spectraliser.Bar.Filter")
#define T_BAR_FILTER_MODE_NONE          T_("Spectraliser.Bar.Filter.None")
#define T_BAR_FILTER_MODE_MONSTERCAT    T_("Spectraliser.Bar.Filter.Monstercat")
#define T_BAR_FILTER_MODE_WAVES         T_("Spectraliser.Bar.Filter.Waves")
#define T_BAR_FILTER_ARG                T_("Spectraliser.Bar.Filter.Arg")

#define S_SOURCE_MODE                   "source_mode"
#define S_STEREO                        "stereo"
#define S_DETAIL                        "detail"
#define S_REFRESH_RATE                  "refresh_rate"
#define S_AUDIO_SOURCE                  "audio_source"
#define S_FIFO_PATH                     "fifo_path"
#define S_BAR_WIDTH                     "width"
#define S_BAR_HEIGHT                    "height"
#define S_SAMPLE_RATE                   "sample_rate"
#define S_BAR_SPACE                     "bar_space"
#define S_COLOR                         "color"
#define S_BAR_FILTER_MODE               "bar_filter_mode"
#define S_BAR_FILTER_ARG                "bar_filter_arg"

enum visual_mode
{
    VISUAL_BARS, VISUAL_WIRE
};