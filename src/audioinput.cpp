#include "audioinput.h"
#include "pulseaudio.h"

#include <mutex>
#include <cassert>
#include <cstring>

// temp
#include <iostream>
// #include "timer.h"

// This is more than enough. (excluding sizeof(float))
#define CACHE_BUFFER_SIZE 4096

AudioInput* AudioInput::s_Instance = nullptr;
float* AudioInput::s_Buffer = nullptr;
std::thread AudioInput::tHndl;

AudioInput::AudioInput()
{
}

AudioInput::~AudioInput()
{
}

void AudioInput::Create()
{
	AudioInput* s_Instance = new AudioInput();
	AudioInput::s_Buffer = new float[CACHE_BUFFER_SIZE];
	tHndl = std::thread([](){
		PulseAudioInput::Init();
		PulseAudioInput::Run();
	});
}

void AudioInput::Quit()
{
	PulseAudioInput::s_Close = true;
	delete[] s_Buffer;
	s_Buffer = nullptr;
	delete s_Instance;
	s_Instance = nullptr;
	tHndl.join();
}

/* num_samples = num_channels * samples
 * This will not block at all, if there is no data, it'll fill in zeros.
 */
float* AudioInput::GetInput(size_t num_samples)
{
	// FuncTimer timer("Nonblocking read: ");
	assert(num_samples <= CACHE_BUFFER_SIZE);
	{
		std::lock_guard<std::mutex> lock(PulseAudioInput::s_Mtx);

		if (PulseAudioInput::s_BufferSize == 0) {
			memset(s_Buffer, 0, num_samples * sizeof(float));
		} else if (PulseAudioInput::s_BufferSize < (num_samples * sizeof(float))) {
			const size_t bytesRead = PulseAudioInput::s_BufferSize;
			memcpy(s_Buffer, PulseAudioInput::s_Buffer, bytesRead);
			PulseAudioInput::s_BufferSize = 0;
			memset(((void*)(s_Buffer)) + bytesRead, 0, (num_samples * sizeof(float)) - bytesRead);
		} else {
			const size_t bytesRead = num_samples * sizeof(float);
			memcpy(s_Buffer, PulseAudioInput::s_Buffer, bytesRead);
			PulseAudioInput::s_BufferSize -= bytesRead;
		}
	}
	return s_Buffer;
}
