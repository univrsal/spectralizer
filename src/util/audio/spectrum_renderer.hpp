/**
 * This file is part of spectraliser
 * which is licensed under the GPL v2.0
 * See LICENSE or http://www.gnu.org/licenses
 * github.com/univrsal/spectraliser
 */

/* Heavy inspiration taken from ncmpcpp
 * https://github.com/arybczak/ncmpcpp/blob/master/src/screens/visualizer.h
 */

#pragma once
#include <mutex>
#include <vector>
#include <fftw3.h>
#include <graphics/graphics.h>

namespace source {
    struct config;
}

enum bar_filter {
    BAR_FILTER_NONE,
    BAR_FILTER_MONSTERCAT,
    BAR_FILTER_WAVES
};

class spectrum_renderer
{
    /* FFTW stuff */
    size_t m_fftw_results;
    double* m_fftw_input;
    fftw_complex* m_fftw_output;
    fftw_plan m_fftw_plan;
    int16_t* m_freq_magnitudes = nullptr; /* Size of m_fftw_results */

    /* Other stuff */
    double m_auto_scale_multiplier;
    uint16_t m_fps = 0;
    size_t m_samples_count = 0;
    bar_filter m_filter_mode = BAR_FILTER_NONE;

    /* Function pointer to draw method */
    void (spectrum_renderer::*draw)(source::config* cfg);
    void (spectrum_renderer::*draw_stereo)(int16_t*, int16_t*, ssize_t, size_t, source::config* cfg);

#ifdef LINUX
    int16_t* m_sample_buf = nullptr; /* Size of samples count */
    int m_fifo = -1;
    std::mutex m_fifo_mutex;
#endif

    /* Drawing methods */
    void draw_frequency_spectrum(source::config* cfg);
    void draw_frequency_spectrum_stereo(int16_t *buf_left, int16_t *buf_right, ssize_t samples, size_t height, source::config*
    cfg);

    /* Bar filtering
     * Source:
     * https://github.com/karlstav/cava/blob/master/cava.c#L186
     * */
    void apply_wave_filter(double arg);
    void apply_monstercat_filter(double arg);
public:
    spectrum_renderer() = default;
    ~spectrum_renderer();

    void setup(source::config* config);

#ifdef LINUX
    void render_fifo(gs_effect_t* effect, source::config* config); /* Called for rendering processed fifo data */
    void read_fifo(source::config* cfg); /* Called every frame tick to read the fifo data */
#endif


};