/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#pragma once
#include <fftw3.h>
#include <vector>
#include "audio_visualizer.hpp"
#include "../util.hpp"

/* Save some writing */
using doublev = std::vector<double>;
using uint32v = std::vector<uint32_t>;

namespace audio
{

    class spectrum_visualizer : public audio_visualizer
    {
        uint32_t m_last_bar_count;
        bool m_sleeping = false;
        float m_sleep_count = 0.f;
        /* fft calculation vars */
        size_t m_fftw_results;
        double *m_fftw_input_left;
        double *m_fftw_input_right;

        fftw_complex *m_fftw_output_left;
        fftw_complex *m_fftw_output_right;

        fftw_plan m_fftw_plan_left;
        fftw_plan m_fftw_plan_right;

        /* Frequency cutoff variables */
        uint32v m_low_cutoff_frequencies;
        uint32v m_high_cutoff_frequencies;
        doublev m_frequency_constants_per_bin;

        uint64_t m_silent_runs; /* determines sleep state */
        /* New values are smoothly copied over if smoothing is used
         * otherwise they're directly copied */
        doublev m_bars_left, m_bars_right, m_bars_left_new, m_bars_right_new;
        doublev m_bars_falloff_left, m_bars_falloff_right;
        doublev m_previous_max_heights;
        doublev m_monstercat_smoothing_weights;

        bool prepare_fft_input(pcm_stereo_sample *buffer, uint32_t sample_size,
                               double *fftw_input, channel_mode channel_mode);

        void create_spectrum_bars(fftw_complex *fftw_output,
                                          size_t fftw_results, int32_t win_height,
                                          uint32_t number_of_bars,
                                          doublev *bars,
                                          doublev *bars_falloff);

        void generate_bars(uint32_t number_of_bars, size_t fftw_results,
                           const uint32v &low_cutoff_frequencies,
                           const uint32v &high_cutoff_frequencies,
                           const fftw_complex *fftw_output,
                           doublev *bars) const;

        void recalculate_cutoff_frequencies(
                uint32_t number_of_bars, uint32v *low_cutoff_frequencies,
                uint32v *high_cutoff_frequencies,
                doublev *freqconst_per_bin);
        void smooth_bars(doublev *bars);
        void apply_falloff(const doublev &bars,
                           doublev *falloff_bars) const;
        void calculate_moving_average_and_std_dev(double new_value,
                                                  size_t max_number_of_elements,
                                                  doublev *old_values,
                                                  double *moving_average,
                                                  double *std_dev) const;
        void maybe_reset_scaling_window(double current_max_height,
                                        size_t max_number_of_elements,
                                        doublev *values,
                                        double *moving_average, double *std_dev);
        void scale_bars(int32_t height, doublev *bars);
        void sgs_smoothing(doublev *bars);
        void monstercat_smoothing(doublev *bars);

    public:
        explicit spectrum_visualizer(source::config* cfg);

        ~spectrum_visualizer() override;

        void update() override;

        void tick(float seconds) override;

        void render(gs_effect_t* effect) override;
    };

}
