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
#include "../util.hpp"
#include "audio_visualizer.hpp"
#include <kiss_fftr.h>
#include <vector>

#define DEAD_BAR_OFFSET 5 /* The last five bars seem to always be silent, so we cut them off */

/* Save some writing */
using doublev = std::vector<double>;
using uint32v = std::vector<uint32_t>;

namespace audio {

class spectrum_visualizer : public audio_visualizer {
    uint32_t m_last_bar_count;
    bool m_sleeping = false;
    float m_sleep_count = 0.f;
    /* fft calculation vars */
    size_t m_fftw_results;
    double *m_fftw_input_left;
    double *m_fftw_input_right;

    /* Frequency cutoff variables */
    uint32v m_low_cutoff_frequencies;
    uint32v m_high_cutoff_frequencies;
    doublev m_frequency_constants_per_bin;

    void smooth_bars(doublev *bars);
    void apply_falloff(const doublev &bars, doublev *falloff_bars) const;
    void calculate_moving_average_and_std_dev(double new_value, size_t max_number_of_elements, doublev *old_values,
                                              double *moving_average, double *std_dev) const;
    void maybe_reset_scaling_window(double current_max_height, size_t max_number_of_elements, doublev *values,
                                    double *moving_average, double *std_dev);
    void scale_bars(int32_t height, doublev *bars);
    void sgs_smoothing(doublev *bars);
    void monstercat_smoothing(doublev *bars);

protected:
    /* New values are smoothly copied over if smoothing is used
     * otherwise they're directly copied */
    doublev m_bars_left, m_bars_right, m_bars_left_new, m_bars_right_new;
    doublev m_bars_falloff_left, m_bars_falloff_right;
    doublev m_previous_max_heights;
    doublev m_monstercat_smoothing_weights;

public:
    explicit spectrum_visualizer(source::config *cfg);

    ~spectrum_visualizer() override;

    void update() override;

    void tick(float seconds) override;
};

}
