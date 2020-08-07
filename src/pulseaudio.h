#pragma once

#include <pulse/pulseaudio.h>
#include <string>
#include <mutex>

#define MAX_BUFFER_SIZE sizeof(float)*2*8192

struct PulseAudioInput
{
	PulseAudioInput() {}
	~PulseAudioInput() {}

	static void Init();
	static int Run();
	static void Shutdown();

	static std::mutex s_Mtx;
	static bool s_Close;
	static PulseAudioInput* s_Instance;
	static std::string s_Device;
	static pa_mainloop* s_Loop;
	static pa_context* s_Handle;
	static pa_stream* s_Stream;

	// Should only be accessed via the mutex
	static size_t s_BufferSize;
	static unsigned char s_Buffer[MAX_BUFFER_SIZE];
};
