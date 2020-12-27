/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */
#pragma once

#include "../util/util.hpp"
#include <cstdint>
#include <map>
#include <mutex>
#include <obs-module.h>

namespace audio {
class audio_visualizer;
}

namespace source {

struct config {
	std::mutex value_mutex;

	/* obs source stuff */
	obs_source_t *source = nullptr;
	obs_data_t *settings = nullptr;

	/* Misc */
	const char *fifo_path = defaults::fifo_path;
	bool auto_clear = false;
	pcm_stereo_sample *buffer = nullptr;

	/* Appearance settings */
	visual_mode visual = defaults::visual;
	smooting_mode smoothing = defaults::smoothing;
	uint32_t color = defaults::color;
	uint16_t detail = defaults::detail, cx = defaults::cx, cy = defaults::cy;
	uint16_t fps = defaults::fps;

	/* Audio settings */
	uint32_t sample_rate = defaults::sample_rate;
	uint32_t sample_size = defaults::sample_size;

	std::string audio_source_name = "";
	double low_cutoff_freq = defaults::lfreq_cut;
	double high_cutoff_freq = defaults::hfreq_cut;

	/* smoothing */
	uint32_t sgs_points = defaults::sgs_points, sgs_passes = defaults::sgs_passes;

	/* scaling */
	bool use_auto_scale = defaults::use_auto_scale;
	double scale_boost = defaults::scale_boost;
	double scale_size = defaults::scale_size;

	double mcat_smoothing_factor = defaults::mcat_smooth;

	/* log frequency scale */
	bool log_freq_scale = defaults::log_freq_scale;
	log_freq_qual log_freq_quality = defaults::log_freq_quality;
	double log_freq_start = defaults::log_freq_start;
	bool log_freq_use_hpf = defaults::log_freq_use_hpf;
	double log_freq_hpf_curve = defaults::log_freq_hpf_curve;

	/* Bar visualizer settings */
	uint16_t bar_space = defaults::bar_space;
	uint16_t bar_width = defaults::bar_width;
	uint16_t bar_height = defaults::bar_height;
	uint16_t bar_min_height = defaults::bar_min_height;

	/* Wire visualizer settings */
	uint16_t wire_thickness = defaults::wire_thickness;
	enum wire_mode wire_mode = defaults::wire_mode;

	/* Circular visualizer settings */
	float offset;  // in degree
	float padding; // in %

	/* General spectrum settings */
	bool stereo = defaults::stereo;
	uint16_t stereo_space = 0;
	double falloff_weight = defaults::falloff_weight;
	double gravity = defaults::gravity;
};

class visualizer_source {
	config m_config;
	audio::audio_visualizer *m_visualizer = nullptr;
	std::map<uint16_t, std::string> m_source_names;

public:
	visualizer_source(obs_source_t *source, obs_data_t *settings);
	~visualizer_source();

	inline void update(obs_data_t *settings);
	inline void tick(float seconds);
	inline void render(gs_effect_t *effect);

	uint32_t get_width() const { return m_config.cx; }

	uint32_t get_height() const { return m_config.cy; }

	void clear_source_names() { m_source_names.clear(); }
	void add_source(uint16_t id, const char *name) { m_source_names[id] = name; }
};

/* Util for registering the source */
static obs_properties_t *get_properties_for_visualiser(void *data);

void register_visualiser();
}
