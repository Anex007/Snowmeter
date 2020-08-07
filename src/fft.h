#pragma once

#include <fftw3.h>

class FFT
{
public:
    FFT();
    ~FFT();

    // Will read in data from audio_data and overwrite audio_data with fft info.
    // NOTE: The data is only overwritten at the first n/2 indicies.
    void Compute(float* audio_data);
private:
    fftwf_plan m_plan0;
    fftwf_plan m_plan1;
    float* m_in0;
    float* m_in1;
    fftwf_complex* m_out0;
    fftwf_complex* m_out1;
};
