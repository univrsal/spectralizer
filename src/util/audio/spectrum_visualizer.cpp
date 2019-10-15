/*************************************************************************
 * This file is part of spectralizer
 * github.con/univrsal/spectralizer
 * Copyright 2019 univrsal <universailp@web.de>.
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

#include "spectrum_visualizer.hpp"
#include "../../source/visualizer_source.hpp"
#include "audio_source.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace audio {
spectrum_visualizer::spectrum_visualizer(source::config *cfg)
	: audio_visualizer(cfg),
	  m_fftw_input_left(nullptr),
	  m_fftw_input_right(nullptr),
	  m_fftw_output_left(nullptr),
	  m_fftw_output_right(nullptr),
	  m_fftw_plan_left(nullptr),
	  m_fftw_plan_right(nullptr),
	  m_fftw_results(0),
	  m_silent_runs(0u),
	  m_last_bar_count(0)
{
	update();
}

spectrum_visualizer::~spectrum_visualizer()
{
	bfree(m_fftw_input_left);
	bfree(m_fftw_input_right);
	bfree(m_fftw_output_left);
	bfree(m_fftw_output_right);
}

void spectrum_visualizer::update()
{
	audio_visualizer::update();
	bfree(m_fftw_input_left);
	bfree(m_fftw_input_right);
	bfree(m_fftw_output_left);
	bfree(m_fftw_output_right);

	m_fftw_results = (size_t)m_cfg->sample_size / 2 + 1;
	m_fftw_input_left = (double *)bzalloc(sizeof(double) * m_cfg->sample_size);
	m_fftw_input_right = (double *)bzalloc(sizeof(double) * m_cfg->sample_size);

	m_fftw_output_left = (fftw_complex *)bzalloc(sizeof(fftw_complex) * m_fftw_results);
	m_fftw_output_right = (fftw_complex *)bzalloc(sizeof(fftw_complex) * m_fftw_results);
}

void spectrum_visualizer::tick(float seconds)
{
	if (m_sleeping) {
		m_sleep_count += seconds;
		if (m_sleep_count >= 0.25) {
			m_sleeping = false;
			m_sleep_count = 0.f;
		}
		return;
	}

	audio_visualizer::tick(seconds);
	/* TODO: Check if data was read? */
	if (!m_data_read)
		debug("No data was read...");

	const auto win_height = m_cfg->bar_height;
	bool is_silent_left = true, is_silent_right = true;

	if (m_cfg->channel == CM_BOTH) {
		is_silent_left = prepare_fft_input(m_cfg->buffer, m_cfg->sample_size, m_fftw_input_left, CM_LEFT);
		is_silent_right = prepare_fft_input(m_cfg->buffer, m_cfg->sample_rate, m_fftw_input_right, CM_RIGHT);
	} else {
		is_silent_left = prepare_fft_input(m_cfg->buffer, m_cfg->sample_size, m_fftw_input_left, CM_LEFT);
	}

	if (!(is_silent_left && is_silent_right)) {
		m_silent_runs = 0;
	} else {
		++m_silent_runs;
	}

	/* TODO make this a constant */
	if (m_silent_runs < 30) {
		auto height = win_height;
		m_fftw_plan_left = fftw_plan_dft_r2c_1d(static_cast<int>(m_cfg->sample_size), m_fftw_input_left,
												m_fftw_output_left, FFTW_ESTIMATE);
		if (!m_fftw_plan_left)
			return;
		if (m_cfg->channel == CM_BOTH) {
			m_fftw_plan_right = fftw_plan_dft_r2c_1d(static_cast<int>(m_cfg->sample_size), m_fftw_input_right,
													 m_fftw_output_right, FFTW_ESTIMATE);
			if (!m_fftw_plan_right)
				return;
			fftw_execute(m_fftw_plan_right);
			height /= 2;
		}

		fftw_execute(m_fftw_plan_left);

		create_spectrum_bars(m_fftw_output_left, m_fftw_results, height, m_cfg->detail, &m_bars_left_new,
							 &m_bars_falloff_left);
		if (m_cfg->channel == CM_BOTH)
			create_spectrum_bars(m_fftw_output_right, m_fftw_results, height, m_cfg->detail, &m_bars_right_new,
								 &m_bars_falloff_right);

		m_bars_left.resize(m_bars_left_new.size());
		for (int i = 0; i < m_bars_left.size(); i++)
			m_bars_left[i] = (m_bars_left[i] * 2 + m_bars_left_new[i] + 3) / 3;

		fftw_destroy_plan(m_fftw_plan_left);
		if (m_cfg->channel == CM_BOTH)
			fftw_destroy_plan(m_fftw_plan_right);
	} else {
		debug("No input; sleeping for %d ms.", 250);
		m_sleeping = true;
	}
}

void spectrum_visualizer::render(gs_effect_t *effect)
{
	if (m_cfg->channel == CM_BOTH) {

	} else {
		int i = -1, pos_x = 0;
		int32_t height;
		for (auto val : m_bars_left) {
			i++;
			height = static_cast<int32_t>(round(val));

			if (height <= 1) {
				if (m_cfg->clamp)
					height = m_cfg->bar_min_height;
				else
					continue; /* Don't draw this bar */
			}

			pos_x = i * (m_cfg->bar_width + m_cfg->bar_space);
			gs_matrix_push();
			gs_matrix_translate3f(pos_x, (m_cfg->bar_height - height), 0);
			gs_draw_sprite(nullptr, 0, m_cfg->bar_width, height);
			gs_matrix_pop();
		}
	}
	UNUSED_PARAMETER(effect);
}

bool spectrum_visualizer::prepare_fft_input(pcm_stereo_sample *buffer, uint32_t sample_size, double *fftw_input,
											channel_mode channel_mode)
{
	bool is_silent = true;

	for (auto i = 0u; i < sample_size; ++i) {
		switch (channel_mode) {
		case CM_LEFT:
			fftw_input[i] = buffer[i].l;
			break;
		case CM_RIGHT:
			fftw_input[i] = buffer[i].r;
			break;
		case CM_BOTH:
			fftw_input[i] = buffer[i].l + buffer[i].r;
			break;
		}

		if (is_silent && fftw_input[i] > 0)
			is_silent = false;
	}

	return is_silent;
}

void spectrum_visualizer::smooth_bars(doublev *bars)
{
	switch (m_cfg->smoothing) {
	case SM_MONSTERCAT:
		monstercat_smoothing(bars);
		break;
	case SM_SGS:
		sgs_smoothing(bars);
		break;
	default:;
	}
}

void spectrum_visualizer::sgs_smoothing(doublev *bars)
{
	auto original_bars = *bars;

	auto smoothing_passes = m_cfg->sgs_passes;
	auto smoothing_points = m_cfg->sgs_points;

	for (auto pass = 0u; pass < smoothing_passes; ++pass) {
		auto pivot = static_cast<uint32_t>(std::floor(smoothing_points / 2.0));

		for (auto i = 0u; i < pivot; ++i) {
			(*bars)[i] = original_bars[i];
			(*bars)[original_bars.size() - i - 1] = original_bars[original_bars.size() - i - 1];
		}

		auto smoothing_constant = 1.0 / (2.0 * pivot + 1.0);
		for (auto i = pivot; i < (original_bars.size() - pivot); ++i) {
			auto sum = 0.0;
			for (auto j = 0u; j <= (2 * pivot); ++j) {
				sum += (smoothing_constant * original_bars[i + j - pivot]) + j - pivot;
			}
			(*bars)[i] = sum;
		}

		// prepare for next pass
		if (pass < (smoothing_passes - 1)) {
			original_bars = *bars;
		}
	}
}

void spectrum_visualizer::monstercat_smoothing(doublev *bars)
{
	auto bars_length = static_cast<int64_t>(bars->size());

	// re-compute weights if needed, this is a performance tweak to computer the
	// smoothing considerably faster
	if (m_monstercat_smoothing_weights.size() != bars->size()) {
		m_monstercat_smoothing_weights.reserve(bars->size());
		for (auto i = 0u; i < bars->size(); ++i) {
			m_monstercat_smoothing_weights[i] = std::pow(m_cfg->mcat_smoothing_factor, i);
		}
	}

	// apply monstercat sytle smoothing
	// Since this type of smoothing smoothes the bars around it, doesn't make
	// sense to smooth the first value so skip it.
	for (auto i = 1l; i < bars_length; ++i) {
		auto outer_index = static_cast<size_t>(i);

		if ((*bars)[outer_index] < m_cfg->bar_min_height) {
			(*bars)[outer_index] = m_cfg->bar_min_height;
		} else {
			for (int64_t j = 0; j < bars_length; ++j) {
				if (i != j) {
					const auto index = static_cast<size_t>(j);
					const auto weighted_value =
						(*bars)[outer_index] / m_monstercat_smoothing_weights[static_cast<size_t>(std::abs(i - j))];

					// Note: do not use max here, since it's actually slower.
					// Separating the assignment from the comparison avoids an
					// unneeded assignment when (*bars)[index] is the largest
					// which
					// is often
					if ((*bars)[index] < weighted_value)
						(*bars)[index] = weighted_value;
				}
			}
		}
	}
}

void spectrum_visualizer::apply_falloff(const doublev &bars, doublev *falloff_bars) const
{
	// Screen size has change which means previous falloff values are not valid
	if (falloff_bars->size() != bars.size()) {
		*falloff_bars = bars;
		return;
	}

	for (auto i = 0u; i < bars.size(); ++i) {
		// falloff should always by at least one
		auto falloff_value = std::min((*falloff_bars)[i] * m_cfg->falloff_weight, (*falloff_bars)[i] - 1);

		(*falloff_bars)[i] = std::max(falloff_value, bars[i]);
	}
}

void spectrum_visualizer::calculate_moving_average_and_std_dev(double new_value, size_t max_number_of_elements,
															   doublev *old_values, double *moving_average,
															   double *std_dev) const
{
	if (old_values->size() > max_number_of_elements)
		old_values->erase(old_values->begin());

	old_values->push_back(new_value);

	auto sum = std::accumulate(old_values->begin(), old_values->end(), 0.0);
	*moving_average = sum / old_values->size();

	auto squared_summation = std::inner_product(old_values->begin(), old_values->end(), old_values->begin(), 0.0);
	*std_dev = std::sqrt((squared_summation / old_values->size()) - std::pow(*moving_average, 2));
}

void spectrum_visualizer::scale_bars(int32_t height, doublev *bars)
{
	if (bars->empty())
		return;

	const auto max_height_iter = std::max_element(bars->begin(), bars->end());

	// max number of elements to calculate for moving average
	const auto max_number_of_elements = static_cast<size_t>(
		((constants::auto_scale_span * m_cfg->sample_rate) / (static_cast<double>(m_cfg->sample_size))) * 2.0);

	double std_dev = 0.0;
	double moving_average = 0.0;
	calculate_moving_average_and_std_dev(*max_height_iter, max_number_of_elements, &m_previous_max_heights,
										 &moving_average, &std_dev);

	maybe_reset_scaling_window(*max_height_iter, max_number_of_elements, &m_previous_max_heights, &moving_average,
							   &std_dev);

	auto max_height = moving_average + (2 * std_dev);
	max_height = std::max(max_height, 1.0); // avoid division by zero when
	// height is zero, this happens when
	// the sound is muted

	for (double &bar : *bars) {
		bar = std::min(static_cast<double>(height - 1), ((bar / max_height) * height) - 1);
	}
}

void spectrum_visualizer::maybe_reset_scaling_window(double current_max_height, size_t max_number_of_elements,
													 doublev *values, double *moving_average, double *std_dev)
{
	const auto reset_window_size = (constants::auto_scaling_reset_window * max_number_of_elements);
	// Current max height is much larger than moving average, so throw away most
	// values re-calculate
	if (static_cast<double>(values->size()) > reset_window_size) {
		// get average over scaling window
		auto average_over_reset_window =
			std::accumulate(values->begin(), values->begin() + static_cast<int64_t>(reset_window_size), 0.0) /
			reset_window_size;

		// if short term average very different from long term moving average,
		// reset window and re-calculate
		if (std::abs(average_over_reset_window - *moving_average) >
			(constants::deviation_amount_to_reset * (*std_dev))) {
			values->erase(values->begin(),
						  values->begin() + static_cast<int64_t>((static_cast<double>(values->size()) *
																  constants::auto_scaling_erase_percent)));

			calculate_moving_average_and_std_dev(current_max_height, max_number_of_elements, values, moving_average,
												 std_dev);
		}
	}
}

void spectrum_visualizer::create_spectrum_bars(fftw_complex *fftw_output, size_t fftw_results, int32_t win_height,
											   uint32_t number_of_bars, doublev *bars, doublev *bars_falloff)
{
	// cut off frequencies only have to be re-calculated if number of bars
	// change
	if (m_last_bar_count != number_of_bars) {
		recalculate_cutoff_frequencies(number_of_bars, &m_low_cutoff_frequencies, &m_high_cutoff_frequencies,
									   &m_frequency_constants_per_bin);
		m_last_bar_count = number_of_bars;
	}

	// Separate the frequency spectrum into bars, the number of bars is based on
	// screen width
	generate_bars(number_of_bars, fftw_results, m_low_cutoff_frequencies, m_high_cutoff_frequencies, fftw_output, bars);

	// smoothing
	smooth_bars(bars);

	// scale bars
	scale_bars(win_height, bars);

	// falloff, save values for next falloff run
	apply_falloff(*bars, bars_falloff);
}

void spectrum_visualizer::recalculate_cutoff_frequencies(uint32_t number_of_bars, uint32v *low_cutoff_frequencies,
														 uint32v *high_cutoff_frequencies, doublev *freqconst_per_bin)
{
	auto freq_const =
		std::log10((m_cfg->low_cutoff_freq / m_cfg->high_cutoff_freq)) / ((1.0 / number_of_bars + 1.0) - 1.0);

	(*low_cutoff_frequencies) = std::vector<uint32_t>(number_of_bars + 1);
	(*high_cutoff_frequencies) = std::vector<uint32_t>(number_of_bars + 1);
	(*freqconst_per_bin) = std::vector<double>(number_of_bars + 1);

	for (auto i = 0u; i <= number_of_bars; i++) {
		(*freqconst_per_bin)[i] =
			static_cast<double>(m_cfg->high_cutoff_freq) *
			std::pow(10.0, (freq_const * -1) + (((i + 1.0) / (number_of_bars + 1.0)) * freq_const));

		auto frequency = (*freqconst_per_bin)[i] / (m_cfg->sample_rate / 2.0);

		(*low_cutoff_frequencies)[i] =
			static_cast<uint32_t>(std::floor(frequency * static_cast<double>(m_cfg->sample_size) / 4.0));

		if (i > 0) {
			if ((*low_cutoff_frequencies)[i] <= (*low_cutoff_frequencies)[i - 1]) {
				(*low_cutoff_frequencies)[i] = (*low_cutoff_frequencies)[i - 1] + 1;
			}
			(*high_cutoff_frequencies)[i - 1] = (*low_cutoff_frequencies)[i - 1];
		}
	}
}

void spectrum_visualizer::generate_bars(uint32_t number_of_bars, size_t fftw_results,
										const uint32v &low_cutoff_frequencies, const uint32v &high_cutoff_frequencies,
										const fftw_complex *fftw_output, doublev *bars) const
{
	if (bars->size() != number_of_bars) {
		bars->resize(number_of_bars, 0.0);
	}

	for (auto i = 0u; i < number_of_bars; i++) {
		double freq_magnitude = 0.0;
		for (auto cutoff_freq = low_cutoff_frequencies[i];
			 cutoff_freq <= high_cutoff_frequencies[i] && cutoff_freq < fftw_results; ++cutoff_freq) {
			freq_magnitude += std::sqrt((fftw_output[cutoff_freq][0] * fftw_output[cutoff_freq][0]) +
										(fftw_output[cutoff_freq][1] * fftw_output[cutoff_freq][1]));
		}
		(*bars)[i] = freq_magnitude / (high_cutoff_frequencies[i] - low_cutoff_frequencies[i] + 1);

		/* boost high freqs */
		(*bars)[i] *= (std::log2(2 + i) * (100.f / number_of_bars));
		(*bars)[i] = std::pow((*bars)[i], 0.5);
	}
}
}
