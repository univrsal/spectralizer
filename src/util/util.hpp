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

#include <obs-module.h>
#include <vector>

/* Logging */
#define log_src(log_level, format, ...) \
    blog(log_level, "[spectralizer: '%s'] " format, obs_source_get_name(context->source), ##__VA_ARGS__)
#define write_log(log_level, format, ...) blog(log_level, "[spectralizer] " format, ##__VA_ARGS__)

#define debug(format, ...) write_log(LOG_DEBUG, format, ##__VA_ARGS__)
#define info(format, ...) write_log(LOG_INFO, format, ##__VA_ARGS__)
#define warn(format, ...) write_log(LOG_WARNING, format, ##__VA_ARGS__)

/* clang-format off */

#define UTIL_EULER 2.7182818284590452353
#define UTIL_SWAP(a, b) do { typeof(a) tmp = a; a = b; b = tmp; } while (0)
#define UTIL_MAX(a, b)                  (((a) > (b)) ? (a) : (b))
#define UTIL_MIN(a, b)                  (((a) < (b)) ? (a) : (b))
#define UTIL_CLAMP(lower, x, upper) 	(UTIL_MIN(upper, UTIL_MAX(x, lower)))
#define T_(v)                           obs_module_text(v)

#define T_SOURCE                        T_("Spectralizer.Source")
#define T_SOURCE_MODE                   T_("Spectralizer.Mode")
#define T_MODE_BARS                     T_("Spectralizer.Mode.Bars")
#define T_MODE_WIRE                     T_("Spectralizer.Mode.Wire")
#define T_STEREO                        T_("Spectralizer.Stereo")
#define T_STEREO_SPACE					T_("Spectralizer.Stereo.Space")
#define T_DETAIL                        T_("Spectralizer.Detail")
#define T_REFRESH_RATE                  T_("Spectralizer.RefreshRate")
#define T_AUDIO_SOURCE                  T_("Spectralizer.AudioSource")
#define T_AUDIO_SOURCE_NONE             T_("Spectralizer.AudioSource.None")
#define T_SOURCE_MPD                    T_("Spectralizer.Source.Fifo")
#define T_FIFO_PATH                     T_("Spectralizer.Source.Fifo.Path")
#define T_BAR_WIDTH                     T_("Spectralizer.Bar.Width")
#define T_BAR_HEIGHT                    T_("Spectralizer.Bar.Height")
#define T_SAMPLE_RATE                   T_("Spectralizer.SampleRate")
#define T_BAR_SPACING                   T_("Spectralizer.Bar.Space")
#define T_WIRE_SPACING                  T_("Spectralizer.Wire.Space")
#define T_WIRE_HEIGHT					T_("Spectralizer.Wire.Height")
#define T_COLOR                         T_("Spectralizer.Color")
#define T_GRAVITY                       T_("Spectralizer.Gravity")
#define T_FALLOFF						T_("Spectralizer.Falloff")
#define T_FILTER_MODE                   T_("Spectralizer.Filter.Mode")
#define T_FILTER_NONE                  	T_AUDIO_SOURCE_NONE
#define T_FILTER_MONSTERCAT             T_("Spectralizer.Filter.Monstercat")
#define T_FILTER_SGS					T_("Spectralizer.Filter.SGS")
#define T_SGS_PASSES					T_("Spectralizer.Filter.SGS.Passes")
#define T_SGS_POINTS					T_("Spectralizer.Filter.SGS.Points")
#define T_FILTER_STRENGTH    			T_("Spectralizer.Filter.Strength")
#define T_AUTO_CLEAR					T_("Spectralizer.AutoClear")
#define T_AUTO_SCALE					T_("Spectralizer.Use.AutoScale")
#define T_SCALE_BOOST					T_("Spectralizer.Scale.Boost")
#define T_SCALE_SIZE					T_("Spectralizer.Scale.Size")
#define T_WIRE_MODE_THIN				T_("Spectralizer.Wire.Mode.Thin")
#define T_WIRE_MODE_THICK				T_("Spectralizer.Wire.Mode.Thick")
#define T_WIRE_MODE_FILL				T_("Spectralizer.Wire.Mode.Fill")
#define T_WIRE_MODE_FILL_INVERTED		T_("Spectralizer.Wire.Mode.Fill.Invert")
#define T_WIRE_MODE						T_("Spectralizer.Wire.Mode")
#define T_WIRE_THICKNESS				T_("Spectralizer.Wire.Thickness")

#define S_SOURCE_MODE                   "source_mode"
#define S_STEREO                        "stereo"
#define S_STEREO_SPACE					"stereo_space"
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
#define S_SGS_PASSES					"sgs_passes"
#define S_SGS_POINTS					"sgs_points"
#define S_GRAVITY                       "gravity"
#define S_FALLOFF						"falloff"
#define S_FILTER_STRENGTH   			"filter_strength"
#define S_AUTO_CLEAR  					"auto_clear"
#define S_AUTO_SCALE					"use_auto_scale"
#define S_SCALE_BOOST					"scale_boost"
#define S_SCALE_SIZE					"scale_size"
#define S_WIRE_MODE						"wire_mode"
#define S_WIRE_THICKNESS				"wire_thickness"

enum visual_mode
{
    VM_BARS, VM_WIRE
};

enum wire_mode
{
    WM_THIN, WM_THICK, WM_FILL, WM_FILL_INVERTED
};

enum smooting_mode
{
    SM_NONE = 0,
    SM_MONSTERCAT,
    SM_SGS
};

enum falloff
{
    FO_NONE = 0,
    FO_FILL,
    FO_TOP
};

enum channel_mode
{
    CM_LEFT = 0,
    CM_RIGHT,
    CM_BOTH
};

template<class T>
struct stereo_sample_frame
{
    T l, r;
};

using i16_stereo_sample = struct stereo_sample_frame<int16_t>;
using f32_stereo_sample = struct stereo_sample_frame<float>;

#define CNST			static const constexpr

namespace defaults {
    CNST bool			stereo			= false;
    CNST visual_mode 	visual			= VM_BARS;
    CNST smooting_mode	smoothing		= SM_NONE;
    CNST uint32_t		color			= 0xffffffff;

    CNST uint16_t		detail			= 32,
                        cx				= 50,
                        cy				= 50,
                        fps				= 30;

    CNST uint32_t		sample_rate		= 44100,
                        sample_size 	= sample_rate / fps;

    CNST double			lfreq_cut		= 30,
                        hfreq_cut		= 22050,
                        falloff_weight	= .95,
                        gravity			= .8;
    CNST uint32_t		sgs_points		= 3,		/* Should be a odd number */
                        sgs_passes		= 2;

    CNST double			mcat_smooth		= 1.5;

    CNST uint16_t		bar_space		= 2,
                        bar_width		= 5,
                        bar_height		= 100,
                        bar_min_height	= 5;

    CNST uint16_t		wire_thickness	= 5;
    CNST wire_mode		wire_mode		= WM_THIN;

    CNST char			*fifo_path		= "/tmp/mpd.fifo";
    CNST char			*audio_source	= "none";

    CNST bool			use_auto_scale	= true;
    CNST double			scale_boost		= 0.0;
    CNST double			scale_size		= 1.0;
};

namespace constants {
    CNST int auto_scale_span 						= 30;
    CNST double auto_scaling_reset_window			= 0.1;
    CNST double auto_scaling_erase_percent 			= 0.75;
    /* Amount of deviation needed between short term and long
     * term moving max height averages to trigger an autoscaling reset */
    CNST double deviation_amount_to_reset 			= 1.0;
}

/* clang-format on */
