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

#include "visualizer_source.hpp"
#include "../util/audio/bar_visualizer.hpp"
#include "../util/audio/circle_bar_visualizer.hpp"
#include "../util/audio/wire_visualizer.hpp"
#include "../util/util.hpp"

namespace source {

static auto fifo_filter = "Fifo file(*.fifo);;"
						  "All Files (*.*)";

struct enum_data {
	visualizer_source *vis;
	obs_property *list;
};

visualizer_source::visualizer_source(obs_source_t *source, obs_data_t *settings)
{
	m_config.settings = settings;
	m_config.source = source;

	update(settings);
}

visualizer_source::~visualizer_source()
{
	m_config.value_mutex.lock();
	delete m_visualizer;
	m_visualizer = nullptr;

	if (m_config.buffer) {
		bfree(m_config.buffer);
		m_config.buffer = nullptr;
	}
	m_config.value_mutex.unlock();
}

void visualizer_source::update(obs_data_t *settings)
{
	visual_mode old_mode = m_config.visual;

	m_config.value_mutex.lock();
	m_config.audio_source_name = obs_data_get_string(settings, S_AUDIO_SOURCE);
	m_config.sample_rate = obs_data_get_int(settings, S_SAMPLE_RATE);
	m_config.sample_size = m_config.sample_rate / m_config.fps;
	m_config.visual = (visual_mode)(obs_data_get_int(settings, S_SOURCE_MODE));
	m_config.stereo = obs_data_get_bool(settings, S_STEREO);
	m_config.stereo_space = obs_data_get_int(settings, S_STEREO_SPACE);
	m_config.color = obs_data_get_int(settings, S_COLOR);
	m_config.bar_width = obs_data_get_int(settings, S_BAR_WIDTH);
	m_config.bar_space = obs_data_get_int(settings, S_BAR_SPACE);
	m_config.detail = obs_data_get_int(settings, S_DETAIL);
	m_config.fifo_path = obs_data_get_string(settings, S_FIFO_PATH);
	m_config.bar_height = obs_data_get_int(settings, S_BAR_HEIGHT);
	m_config.smoothing = (smooting_mode)obs_data_get_int(settings, S_FILTER_MODE);
	m_config.sgs_passes = obs_data_get_int(settings, S_SGS_PASSES);
	m_config.sgs_points = obs_data_get_int(settings, S_SGS_POINTS);
	m_config.falloff_weight = obs_data_get_double(settings, S_FALLOFF);
	m_config.gravity = obs_data_get_double(settings, S_GRAVITY);
	m_config.mcat_smoothing_factor = obs_data_get_double(settings, S_FILTER_STRENGTH);
	m_config.cx = UTIL_MAX(m_config.detail * (m_config.bar_width + m_config.bar_space) - m_config.bar_space, 10);
	m_config.cy = UTIL_MAX(m_config.bar_height + (m_config.stereo ? m_config.stereo_space : 0), 10);
	m_config.use_auto_scale = obs_data_get_bool(settings, S_AUTO_SCALE);
	m_config.scale_boost = obs_data_get_double(settings, S_SCALE_BOOST);
	m_config.scale_size = obs_data_get_double(settings, S_SCALE_SIZE);
	m_config.wire_mode = (wire_mode)obs_data_get_int(settings, S_WIRE_MODE);
	m_config.wire_thickness = obs_data_get_int(settings, S_WIRE_THICKNESS);
	m_config.log_freq_scale = obs_data_get_bool(settings, S_LOG_FREQ_SCALE);
	m_config.log_freq_quality = (log_freq_qual)obs_data_get_int(settings, S_LOG_FREQ_SCALE_QUALITY);
	m_config.log_freq_start = obs_data_get_double(settings, S_LOG_FREQ_SCALE_START);
	m_config.log_freq_use_hpf = obs_data_get_bool(settings, S_LOG_FREQ_SCALE_USE_HPF);
	m_config.log_freq_hpf_curve = obs_data_get_double(settings, S_LOG_FREQ_SCALE_HPF_CURVE);

#ifdef LINUX
	m_config.auto_clear = obs_data_get_bool(settings, S_AUTO_CLEAR);

	struct obs_video_info ovi;
	if (obs_get_video_info(&ovi)) {
		m_config.fps = ovi.fps_num;
	} else {
		m_config.fps = 30;
		warn("Couldn't determine fps, mpd fifo might not work as intended!");
	}
#endif

	if (m_visualizer) /* this modifies sample size, if an internal audio source is used */
		m_visualizer->update();

	if (m_config.buffer)
		bfree(m_config.buffer);

	m_config.buffer = static_cast<pcm_stereo_sample *>(bzalloc(m_config.sample_size * sizeof(pcm_stereo_sample)));

	if (old_mode != m_config.visual || !m_visualizer) {
		delete m_visualizer;

		switch (m_config.visual) {
		case VM_BARS:
			m_visualizer = new audio::bar_visualizer(&m_config);
			break;
		case VM_WIRE:
			m_visualizer = new audio::wire_visualizer(&m_config);
			break;
		case VM_CIRCULAR_BARS:
			m_visualizer = new audio::circle_bar_visualizer(&m_config);
		}
	}

	m_config.value_mutex.unlock();
}

void visualizer_source::tick(float seconds)
{
	m_config.value_mutex.lock();

	if (m_visualizer)
		m_visualizer->tick(seconds);

	m_config.value_mutex.unlock();
}

void visualizer_source::render(gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	if (m_visualizer) {
		m_config.value_mutex.lock();
		gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
		gs_eparam_t *color = gs_effect_get_param_by_name(solid, "color");
		gs_technique_t *tech = gs_effect_get_technique(solid, "Solid");

		struct vec4 colorVal;
		vec4_from_rgba(&colorVal, m_config.color);
		gs_effect_set_vec4(color, &colorVal);

		gs_technique_begin(tech);
		gs_technique_begin_pass(tech, 0);

		m_visualizer->render(solid);

		gs_technique_end_pass(tech);
		gs_technique_end(tech);
		m_config.value_mutex.unlock();
	}
}

static bool filter_changed(obs_properties_t *props, obs_property_t *p, obs_data_t *data)
{
	int mode = obs_data_get_int(data, S_FILTER_MODE);
	auto *strength = obs_properties_get(props, S_FILTER_STRENGTH);
	auto *sgs_pass = obs_properties_get(props, S_SGS_PASSES);
	auto *sgs_points = obs_properties_get(props, S_SGS_POINTS);

	if (mode == SM_NONE) {
		obs_property_set_visible(strength, false);
		obs_property_set_visible(sgs_pass, false);
		obs_property_set_visible(sgs_points, false);
	} else if (mode == SM_SGS) {
		obs_property_set_visible(sgs_pass, true);
		obs_property_set_visible(sgs_points, true);
		obs_property_set_visible(strength, false);
	} else if (mode == SM_MONSTERCAT) {
		obs_property_set_visible(strength, true);
		obs_property_set_visible(sgs_pass, false);
		obs_property_set_visible(sgs_points, false);
	}
	return true;
}

static bool use_auto_scale_changed(obs_properties_t *props, obs_property_t *p, obs_data_t *data)
{
	auto state = !obs_data_get_bool(data, S_AUTO_SCALE);
	auto boost = obs_properties_get(props, S_SCALE_BOOST);
	auto size = obs_properties_get(props, S_SCALE_SIZE);

	obs_property_set_visible(boost, state);
	obs_property_set_visible(size, state);
	return true;
}

static bool source_changed(obs_properties_t *props, obs_property_t *p, obs_data_t *data)
{
	auto *id = obs_data_get_string(data, S_AUDIO_SOURCE);
	auto *sr = obs_properties_get(props, S_SAMPLE_RATE);
	obs_property_t *fifo = nullptr;
#ifdef LINUX
	fifo = obs_properties_get(props, S_FIFO_PATH);
#endif
	if (strcmp(id, "mpd") == 0) {
		obs_property_set_visible(sr, true);
		if (fifo) {
			obs_property_set_visible(fifo, true);
		}
	}
	return true;
}

static bool stereo_changed(obs_properties_t *props, obs_property_t *p, obs_data_t *data)
{
	auto stereo = obs_data_get_bool(data, S_STEREO);
	auto *space = obs_properties_get(props, S_STEREO_SPACE);
	obs_property_set_visible(space, stereo);
	return true;
}

static bool visual_mode_changed(obs_properties_t *props, obs_property_t *p, obs_data_t *data)
{
	visual_mode vm = (visual_mode)obs_data_get_int(data, S_SOURCE_MODE);
	auto *wire_mode = obs_properties_get(props, S_WIRE_MODE);
	auto *height = obs_properties_get(props, S_BAR_HEIGHT);
	auto *width = obs_properties_get(props, S_BAR_WIDTH);
	auto *space = obs_properties_get(props, S_BAR_SPACE);

	obs_property_set_visible(width, vm != VM_WIRE);
	obs_property_set_description(space, vm == VM_WIRE ? T_WIRE_SPACING : T_BAR_SPACING);
	obs_property_set_description(height, vm == VM_WIRE ? T_WIRE_HEIGHT : T_BAR_HEIGHT);
	obs_property_set_visible(wire_mode, vm == VM_WIRE);
	return true;
}

static bool wire_mode_changed(obs_properties_t *props, obs_property_t *p, obs_data_t *data)
{
	wire_mode wm = (wire_mode)obs_data_get_int(data, S_WIRE_MODE);
	auto *wire_thickness = obs_properties_get(props, S_WIRE_THICKNESS);
	obs_property_set_visible(wire_thickness, wm == WM_THICK);
	return true;
}

static bool log_freq_changed(obs_properties_t *props, obs_property_t *p, obs_data_t *data)
{
	bool log_freq_enabled = obs_data_get_bool(data, S_LOG_FREQ_SCALE);
	bool log_freq_hpf_enabled = obs_data_get_bool(data, S_LOG_FREQ_SCALE_USE_HPF);
	//auto *log_freq_quality = obs_properties_get(props, S_LOG_FREQ_SCALE_QUALITY);
	auto *log_freq_start = obs_properties_get(props, S_LOG_FREQ_SCALE_START);
	auto *log_freq_use_hpf = obs_properties_get(props, S_LOG_FREQ_SCALE_USE_HPF);
	auto *log_freq_hpf_curve = obs_properties_get(props, S_LOG_FREQ_SCALE_HPF_CURVE);

	// FIXME look below in get_properties_for_visualizer()
	//obs_property_set_visible(log_freq_quality, log_freq_enabled);
	obs_property_set_visible(log_freq_start, log_freq_enabled);
	obs_property_set_visible(log_freq_use_hpf, log_freq_enabled);
	obs_property_set_visible(log_freq_hpf_curve, log_freq_enabled && log_freq_hpf_enabled);
	return true;
}

static bool log_freq_use_hpf_changed(obs_properties_t *props, obs_property_t *p, obs_data_t *data)
{
	bool log_freq_enabled = obs_data_get_bool(data, S_LOG_FREQ_SCALE);
	bool log_freq_hpf_enabled = obs_data_get_bool(data, S_LOG_FREQ_SCALE_USE_HPF);
	auto *log_freq_hpf_curve = obs_properties_get(props, S_LOG_FREQ_SCALE_HPF_CURVE);

	obs_property_set_visible(log_freq_hpf_curve, log_freq_enabled && log_freq_hpf_enabled);
	return true;
}

static bool add_source(void *data, obs_source_t *src)
{
	uint32_t caps = obs_source_get_output_flags(src);
	enum_data *d = (enum_data *)data;

	if ((caps & OBS_SOURCE_AUDIO) == 0)
		return true;
	const char *name = obs_source_get_name(src);
	obs_property_list_add_string(d->list, name, name);
	return true;
}

obs_properties_t *get_properties_for_visualiser(void *data)
{
	UNUSED_PARAMETER(data);
	obs_properties_t *props = obs_properties_create();

	auto *mode =
		obs_properties_add_list(props, S_SOURCE_MODE, T_SOURCE_MODE, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(mode, T_MODE_BARS, (int)VM_BARS);
	obs_property_list_add_int(mode, T_MODE_CIRCLE_BARS, (int)VM_CIRCULAR_BARS);
	obs_property_list_add_int(mode, T_MODE_WIRE, (int)VM_WIRE);
	obs_property_set_modified_callback(mode, visual_mode_changed);

	auto *src =
		obs_properties_add_list(props, S_AUDIO_SOURCE, T_AUDIO_SOURCE, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	auto *filter =
		obs_properties_add_list(props, S_FILTER_MODE, T_FILTER_MODE, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	obs_property_set_modified_callback(filter, filter_changed);
	obs_property_set_modified_callback(src, source_changed);

	obs_property_list_add_int(filter, T_FILTER_NONE, (int)SM_NONE);
	obs_property_list_add_int(filter, T_FILTER_MONSTERCAT, (int)SM_MONSTERCAT);
	obs_property_list_add_int(filter, T_FILTER_SGS, (int)SM_SGS);

	obs_property_set_visible(obs_properties_add_float_slider(props, S_FILTER_STRENGTH, T_FILTER_STRENGTH, 1, 1.5, 0.01),
							 false);
	obs_property_set_visible(obs_properties_add_int(props, S_SGS_POINTS, T_SGS_POINTS, 1, 32, 1), false);
	obs_property_set_visible(obs_properties_add_int(props, S_SGS_PASSES, T_SGS_PASSES, 1, 32, 1), false);

	obs_properties_add_color(props, S_COLOR, T_COLOR);

	/* Bar settings */
	auto *w = obs_properties_add_int(props, S_BAR_WIDTH, T_BAR_WIDTH, 1, UINT16_MAX, 1);
	auto *h = obs_properties_add_int(props, S_BAR_HEIGHT, T_BAR_HEIGHT, 10, UINT16_MAX, 1);
	auto *s = obs_properties_add_int(props, S_BAR_SPACE, T_BAR_SPACING, 0, UINT16_MAX, 1);
	auto *sr = obs_properties_add_int(props, S_SAMPLE_RATE, T_SAMPLE_RATE, 128, UINT16_MAX, 10);
	obs_property_int_set_suffix(sr, " Hz");
	obs_property_int_set_suffix(w, " Pixel");
	obs_property_int_set_suffix(h, " Pixel");
	obs_property_int_set_suffix(s, " Pixel");

	obs_property_set_visible(sr, false); /* Sampel rate is only needed for fifo */

	/* Wire settings */
	auto *wm = obs_properties_add_list(props, S_WIRE_MODE, T_WIRE_MODE, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	auto *th = obs_properties_add_int(props, S_WIRE_THICKNESS, T_WIRE_THICKNESS, 2, 0xffff, 1);
	obs_property_int_set_suffix(th, " Pixel");

	obs_property_list_add_int(wm, T_WIRE_MODE_THIN, WM_THIN);
	obs_property_list_add_int(wm, T_WIRE_MODE_THICK, WM_THICK);
	obs_property_list_add_int(wm, T_WIRE_MODE_FILL, WM_FILL);
	obs_property_list_add_int(wm, T_WIRE_MODE_FILL_INVERTED, WM_FILL_INVERTED);
	obs_property_set_visible(wm, false);
	obs_property_set_visible(th, false);
	obs_property_set_modified_callback(wm, wire_mode_changed);

	/* Scale stuff */
	auto auto_scale = obs_properties_add_bool(props, S_AUTO_SCALE, T_AUTO_SCALE);
	obs_property_set_modified_callback(auto_scale, use_auto_scale_changed);
	obs_properties_add_float_slider(props, S_SCALE_SIZE, T_SCALE_SIZE, 0.001, 2, 0.001);
	obs_properties_add_float_slider(props, S_SCALE_BOOST, T_SCALE_BOOST, 0.001, 100, 0.001);

	/* Smoothing stuff */
	obs_properties_add_float_slider(props, S_GRAVITY, T_GRAVITY, 0, 1, 0.01);
	/* This setting doesn't really do anything, it's used in cli-visualizer to determine
     * the fallow of colors inside bins
     */
	obs_property_set_visible(obs_properties_add_float_slider(props, S_FALLOFF, T_FALLOFF, 0, 2, 0.01), false);

	obs_property_list_add_string(src, T_AUDIO_SOURCE_NONE, defaults::audio_source);
#ifdef LINUX
	/* Add MPD stuff */
	obs_property_list_add_string(src, T_SOURCE_MPD, "mpd");
	auto *path = obs_properties_add_path(props, S_FIFO_PATH, T_FIFO_PATH, OBS_PATH_FILE, fifo_filter, "");
	obs_property_set_visible(path, false);
	obs_properties_add_bool(props, S_AUTO_CLEAR, T_AUTO_CLEAR);
#endif

	auto *log_freq = obs_properties_add_bool(props, S_LOG_FREQ_SCALE, T_LOG_FREQ_SCALE);
	obs_property_set_modified_callback(log_freq, log_freq_changed);

	auto *log_freq_quality = obs_properties_add_list(props, S_LOG_FREQ_SCALE_QUALITY, T_LOG_FREQ_SCALE_QUAL,
													 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(log_freq_quality, T_LOG_FREQ_SCALE_QUAL_FAST, LFQ_FAST);
	obs_property_list_add_int(log_freq_quality, T_LOG_FREQ_SCALE_QUAL_PRECISE, LFQ_PRECISE);
	// FIXME This setting makes no sense with current implementation.
	//
	// Performing a DFT requires input buffer of specific length. Higher quality DFTs require
	// longer input buffer - a classic "sacrifice X for better Y" scenario.
	//
	// Unfortunately, right now Spectralizer creates a spectrum directly from FFTW output and thus
	// sample detail has to be directly tied to OBS's ticks and audio frequency. Increasing detail
	// leaves part of input buffer empty, which makes the result invalid - see
	// obs_internal_source::update() comment as well.
	//
	// Solution would be to double-buffer FFTW results and interpolate on OBS ticks in between the
	// results. This would require reworking spectrum_visualizer.cpp code and most probably would
	// introduce latency between played audio and displayed spectrum - needs further checking.
	//obs_property_set_visible(log_freq_quality, defaults::log_freq_scale);
	obs_property_set_visible(log_freq_quality, false);

	auto *log_freq_start =
		obs_properties_add_float_slider(props, S_LOG_FREQ_SCALE_START, T_LOG_FREQ_SCALE_START, 20.0, 100.0, 0.1);
	obs_property_float_set_suffix(log_freq_start, " Hz");
	obs_property_set_visible(log_freq_start, defaults::log_freq_scale);

	auto *log_freq_use_hpf = obs_properties_add_bool(props, S_LOG_FREQ_SCALE_USE_HPF, T_LOG_FREQ_SCALE_USE_HPF);
	obs_property_set_visible(log_freq_use_hpf, defaults::log_freq_scale);
	obs_property_set_modified_callback(log_freq_use_hpf, log_freq_use_hpf_changed);

	obs_property_set_visible(obs_properties_add_float_slider(props, S_LOG_FREQ_SCALE_HPF_CURVE,
															 T_LOG_FREQ_SCALE_HPF_CURVE, 2.0,
															 defaults::log_freq_hpf_curve_max, 0.1),
							 defaults::log_freq_scale && defaults::log_freq_use_hpf);

	auto *stereo = obs_properties_add_bool(props, S_STEREO, T_STEREO);
	auto *space = obs_properties_add_int(props, S_STEREO_SPACE, T_STEREO_SPACE, 0, UINT16_MAX, 1);
	obs_property_int_set_suffix(space, " Pixel");
	auto *dt = obs_properties_add_int(props, S_DETAIL, T_DETAIL, 1, UINT16_MAX, 1);
	obs_property_int_set_suffix(dt, " Bins");
	obs_property_set_visible(space, false);
	obs_property_set_modified_callback(stereo, stereo_changed);

	enum_data d;
	d.list = src;
	d.vis = reinterpret_cast<visualizer_source *>(data);
	obs_enum_sources(add_source, &d);
	return props;
}

void register_visualiser()
{
	obs_source_info si = {};
	si.id = "spectralizer";
	si.type = OBS_SOURCE_TYPE_INPUT;
	si.output_flags = OBS_SOURCE_VIDEO; // | OBS_SOURCE_CUSTOM_DRAW;
	si.get_properties = get_properties_for_visualiser;

	si.get_name = [](void *) { return T_SOURCE; };
	si.create = [](obs_data_t *settings, obs_source_t *source) {
		return static_cast<void *>(new visualizer_source(source, settings));
	};
	si.destroy = [](void *data) { delete reinterpret_cast<visualizer_source *>(data); };
	si.get_width = [](void *data) { return reinterpret_cast<visualizer_source *>(data)->get_width(); };
	si.get_height = [](void *data) { return reinterpret_cast<visualizer_source *>(data)->get_height(); };
	si.icon_type = OBS_ICON_TYPE_AUDIO_OUTPUT;
	si.get_defaults = [](obs_data_t *settings) {
		obs_data_set_default_int(settings, S_COLOR, 0xFFFFFFFF);
		obs_data_set_default_int(settings, S_DETAIL, defaults::detail);
		obs_data_set_default_bool(settings, S_STEREO, defaults::stereo);
		obs_data_set_default_int(settings, S_SOURCE_MODE, (int)VM_BARS);
		obs_data_set_default_string(settings, S_AUDIO_SOURCE, defaults::audio_source);
		obs_data_set_default_int(settings, S_SAMPLE_RATE, defaults::sample_rate);
		obs_data_set_default_int(settings, S_FILTER_MODE, (int)SM_NONE);
		obs_data_set_default_double(settings, S_FILTER_STRENGTH, defaults::mcat_smooth);
		obs_data_set_default_double(settings, S_GRAVITY, defaults::gravity);
		obs_data_set_default_double(settings, S_FALLOFF, defaults::falloff_weight);
		obs_data_set_default_string(settings, S_FIFO_PATH, defaults::fifo_path);
		obs_data_set_default_int(settings, S_SGS_PASSES, defaults::sgs_passes);
		obs_data_set_default_int(settings, S_SGS_POINTS, defaults::sgs_points);
		obs_data_set_default_int(settings, S_BAR_WIDTH, defaults::bar_width);
		obs_data_set_default_int(settings, S_BAR_HEIGHT, defaults::bar_height);
		obs_data_set_default_int(settings, S_BAR_SPACE, defaults::bar_space);
		obs_data_set_default_bool(settings, S_AUTO_SCALE, defaults::use_auto_scale);
		obs_data_set_default_double(settings, S_SCALE_SIZE, defaults::scale_size);
		obs_data_set_default_double(settings, S_SCALE_BOOST, defaults::scale_boost);
		obs_data_set_default_int(settings, S_WIRE_MODE, defaults::wire_mode);
		obs_data_set_default_int(settings, S_WIRE_THICKNESS, defaults::wire_thickness);
		obs_data_set_default_bool(settings, S_LOG_FREQ_SCALE, defaults::log_freq_scale);
		obs_data_set_default_int(settings, S_LOG_FREQ_SCALE_QUALITY, defaults::log_freq_quality);
		obs_data_set_default_double(settings, S_LOG_FREQ_SCALE_START, defaults::log_freq_start);
		obs_data_set_default_bool(settings, S_LOG_FREQ_SCALE_USE_HPF, defaults::log_freq_use_hpf);
		obs_data_set_default_double(settings, S_LOG_FREQ_SCALE_HPF_CURVE, defaults::log_freq_hpf_curve);
	};

	si.update = [](void *data, obs_data_t *settings) { reinterpret_cast<visualizer_source *>(data)->update(settings); };
	si.video_tick = [](void *data, float seconds) { reinterpret_cast<visualizer_source *>(data)->tick(seconds); };
	si.video_render = [](void *data, gs_effect_t *effect) {
		reinterpret_cast<visualizer_source *>(data)->render(effect);
	};

	obs_register_source(&si);
}

}
