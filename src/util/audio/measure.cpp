/* Copyright (C) 2014 Rainmeter Project Developers
 *
 * This Source Code Form is subject to the terms of the GNU General Public
 * License; either version 2 of the License, or (at your option) any later
 * version. If a copy of the GPL was not distributed with this file, You can
 * obtain one at <https://www.gnu.org/licenses/gpl-2.0.html>. */

/*
 * Adjusted from original rainmeter code to work inside obs
 */

#include "measure.hpp"
#include "../util.hpp"

// REFERENCE_TIME time units per second and per millisecond
#define WINDOWS_BUG_WORKAROUND	1
#define REFTIMES_PER_SEC		10000000
#define TWOPI					(2 * 3.14159265358979323846)
#define EXIT_ON_ERROR(hres)		if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(p)			if ((p) != NULL) { (p)->Release(); (p) = NULL; }
#define CLAMP01(x)				UTIL_MAX(0.0, UTIL_MIN(1.0, (x)))

#define EMPTY_TIMEOUT			0.500
#define DEVICE_TIMEOUT			1.500
#define QUERY_TIMEOUT			(1.0 / 60)

namespace audio {

    measure_global_config::measure_global_config()
        : m_gainRMS(1.0), m_sensitivity(35.0)
    {
        m_envRMS[0] = 300;
        m_envRMS[1] = 300;
        m_envPeak[0] = 50;
        m_envPeak[1] = 2500;
        m_envFFT[0] = 300;
        m_envFFT[1] = 300;
        m_kRMS[0] = 0.0f;
        m_kRMS[1] = 0.0f;
        m_kPeak[0] = 0.0f;
        m_kPeak[1] = 0.0f;
        m_kFFT[0] = 0.0f;
        m_kFFT[1] = 0.0f;
    }

    measure::measure() :
        m_port(PORT_OUTPUT),
        m_channel(CM_BOTH),
        m_type(TYPE_RMS),
        m_format(FMT_INVALID),
        m_fftSize(0),
        m_fftOverlap(0),
        m_fftIdx(-1),
        m_nBands(0),
        m_bandIdx(-1),
        m_gainPeak(1.0),
        m_freqMin(20.0),
        m_freqMax(20000.0),
        m_skin(NULL),
        m_fftKWdw(NULL),
        m_fftTmpIn(NULL),
        m_fftTmpOut(NULL),
        m_fftBufW(0),
        m_fftBufP(0),
        m_bandFreq(NULL)
    {        for (int iChan = 0; iChan < MAX_CHANNELS; ++iChan)
        {
            m_rms[iChan] = 0.0;
            m_peak[iChan] = 0.0;
            m_fftCfg[iChan] = NULL;
            m_fftIn[iChan] = NULL;
            m_fftOut[iChan] = NULL;
            m_bandOut[iChan] = NULL;
        }

        /* TODO: get performance frequency? */
        uint64_t pcFreq;
        //QueryPerformanceFrequency(&pcFreq);
        m_pcMult = 1.0;// / (double)pcFreq.QuadPart;
    }

std::vector<measure*> s_parents;

void measure::init(measure_config *cfg)
{
    /* TODO: Make sure to enforce the correct ranges on these values,
     * like in original rainmeter code, also potentially get rid of some of these
     * values, as they are the same across all measures and therefore could be
     * shared instead of saving them redunandtly here
     */
    // initialize FFT data
    m_fftSize = cfg->fft_size;

    if (m_fftSize)
        m_fftOverlap = cfg->fft_overlap;

    // initialize frequency bands
    m_nBands = cfg->band_count;

    m_freqMin = UTIL_MAX(0.0, cfg->freq_min);
    m_freqMax = UTIL_MAX(0.0, cfg->freq_max);

    // initialize the watchdog timer
    // TODO: Performance counter?
    //QueryPerformanceCounter(&m->m_pcPoll);

}

void measure::update(measure_config *cfg)
{
    // parse data type
    m_type = cfg->type;

    // parse FFT index request
    m_fftIdx = UTIL_MAX(0, cfg->fft_index);
    m_fftIdx = UTIL_MIN(m_fftSize / 2, m_fftIdx);

    // parse band index request
    m_bandIdx = UTIL_MAX(0, cfg->band_index);
    m_bandIdx = UTIL_MIN(m_nBands, m_bandIdx);

    // parse envelope values on parents only
    // TODO: calculate m_kRMS/m_kPeak/m_kFFT
//    if (!m->m_parent)
//    {
//        // (re)parse envelope values
//        m_envRMS[0] = max(0, RmReadInt(rm, L"RMSAttack", m->m_envRMS[0]));
//        m_envRMS[1] = max(0, RmReadInt(rm, L"RMSDecay", m->m_envRMS[1]));
//        m_envPeak[0] = max(0, RmReadInt(rm, L"PeakAttack", m->m_envPeak[0]));
//        m_envPeak[1] = max(0, RmReadInt(rm, L"PeakDecay", m->m_envPeak[1]));
//        m_envFFT[0] = max(0, RmReadInt(rm, L"FFTAttack", m->m_envFFT[0]));
//        m_envFFT[1] = max(0, RmReadInt(rm, L"FFTDecay", m->m_envFFT[1]));

//        // (re)parse gain constants
//        m->m_gainRMS = max(0.0, RmReadDouble(rm, L"RMSGain", m->m_gainRMS));
//        m->m_gainPeak = max(0.0, RmReadDouble(rm, L"PeakGain", m->m_gainPeak));
//        m->m_sensitivity = max(1.0, RmReadDouble(rm, L"Sensitivity", m->m_sensitivity));

//        // regenerate filter constants
//        if (m->m_wfx)
//        {
//            const double freq = m->m_wfx->nSamplesPerSec;
//            m->m_kRMS[0] = (float) exp(log10(0.01) / (freq * (double)m->m_envRMS[0] * 0.001));
//            m->m_kRMS[1] = (float) exp(log10(0.01) / (freq * (double)m->m_envRMS[1] * 0.001));
//            m->m_kPeak[0] = (float) exp(log10(0.01) / (freq * (double)m->m_envPeak[0] * 0.001));
//            m->m_kPeak[1] = (float) exp(log10(0.01) / (freq * (double)m->m_envPeak[1] * 0.001));

//            if (m->m_fftSize)
//            {
//                m->m_kFFT[0] = (float) exp(log10(0.01) / (freq / (m->m_fftSize-m->m_fftOverlap) * (double)m->m_envFFT[0] * 0.001));
//                m->m_kFFT[1] = (float) exp(log10(0.01) / (freq / (m->m_fftSize-m->m_fftOverlap) * (double)m->m_envFFT[1] * 0.001));
//            }
//        }
//    }
}


double measure::tick(source::config *cfg)
{
    measure_global_config *gc = m_global_config;
    uint64_t pcCur = 0;
    // TODO: Performance counter
    //QueryPerformanceCounter(&pcCur);

    // query the buffer
    //if (m_clCapture && (pcCur.QuadPart - m->m_pcPoll.QuadPart) * m->m_pcMult >= QUERY_TIMEOUT)
    if (true) /* TODO: run this only when we have data */
    {
        uint8_t* buffer;
        uint32_t nFrames;
        uint16_t flags;
        uint64_t pos;
        int hr;

        //while ((hr = m->m_clCapture->GetBuffer(&buffer, &nFrames, &flags, &pos, NULL)) == S_OK)
        {
            // measure RMS and peak levels
            float rms[MAX_AUDIO_CHANNELS];
            float peak[MAX_AUDIO_CHANNELS];
            for (int iChan = 0; iChan < MAX_AUDIO_CHANNELS; ++iChan)
            {
                rms[iChan] = (float)m_rms[iChan];
                peak[iChan] = (float)m_peak[iChan];
            }

            // loops unrolled 16b and mono, stereo
            if (cfg->stereo)
            {
                for (unsigned int i = 0; i< nFrames; ++i)
                {
                    float xL = (float)cfg->buffer[i].l * 1.0f / 0x7fff;
                    float sqrL = xL * xL;
                    float absL = abs(xL);
                    rms[0] = sqrL + gc->m_kRMS[(sqrL < rms[0])] * (rms[0] - sqrL);
                    peak[0] = absL + gc->m_kPeak[(absL < peak[0])] * (peak[0] - absL);
                    rms[1] = rms[0];
                    peak[1] = peak[0];
                }
            }
            else
            {
                for (unsigned int i = 0; i < nFrames; ++i)
                {
                    float xL = (float)cfg->buffer[i].l * 1.0f / 0x7fff;
                    float xR = (float)cfg->buffer[i].r * 1.0f / 0x7fff;
                    float sqrL = xL * xL;
                    float sqrR = xR * xR;
                    float absL = abs(xL);
                    float absR = abs(xR);
                    rms[0] = sqrL + gc->m_kRMS[(sqrL < rms[0])] * (rms[0] - sqrL);
                    rms[1] = sqrR + gc->m_kRMS[(sqrR < rms[1])] * (rms[1] - sqrR);
                    peak[0] = absL + gc->m_kPeak[(absL < peak[0])] * (peak[0] - absL);
                    peak[1] = absR + gc->m_kPeak[(absR < peak[1])] * (peak[1] - absR);
                }
            }

            for (int iChan = 0; iChan < MAX_CHANNELS; ++iChan) {
                m_rms[iChan] = rms[iChan];
                m_peak[iChan] = peak[iChan];
            }

            // process FFTs (optional)
            if (m_fftSize)
            {
                f32_stereo_sample* sF32 = (f32_stereo_sample*)cfg->buffer;
                i16_stereo_sample* sI16 = (i16_stereo_sample*)buffer;
                const float	scalar = (float)(1.0 / sqrt(m_fftSize));

                // TODO: proper multi channel support
                for (unsigned int iFrame = 0; iFrame < nFrames; ++iFrame)
                {
                    // fill ring buffers (demux streams)
                    (m_fftIn[0])[m_fftBufW] = (*sF32).l;
                    (m_fftIn[1])[m_fftBufW] = (*sF32).r;
                    sF32++;

                    m_fftBufW = (m_fftBufW + 1) % m_fftSize;

                    // if overlap limit reached, process FFTs for each channel
                    if (!--m_fftBufP)
                    {
                        for (unsigned int iChan = 0; iChan < 2; ++iChan)
                        {
#define AUDCLNT_BUFFERFLAGS_SILENT 0 // TODO
                            if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT))
                            {
                                // copy from the ring buffer to temp space
                                memcpy(&m_fftTmpIn[0], &(m_fftIn[iChan])[m_fftBufW], (m_fftSize - m_fftBufW) * sizeof(float));
                                memcpy(&m_fftTmpIn[m_fftSize - m_fftBufW], &m_fftIn[iChan][0], m_fftBufW * sizeof(float));

                                // apply the windowing function
                                for (int iBin = 0; iBin < m_fftSize; ++iBin)
                                {
                                    m_fftTmpIn[iBin] *= m_fftKWdw[iBin];
                                }

                                kiss_fftr(m_fftCfg[iChan], m_fftTmpIn, m_fftTmpOut);
                            }
                            else
                            {
                                memset(m_fftTmpOut, 0,m_fftSize * sizeof(kiss_fft_cpx));
                            }

                            // filter the bin levels as with peak measurements
                            for (int iBin = 0; iBin < m_fftSize; ++iBin)
                            {
                                float x0 = (m_fftOut[iChan])[iBin];
                                float x1 = (m_fftTmpOut[iBin].r * m_fftTmpOut[iBin].r + m_fftTmpOut[iBin].i * m_fftTmpOut[iBin].i) * scalar;
                                x0 = x1 + gc->m_kFFT[(x1 < x0)] * (x0 - x1);
                                (m_fftOut[iChan])[iBin] = x0;
                            }
                        }

                        m_fftBufP = m_fftSize - m_fftOverlap;
                    }
                }

                // integrate FFT results into log-scale frequency bands
                if (m_nBands)
                {
                    const float df = (float)cfg->sample_size / m_fftSize;
                    const float scalar = 2.0f / (float)cfg->sample_size;
                    for (unsigned int iChan = 0; iChan < 2; ++iChan)
                    {
                        memset(m_bandOut[iChan], 0, m_nBands * sizeof(float));
                        int iBin = 0;
                        int iBand = 0;
                        float f0 = 0.0f;

                        while (iBin <= (m_fftSize / 2) && iBand < m_nBands)
                        {
                            float fLin1 = ((float)iBin + 0.5f) * df;
                            float fLog1 = m_bandFreq[iBand];
                            float x = (m_fftOut[iChan])[iBin];
                            float& y = (m_bandOut[iChan])[iBand];

                            if (fLin1 <= fLog1)
                            {
                                y += (fLin1 - f0) * x * scalar;
                                f0 = fLin1;
                                iBin += 1;
                            }
                            else
                            {
                                y += (fLog1 - f0) * x * scalar;
                                f0 = fLog1;
                                iBand += 1;
                            }
                        }
                    }
                }
            }

            // release the buffer
            //m_clCapture->ReleaseBuffer(nFrames);

            // mark the time of last buffer update
            m_pcFill = pcCur;
        }

        // detect device disconnection
        if (false /* TODO: this should be rexectuded on device disconnnection */)
        {
            for (int iChan = 0; iChan < 2; ++iChan)
            {
                m_rms[iChan] = 0.0;
                m_peak[iChan] = 0.0;
            }
        }

        m_pcPoll = pcCur;

    }
    /* TODO: this is most likely not needed any more */
//    else if (!m->m_parent && !m->m_clCapture && (pcCur.QuadPart - m->m_pcPoll.QuadPart) * m->m_pcMult >= DEVICE_TIMEOUT)
//    {
//        // poll for new devices
//        assert(m->m_enum);
//        assert(!m->m_dev);
//        m->DeviceInit();
//        m->m_pcPoll = pcCur;
//    }

    switch (m_type)
    {
    case measure::TYPE_RMS:
        if (m_channel == CM_BOTH)
            return CLAMP01((sqrt(m_rms[0]) + sqrt(m_rms[1])) * 0.5 * m_global_config->m_gainRMS);
        else
            return CLAMP01(sqrt(m_rms[m_channel]) * m_global_config->m_gainRMS);
        break;

    case measure::TYPE_PEAK:
        if (m_channel == CM_BOTH)
            return CLAMP01((m_peak[0] + m_peak[1]) * 0.5 * m_gainPeak);
        else
            return CLAMP01(m_peak[m_channel] * m_gainPeak);
        break;

    case measure::TYPE_FFT:
        if (m_fftSize) {
            double x;
            const int iFFT = m_fftIdx;
            if (m_channel == CM_BOTH)
                x = (m_fftOut[0][iFFT] + m_fftOut[1][iFFT]) * 0.5;
            else
                x = m_fftOut[m_channel][iFFT];

            x = CLAMP01(x);
            x = UTIL_MAX(0, 10.0 / m_global_config->m_sensitivity * log10(x) + 1.0);
            return x;
        }
        break;

    case measure::TYPE_BAND:
        if (m_nBands) {
            double x;
            const int iBand = m_bandIdx;
            if (m_channel == CM_BOTH)
                x = (m_bandOut[0][iBand] + m_bandOut[1][iBand]) * 0.5;
            else
                x = m_bandOut[m_channel][iBand];

            x = CLAMP01(x);
            x = UTIL_MAX(0, 10.0 / m_global_config->m_sensitivity * log10(x) + 1.0);
            return x;
        }
        break;

    case measure::TYPE_FFTFREQ:
        if (m_fftSize && m_fftIdx <= (m_fftSize / 2))
            return (m_fftIdx * cfg->sample_rate / m_fftSize);
        break;

    case measure::TYPE_BANDFREQ:
        if (m_nBands && m_bandIdx < m_nBands)
            return m_bandFreq[m_bandIdx];
        break;
    }

    return 0.0;
}

}
