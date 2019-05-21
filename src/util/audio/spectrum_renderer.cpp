/**
 * This file is part of spectraliser
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectraliser
 */

#ifdef LINUX /* open() & read() */

#include <fcntl.h>
#include <unistd.h>
#include <limits>

#endif

#include "spectrum_renderer.hpp"
#include "../../source/visualiser_source.hpp"

void spectrum_renderer::setup(source::config* config)
{
    m_fps = config->fps;
    m_samples_count = config->sample_rate / m_fps;

    if (config->stereo)
        m_samples_count *= 2;

    /* FFTW setup */
    m_fftw_results = m_samples_count / 2 + 1;
    m_fftw_input = static_cast<double*>(fftw_malloc(sizeof(double) * m_samples_count));
    m_fftw_output = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * m_fftw_results));
    m_fftw_plan = fftw_plan_dft_r2c_1d(m_samples_count, m_fftw_input, m_fftw_output, FFTW_EXHAUSTIVE);

    if (m_freq_magnitudes)
        brealloc(m_freq_magnitudes, m_fftw_results * sizeof(int16_t));
    else
        m_freq_magnitudes = static_cast<int16_t*>(bzalloc(sizeof(int16_t) * m_fftw_results));

    switch (config->mode) {
        default:
        case VISUAL_BARS:
            draw = &spectrum_renderer::draw_frequency_spectrum;
            draw_stereo = &spectrum_renderer::draw_frequency_spectrum_stereo;
            break;
        case VISUAL_WIRE: /* TODO */
            draw = nullptr;
            draw_stereo = nullptr;
            break;
    }

#ifdef LINUX
    if (strlen(config->fifo_path) > 0 && (m_fifo = open(config->fifo_path, O_RDONLY, O_NONBLOCK)) < 0)
        blog(LOG_INFO, "[spectraliser] Couldn't open fifo '%s'", config->fifo_path);

    if (m_sample_buf)
        brealloc(m_sample_buf, sizeof(int16_t) * m_samples_count);
    else
        m_sample_buf = static_cast<int16_t*>(bzalloc(sizeof(int16_t) * m_samples_count));
#endif
}

spectrum_renderer::~spectrum_renderer()
{
    bfree(m_sample_buf);
    bfree(m_freq_magnitudes);
    fftw_free(m_fftw_input);
    fftw_free(m_fftw_output);
    fftw_free(m_fftw_plan);

    m_fftw_input = nullptr;
    m_fftw_input = nullptr;
    m_fftw_plan = nullptr;
    m_sample_buf = nullptr;
    m_freq_magnitudes = nullptr;
}

#ifdef LINUX

void spectrum_renderer::render_fifo(gs_effect_t* effect, source::config* config)
{
    std::lock_guard<std::mutex> l(m_fifo_mutex);
    /* Basically 1:1 arybczak's code */
    if (m_fifo < 0)
        return;

    if (config->stereo) {
//        auto chan_samples = samples_read / 2;
//        int16_t buf_left[chan_samples], buf_right[chan_samples];
//        for (ssize_t i = 0, j = 0; i < samples_read; i += 2, ++j)
//        {
//            buf_left[j] = m_samples[i];
//            buf_right[j] = m_samples[i+1];
//        }
        //size_t half_height = config->cy / 2;
        //(this->*draw_stereo)(buf_left, buf_right, chan_samples, half_height, config);
    } else {
        if (draw)
            (this->*draw)(config);
    }
}

void spectrum_renderer::draw_frequency_spectrum(source::config* cfg)
{
    uint32_t pos_x = 0, final_bar_height = 0;
    int bar_height;
    int bins_per_bar = m_fftw_results / cfg->detail * 7 / 10;
    for (int i = 0; i < cfg->detail; i++) {
        bar_height = 0;
        for (int j = 0; j < bins_per_bar; ++j)
            bar_height += m_freq_magnitudes[(int)(i * bins_per_bar + j)];
        pos_x = i * (cfg->bar_width + cfg->bar_space);

        // Buff higher frequencies.
        bar_height *= log2(2 + i) * 100.0 / cfg->detail;
        // Moderately normalize the heights.
        bar_height = pow(bar_height, 0.5);

        final_bar_height = UTIL_CLAMP(5, floor(bar_height), cfg->bar_height);
        /* Draw bars with obs graphic functions */
        gs_matrix_push();
        /* Offset to bar x position */
        gs_matrix_translate3f(pos_x, 0, 0);

        gs_draw_sprite(nullptr, 0, cfg->bar_width, final_bar_height);
        gs_matrix_pop();
    }
}


void
spectrum_renderer::draw_frequency_spectrum_stereo(int16_t* buf_left, int16_t* buf_right, ssize_t samples, size_t height,
                                                  source::config* cfg)
{
    //draw_frequency_spectrum(buf_left, samples, 0, height, cfg);
    //draw_frequency_spectrum(buf_right, samples, height, cfg->cy - height, cfg);
}

void spectrum_renderer::read_fifo(source::config* cfg)
{
    if (m_fifo < 0)
        return;
    std::lock_guard<std::mutex> l(m_fifo_mutex);
    ssize_t data = read(m_fifo, m_sample_buf, m_samples_count * sizeof(int16_t));
    if (data < 0) {
        blog(LOG_DEBUG, "Fifo is empty");
        return;
    }

    //const ssize_t samples_read = data / sizeof(int16_t);
    m_auto_scale_multiplier += 1.0 / m_fps;

    /* Frequency adjusting (I think) */
    int32_t tmp;
    for (size_t i = 0; i < m_samples_count; i ++) {
        tmp = m_sample_buf[i];

        double scale = std::numeric_limits<int16_t>::min();
        scale /= tmp;
        scale = fabs(scale);

        if (scale < m_auto_scale_multiplier)
            m_auto_scale_multiplier = scale;

        if (m_auto_scale_multiplier <= 50.0) // limit the auto scale
            tmp *= m_auto_scale_multiplier;
        if (tmp < std::numeric_limits<int16_t>::min())
            m_sample_buf[i] = std::numeric_limits<int16_t>::min();
        else if (tmp > std::numeric_limits<int16_t>::max())
            m_sample_buf[i] = std::numeric_limits<int16_t>::max();
        else
            m_sample_buf[i] = tmp;

        m_fftw_input[i] = m_sample_buf[i];
    }

    fftw_execute(m_fftw_plan);

    /* Calculate bar amplitudes */
    for (size_t i = 0; i < m_fftw_results; ++i) {
        m_freq_magnitudes[i] =
                sqrt(m_fftw_output[i][0] * m_fftw_output[i][0] + m_fftw_output[i][1] * m_fftw_output[i][1]) / 2e4 *
                cfg->bar_height;
    }

    switch(cfg->filter_mode) {
        case BAR_FILTER_WAVES:
            apply_wave_filter(cfg->bar_filter_arg);
            break;
        case BAR_FILTER_MONSTERCAT:
            apply_monstercat_filter(cfg->bar_filter_arg);
            break;
        default:
        case BAR_FILTER_NONE:;
    }
}

void spectrum_renderer::apply_wave_filter(double arg)
{
    int m_y, de, z;
    for (z = 0; z < m_fftw_results; z++) { // waves
        m_freq_magnitudes[z] = m_freq_magnitudes[z] / arg;
        //if (f[z] < 1) f[z] = 1;
        for (m_y = z - 1; m_y >= 0; m_y--) {
            de = z - m_y;
            m_freq_magnitudes[m_y] = UTIL_MAX(m_freq_magnitudes[z] - pow(de, 2), m_freq_magnitudes[m_y]);
        }
        for (m_y = z + 1; m_y < m_fftw_results; m_y++) {
            de = m_y - z;
            m_freq_magnitudes[m_y] = UTIL_MAX(m_freq_magnitudes[z] - pow(de, 2), m_freq_magnitudes[m_y]);
        }
    }
}

void spectrum_renderer::apply_monstercat_filter(double arg)
{
    int m_y, de, z;
    for (z = 0; z < m_fftw_results; z++) {
        //if (f[z] < 1)f[z] = 1;
        for (m_y = z - 1; m_y >= 0; m_y--) {
            de = z - m_y;
            m_freq_magnitudes[m_y] = UTIL_MAX(m_freq_magnitudes[z] / pow(arg, de), m_freq_magnitudes[m_y]);
        }
        for (m_y = z + 1; m_y < m_fftw_results; m_y++) {
            de = m_y - z;
            m_freq_magnitudes[m_y] = UTIL_MAX(m_freq_magnitudes[z] / pow(arg, de), m_freq_magnitudes[m_y]);
        }
    }
}

#endif
