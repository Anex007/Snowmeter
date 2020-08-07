#include "fft.h"
#include <cstring>
#include <cmath>

// We're only using floating point values here. no doubles!!
// Make sure to dft both channels.
// Since the measure thing takes some seconds to find the optimal algorithm
//   it's better to run it on a seperate thread so as to have no delays.
// TODO: dont forget to make your own implementation of fft
//  You really dont know whats happening.
// Make your own fft implementation it doesnt matter if its that slow.

#define MAX_SAMPLES 1024

FFT::FFT()
{
    m_in0 = fftwf_alloc_real(MAX_SAMPLES);
    m_in1 = fftwf_alloc_real(MAX_SAMPLES);
    m_out0 = fftwf_alloc_complex(MAX_SAMPLES);
    m_out1 = fftwf_alloc_complex(MAX_SAMPLES);
    m_plan0 = fftwf_plan_dft_r2c_1d(MAX_SAMPLES, m_in0, m_out0, FFTW_MEASURE);
    m_plan1 = fftwf_plan_dft_r2c_1d(MAX_SAMPLES, m_in1, m_out1, FFTW_MEASURE);
}

void FFT::Compute(float* audio_data)
{
    //memcpy to right channel buffers
    float *right = m_in0, *left = m_in1;
    for (int i = 0; i < 2 * MAX_SAMPLES; i+=2, right++, left++) {
        *right = audio_data[i];
        *left = audio_data[i+1];
    }
    fftwf_execute(m_plan0);
    fftwf_execute(m_plan1);
    fftwf_complex* right_o = m_out0;
    fftwf_complex* left_o = m_out1;
    const int half_way = MAX_SAMPLES/2;
    for (int i = 0; i < half_way; i++, right_o++, left_o++) {
        float xr = *right_o[0];
        float yr = *right_o[1];
        float xl = *left_o[0];
        float yl = *left_o[1];
        audio_data[half_way - i - 1] = hypotf(xl, yl);
        audio_data[half_way + i] = hypotf(xr, yr);
        // audio_data[i] = xr;
        // audio_data[i+1] = xl;
    }
}

FFT::~FFT()
{
    fftwf_destroy_plan(m_plan0);
    fftwf_destroy_plan(m_plan1);
    fftwf_free(m_in0);
    fftwf_free(m_in1);
    fftwf_free(m_out0);
    fftwf_free(m_out1);
}