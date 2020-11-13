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

#include "spectrum_visualizer.hpp"
#include "../../source/visualizer_source.hpp"
#include "audio_source.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace audio {
spectrum_visualizer::spectrum_visualizer(source::config *cfg)
    : audio_visualizer(cfg),
      m_last_bar_count(0)
{
    update();
}

spectrum_visualizer::~spectrum_visualizer()
{
}

void spectrum_visualizer::update()
{
    audio_visualizer::update();
    m_monstercat_smoothing_weights.clear(); /* Force recomputing of smoothing */

    m_fftw_results = (size_t)m_cfg->sample_size / 2 + 1;
}

void spectrum_visualizer::tick(float seconds)
{
    if (m_sleeping) {
        m_sleep_count += seconds;
        if (m_sleep_count >= 0.25f) {
            m_sleeping = false;
            m_sleep_count = 0.f;
        }
        return;
    }

    audio_visualizer::tick(seconds);

    const auto win_height = m_cfg->bar_height;

    /* TODO make this a constant */
    auto height = win_height;
    double grav = 1 - m_cfg->gravity;

    if (m_cfg->stereo)
        height /= 2;

    if (m_cfg->stereo) {
        m_bars_right.resize(m_bars_right_new.size(), 0.0);
        for (size_t i = 0; i < m_bars_right.size(); i++) {
            m_bars_right[i] = m_bars_right[i] * m_cfg->gravity + m_bars_right_new[i] * grav;
        }
    }

    m_bars_left.resize(m_bars_left_new.size(), 0.0);
    for (size_t i = 0; i < m_bars_left.size(); i++) {
        m_bars_left[i] = m_bars_left[i] * m_cfg->gravity + m_bars_left_new[i] * grav;
    }
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
        m_monstercat_smoothing_weights.resize(bars->size());
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

    if (m_cfg->use_auto_scale) {
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
        // avoid division by zero when
        // height is zero, this happens when
        // the sound is muted
        max_height = std::max(max_height, 1.0);

        for (double &bar : *bars) {
            bar = std::min(static_cast<double>(height - 1), ((bar / max_height) * height) - 1);
        }
    } else {
        for (double &bar : *bars) {
            bar *= m_cfg->scale_size;
            bar += m_cfg->scale_boost;
        }
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

}
