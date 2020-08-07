#pragma once

#include <cstddef>
#include <thread>

class AudioInput
{
public:
	AudioInput();
	~AudioInput();

	static void Create();
	static void Quit();
	static float* GetInput(size_t num_samples);

private:
	static std::thread tHndl;
	static AudioInput* s_Instance;
	static float* s_Buffer;
};
