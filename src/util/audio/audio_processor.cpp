/**
 * This file is part of spectralizer
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectralizer
 */

#include "audio_processor.hpp"
#include "../../source/visualizer_source.hpp"

/*
    Most processing is reused from cava
    https://github.com/karlstav/cava/blob/master/cava.c
 */

namespace audio
{

    double smoothing_values[] = {0.8, 0.8, 1, 1, 0.8, 0.8, 1, 0.8, 0.8, 1, 1, 0.8, 1, 1, 0.8, 0.6, 0.6, 0.7, 0.8, 0.8,
                                 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8,
                                 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.7, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6,
                                 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6};

    audio_processor::audio_processor(source::config* cfg)
    {
        update(cfg);
    }

    audio_processor::~audio_processor()
    {
        clean_up();
    }

    void audio_processor::update(source::config* cfg)
    {
        m_buf_size = cfg->buffer_size * cfg->buffer_size;
        m_samples = 2 * (m_buf_size / 2 + 1);
        m_channels = cfg->stereo ? 2 : 1;
        m_eq_dist = 64. / cfg->detail;
        m_sleep_counter = 0.f;

        /* Utility arrays*/
        m_fall_off = static_cast<int*>(bzalloc(cfg->detail * sizeof(int) * 2));
        m_last_freqs = static_cast<int*>(bzalloc(cfg->detail * sizeof(int) * 2));
        m_low_freq_cut = static_cast<int*>(bzalloc(cfg->detail * sizeof(int)));
        m_high_freq_cut = static_cast<int*>(bzalloc(cfg->detail * sizeof(int)));
        m_freq_mem = static_cast<int*>(bzalloc(cfg->detail * sizeof(int) * 2));

        m_freq_peak = static_cast<float*>(bzalloc(cfg->detail * sizeof(float)));
        m_freq_weight = static_cast<double*>(bzalloc(cfg->detail * sizeof(double)));

        m_fftw_in_l = static_cast<double*>(bzalloc(m_samples * sizeof(double)));
        m_fftw_in_r = static_cast<double*>(bzalloc(m_samples * sizeof(double)));

        /* setup fftw values */
        m_fftw_out_l = static_cast<fftw_complex*>(bzalloc(m_samples * sizeof(fftw_complex)));
        m_fftw_out_r = static_cast<fftw_complex*>(bzalloc(m_samples * sizeof(fftw_complex)));

        m_fftw_plan_l = fftw_plan_dft_r2c_1d(m_buf_size, m_fftw_in_l, m_fftw_out_l, FFTW_MEASURE);
        m_fftw_plan_r = fftw_plan_dft_r2c_1d(m_buf_size, m_fftw_in_r, m_fftw_out_r, FFTW_MEASURE);

        /* zero buffers */
        bzero(m_audio_out_r, sizeof(int16_t) * AUDIO_SIZE);
        bzero(m_audio_out_l, sizeof(int16_t) * AUDIO_SIZE);

        bzero(m_fftw_out_l, sizeof(fftw_complex) * m_samples);
        bzero(m_fftw_out_r, sizeof(fftw_complex) * m_samples);

        bzero(m_fftw_in_l, sizeof(double) * m_samples);
        bzero(m_fftw_in_r, sizeof(double) * m_samples);

        /* Misc caluclations, that only have to be done once per updated settings */
        double frequency_constant, pot, fre, fc;
        int n, smooth_index;

        m_current_gravity = cfg->gravity * ((float) cfg->bar_height / 2160) * pow((60 / (float) cfg->fps), 2.5);
        frequency_constant = log10((float) cfg->freq_cutoff_high) / (1.f / (cfg->detail + 1.f) - 1);

        /* Caculate cut-off frequencies & and weigh frequencies */
        for (n = 0; n < cfg->detail + 1; n++) {
            pot += frequency_constant * (-1);
            pot += (n + 1.f) / (cfg->detail + 1.f) * frequency_constant;
            fc = cfg->freq_cutoff_high * pow(10, pot);
            fre = fc / (cfg->sample_rate / 2);

            /* */
            m_low_freq_cut[n] = fre * (m_buf_size / 2) + 1;
            if (n > 0) {
                m_high_freq_cut[n - 1] = m_low_freq_cut[n] - 1;

                /* Adjust spectrum if exp function "clumps" (idk what that means) */
                if (m_low_freq_cut[n] <= m_low_freq_cut[n - 1])
                    m_low_freq_cut[n] = m_low_freq_cut[n - 1] + 1;
                m_high_freq_cut[n - 1] = m_low_freq_cut[n] - 1;
            }

            /* Weigh frequencies */
            /* Smooth index grabs a smoothing value out of the predefined array of values */
            smooth_index = UTIL_CLAMP(0, (int) floor(n * m_eq_dist), cfg->detail);
            m_freq_weight[n] = pow(fc, .85);
            m_freq_weight[n] *= (float) cfg->bar_height / pow(2, 28);
            m_freq_weight[n] *= smoothing_values[smooth_index];
        }
    }

    void audio_processor::clean_up()
    {
        /* Free allocated memory */
        bfree(m_fftw_in_l);
        bfree(m_fftw_in_r);
        bfree(m_fftw_out_l);
        bfree(m_fftw_out_r);

        fftw_destroy_plan(m_fftw_plan_l);
        fftw_destroy_plan(m_fftw_plan_r);

        /* Free utility arrays */
        bfree(m_fall_off);
        bfree(m_last_freqs);
        bfree(m_low_freq_cut);
        bfree(m_high_freq_cut);
        bfree(m_freq_mem);
        bfree(m_freq_peak);
        bfree(m_freq_weight);

        /* Set to null for good measure */
        m_fall_off = nullptr;
        m_last_freqs = nullptr;
        m_low_freq_cut = nullptr;
        m_high_freq_cut = nullptr;
        m_freq_mem = nullptr;
        m_freq_peak = nullptr;
        m_freq_weight = nullptr;

        m_fftw_in_l = nullptr;
        m_fftw_in_r = nullptr;
        m_fftw_out_l = nullptr;
        m_fftw_out_r = nullptr;
    }

    void audio_processor::tick(float seconds, source::config* cfg)
    {
        int i, o;
        /* Process collected audio */
        bool silence = true;
        for (int i = 0; i < m_samples; i++) {
            if (i < m_buf_size && i < AUDIO_SIZE) {
                m_fftw_in_l[i] = m_audio_out_l[i];

                if (m_channels > 1)
                    m_fftw_in_r[i] = m_audio_out_r[i];
                if (silence && (m_fftw_in_l[i] > 0 || m_fftw_in_r[i] > 0))
                    silence = false;
            } else {
                m_fftw_in_l[i] = 0;
                if (m_channels > 1)
                    m_fftw_in_r[i] = 0;
            }
        }

        if (silence)
            m_sleep_counter += seconds;
        else
            m_sleep_counter = 0;

        if (m_sleep_counter < 5) { /* Audio for >5 seconds -> can process */
            if (m_channels > 1) {
                fftw_execute(m_fftw_plan_l);
                fftw_execute(m_fftw_plan_r);
                separate_freq_bands(cfg, cfg->detail, true);
                separate_freq_bands(cfg, cfg->detail, false);
            } else {
                fftw_execute(m_fftw_plan_l);
                separate_freq_bands(cfg, cfg->detail, true);
            }
        } else {
#ifdef DEBUG
            blog(LOG_DEBUG, "[spectralizer] No sound for 3 seconds, sleeping.");
#endif
            continue;
        }

        /* Additional filtering */
        if (cfg->filter_mode == source::FILTER_MCAT) {
            if (m_channels > 1) {
                apply_monstercat_filter(cfg, &m_freq_l);
                apply_monstercat_filter(cfg, &m_freq_r);
            } else {
                apply_monstercat_filter(cfg, &m_freq_l);
            }
        } else if (cfg->filter_mode == source::FILTER_WAVES) {
            if (m_channels > 1) {
                apply_wave_filter(cfg, &m_freq_l);
                apply_wave_filter(cfg, &m_freq_r);
            } else {
                apply_wave_filter(cfg, &m_freq_l);
            }
        }


    }

    void audio_processor::apply_falloff(source::config *cfg, int* t) {
        if (m_current_gravity > 0) {
            for (int o = 0; o < cfg->detail; o++) {
                if (t[o] < m_last_freqs[o])
            }
        }
    }

    void audio_processor::separate_freq_bands(source::config* cfg, uint16_t detail, bool left_channel)
    {
        int o, i;
        double peak, amplitude, tmp;
        for (o = 0; o < detail; o++) {
            peak = 0;
            for (i = m_low_freq_cut[o]; i <= m_high_freq_cut[o]; i++) {
                if (left_channel)
                    amplitude = hypot(m_fftw_out_l[i][0], m_fftw_out_l[i][1]);
                else
                    amplitude = hypot(m_fftw_out_r[i][0], m_fftw_out_r[i][1]);
                peak += amplitude;
            }

            peak = peak / (m_high_freq_cut[o] - m_low_freq_cut[o] + 1); /* Averaging */
            tmp = peak / cfg->sens * m_freq_weight[o];

            if (tmp <= cfg->ignore)
                tmp = 0;

            if (left_channel)
                m_freq_l[o] = tmp;
            else
                m_freq_r[o] = tmp;
        }
    }

    void audio_processor::apply_monstercat_filter(source::config *cfg, int* t)
    {
        int i, m_y, de
        for (i = 0; i < cfg->detail; i++) {
            for (m_y = z - 1; m_y >= 0; m_y--) {
                de = z - m_y;
                t[m_y] = maxtarr[z] / pow(cfg->mcat_strength, de), t[m_y]);
            }

            for (m_y = z + 1; m_y < cfg->detail; m_y++) {
                de = m_y - z;
                t[m_y] = max(t[z] / pow(cfg->mcat_strength, de), t[m_y]);
            }
        }
    }

    void audio_processor::apply_wave_filter(source::config *cfg, int* t) {
        int i, m_y, de;
        for (z = 0; z < cfg->detail; z++) {
            for (m_y = z - 1; m_y >= 0; m_y--) {
                de = z - m_y;
                t[m_y] = max(t[z] - pow(cfg->mcat_strength, 2), t[m_y]);
            }

            for (m_y = z + 1; m_y < cfg->detail; m_y++) {
                de = m_y - z;
                t[m_y] = max(t[z] - pow(cfg->mcat_strength, 2), t[m_y]);
            }
        }
    }

    uint8_t audio_processor::get_channels()
    {
        return m_channels;
    }

    int32_t audio_processor::get_buffer_size()
    {
        return m_buf_size;
    }

} /* namespace audio */
