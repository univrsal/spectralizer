/* Copyright (C) 2014 Rainmeter Project Developers
 *
 * This Source Code Form is subject to the terms of the GNU General Public
 * License; either version 2 of the License, or (at your option) any later
 * version. If a copy of the GPL was not distributed with this file, You can
 * obtain one at <https://www.gnu.org/licenses/gpl-2.0.html>. */

/*
 * Adjusted from original rainmeter code to work inside obs
 */

#pragma once
#include <cstdio>
#include <cmath>
#include <cassert>
#include <vector>
#include <string>
#include "../util.hpp"
#include "kiss_fft130/kiss_fftr.h"
#include "../../source/visualizer_source.hpp"

namespace audio {
struct measure {
    enum Port
    {
        PORT_OUTPUT,
        PORT_INPUT,
    };

    enum Type
    {
        TYPE_RMS,
        TYPE_PEAK,
        TYPE_FFT,
        TYPE_BAND,
        TYPE_FFTFREQ,
        TYPE_BANDFREQ,
        // ... //
        NUM_TYPES
    };

    enum Format
    {
        FMT_INVALID,
        FMT_PCM_S16,
        FMT_PCM_F32,
        // ... //
        NUM_FORMATS
    };

    struct BandInfo
    {
        float freq;
        float x;
    };

    struct measure_global_config *m_global_config = nullptr;
    struct measure_config *m_cfg;

    Port					m_port;						// port specifier (parsed from options)
    channel_mode            m_channel;					// channel specifier (parsed from options)
    Type					m_type;						// data type specifier (parsed from options)
    Format					m_format;					// format specifier (detected in init)
    int						m_fftSize;					// size of FFT (parsed from options)
    int						m_fftOverlap;				// number of samples between FFT calculations
    int						m_fftIdx;					// FFT index to retrieve (parsed from options)
    int						m_nBands;					// number of frequency bands (parsed from options)
    int						m_bandIdx;					// band index to retrieve (parsed from options)
    double					m_gainPeak;					// peak gain (parsed from options)
    double					m_freqMin;					// min freq for band measurement
    double					m_freqMax;					// max freq for band measurement
    void*					m_skin;						// skin pointer
    double					m_rms[MAX_AUDIO_CHANNELS];		// current RMS levels
    double					m_peak[MAX_AUDIO_CHANNELS];		// current peak levels
    double					m_pcMult;					// performance counter inv frequency
    uint64_t     			m_pcFill;					// performance counter on last full buffer
    uint64_t    			m_pcPoll;					// performance counter on last device poll
    kiss_fftr_cfg			m_fftCfg[MAX_AUDIO_CHANNELS];		// FFT states for each channel
    float*					m_fftIn[MAX_AUDIO_CHANNELS];		// buffer for each channel's FFT input
    float*					m_fftOut[MAX_AUDIO_CHANNELS];		// buffer for each channel's FFT output
    float*					m_fftKWdw;					// window function coefficients
    float*					m_fftTmpIn;					// temp FFT processing buffer
    kiss_fft_cpx*			m_fftTmpOut;				// temp FFT processing buffer
    int						m_fftBufW;					// write index for input ring buffers
    int						m_fftBufP;					// decremental counter - process FFT at zero
    float*					m_bandFreq;					// buffer of band max frequencies
    float*					m_bandOut[MAX_AUDIO_CHANNELS];	// buffer of band values

    measure();

    double tick(source::config *cfg);
    void init(struct measure_config *cfg);
    void update(struct measure_config *cfg);
};

/* Contains values shared across all audio measures in
 * a visualizer */
struct measure_global_config {
    measure_global_config();
    int						m_envRMS[2];				// RMS attack/decay times in ms (parsed from options)
    int						m_envPeak[2];				// peak attack/decay times in ms (parsed from options)
    int						m_envFFT[2];				// FFT attack/decay times in ms (parsed from options)
    double					m_gainRMS;					// RMS gain (parsed from options)
    double					m_sensitivity;				// dB range for FFT/Band return values (parsed from options)
    float					m_kRMS[2];					// RMS attack/decay filter constants
    float					m_kPeak[2];					// peak attack/decay filter constants
    float					m_kFFT[2];					// FFT attack/decay filter constants
};

/* Contains values specific to each measure in a
 * visualizer */
struct measure_config {
    int fft_size;
    int fft_overlap;
    int band_count;
    int fft_index;
    int band_index;
    double freq_min, freq_max;
    measure::Type type;
    /* These can most likely be shared */
    int rms_attack, rms_decay, peak_attack, peak_decay;
    int fft_attack, fft_decay;
    double rms_gain, peak_gain, sensitivity;
};


}
