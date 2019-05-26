/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#pragma once

#include <obs-module.h>

/* Logging */
#define log_src(log_level, format, ...)                            \
    blog(log_level, "[spectralizer: '%s'] " format,             \
         obs_source_get_name(context->source), ##__VA_ARGS__)
#define log(log_level, format, ...)                            \
    blog(log_level, "[spectralizer] " format, ##__VA_ARGS__)

#define debug(format, ...)                      \
    log(LOG_DEBUG, format, ##__VA_ARGS__)
#define info(format, ...)                       \
    log(LOG_INFO, format, ##__VA_ARGS__)
#define warn(format, ...)                       \
    log(LOG_WARNING, format, ##__VA_ARGS__)

#define UTIL_MAX(a, b)                  (((a) > (b)) ? (a) : (b))
#define UTIL_MIN(a, b)                  (((a) < (b)) ? (a) : (b))
#define UTIL_CLAMP(lower, x, upper) (UTIL_MIN(upper, UTIL_MAX(x, lower)))
#define T_(v)                           obs_module_text(v)

#define T_SOURCE                        T_("Spectralizer.Source")
#define T_SOURCE_MODE                   T_("Spectralizer.Mode")
#define T_MODE_BARS                     T_("Spectralizer.Mode.Bars")
#define T_MODE_WIRE                     T_("Spectralizer.Mode.Wire")
#define T_STEREO                        T_("Spectralizer.Stereo")
#define T_DETAIL                        T_("Spectralizer.Detail")
#define T_REFRESH_RATE                  T_("Spectralizer.RefreshRate")
#define T_AUDIO_SOURCE                  T_("Spectralizer.AudioSource")
#define T_SOURCE_MPD                    T_("Spectralizer.Source.Fifo")
#define T_FIFO_PATH                     T_("Spectralizer.Source.Fifo.Path")
#define T_BAR_WIDTH                     T_("Spectralizer.Bar.Width")
#define T_BAR_HEIGHT                    T_("Spectralizer.Bar.Height")
#define T_SAMPLE_RATE                   T_("Spectralizer.SampleRate")
#define T_BAR_SPACING                   T_("Spectralizer.Bar.Space")
#define T_COLOR                         T_("Spectralizer.Color")
#define T_GRAVITY                       T_("Spectralizer.Gravity")
#define T_INTEGRAl                      T_("Spectralizer.Integral")
#define T_SENSITIVITY                   T_("Spectralizer.Sensitivity")
#define T_FILTER_MODE                   T_("Spectralizer.Filter.Mode")
#define T_FILTER_NONE                   T_("Spectralizer.Filter.None")
#define T_FILTER_MONSTERCAT             T_("Spectralizer.Filter.Monstercat")
#define T_FILTER_WAVES                  T_("Spectralizer.Filter.Wave")
#define T_MONSTERCAT_FILTER_STRENGTH    T_("Spectralizer.Filter.Monstercat.Strength")

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
#define S_FILTER_MODE                   "filter_mode"
#define S_GRAVITY                       "gravity"
#define S_INTEGRAL                      "integral"
#define S_SENSITIVITY                   "sensitivity"
#define S_MONSTERCAT_FILTER_STRENGTH    "filter_strength"

enum visual_mode
{
    VISUAL_BARS, VISUAL_WIRE
};
