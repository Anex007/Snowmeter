#include "pulseaudio.h"

#include <cassert>
#include <iostream>
#include <cstring>
// TODO: Make the data consumable, since you have locks setup this shouldnt be that hard.
// 		Figure out the minumum storage required so that you dont have to transfer old bytes to
//		a different offset.
// TODO: After finishing make sure you unreference all the pulseaudio resources
// to make sure there is no memory leak.

#define APPNAME "snowmeter"
#define SERVER NULL
#define STREAMNAME "snowmeter capture"
#define STREAM_FAILED -4
#define STREAM_READ_FAILED -6
#define CONTEXT_CRASH -8

std::mutex PulseAudioInput::s_Mtx;
bool PulseAudioInput::s_Close = false;
PulseAudioInput* PulseAudioInput::s_Instance = nullptr;
std::string PulseAudioInput::s_Device;
pa_mainloop* PulseAudioInput::s_Loop = nullptr;
pa_context* PulseAudioInput::s_Handle = nullptr;
pa_stream* PulseAudioInput::s_Stream = nullptr;
size_t PulseAudioInput::s_BufferSize = 0;
unsigned char PulseAudioInput::s_Buffer[MAX_BUFFER_SIZE];

static pa_sample_spec samplespec {
    .format = PA_SAMPLE_FLOAT32LE,
    .rate = 44100,
    .channels = 2
};

static void stream_suspended_cb(pa_stream* stream, void* userdata)
{
}

// We first need to get the device name before connecting.
static void server_info_cb(pa_context* hndl, const pa_server_info* info, void* userdata)
{
    pa_buffer_attr pa_buf;
    PulseAudioInput* pulseInput = (PulseAudioInput*)userdata;
    pulseInput->s_Device.append(info->default_sink_name);
    pulseInput->s_Device.append(".monitor");

    // NOTE: Change the buffer length and shits here.
    pa_buf.fragsize = (uint32_t)-1;
    pa_buf.maxlength = (uint32_t)-1;

    // After getting the data we can connect the stream and other shits.

    // flags is set to 0.
    if (pa_stream_connect_record(pulseInput->s_Stream, pulseInput->s_Device.c_str(), &pa_buf, (pa_stream_flags)0) < 0) {
        std::cerr << "Pulse: Failed to connect record stream\n";
        pa_mainloop_quit(pulseInput->s_Loop, STREAM_FAILED);
        return;
    }
}

// This is the heart of this class where we read in audio input
static void stream_read_cb(pa_stream* stream, size_t nbytes, void *userdata)
{
    PulseAudioInput* pulseInput = (PulseAudioInput*)userdata;
    const void* data;

    if (pa_stream_peek(stream, &data, &nbytes) < 0) {
        std::cerr << "Great News!! The core of audio input just crashed, welp its time to go cry under shower\n";
        pa_mainloop_quit(pulseInput->s_Loop, STREAM_READ_FAILED);
        return;
    }

    assert(data);
    assert(nbytes <= MAX_BUFFER_SIZE);

#define MAX_SAMPLES_EXP 2048*sizeof(float)

    {
        std::lock_guard<std::mutex> lock(pulseInput->s_Mtx);
        if (pulseInput->s_BufferSize + nbytes > MAX_BUFFER_SIZE) {
            if (nbytes >= MAX_SAMPLES_EXP) {
                memcpy(pulseInput->s_Buffer, data, nbytes);
                pulseInput->s_BufferSize = nbytes;
            } else {
                // assume nbytes = 1048 * float_size, s_BufferSize = 2*8192*float_size
                // offset = 15384, bufferSize = 1000, 
                const size_t offset = pulseInput->s_BufferSize - MAX_SAMPLES_EXP + nbytes;
                pulseInput->s_BufferSize = MAX_SAMPLES_EXP - nbytes;
                memcpy(pulseInput->s_Buffer, pulseInput->s_Buffer + offset, pulseInput->s_BufferSize);
                memcpy(pulseInput->s_Buffer + pulseInput->s_BufferSize, data, nbytes);
                pulseInput->s_BufferSize = MAX_SAMPLES_EXP;
            }
        } else {
            memcpy(pulseInput->s_Buffer + pulseInput->s_BufferSize, data, nbytes);
            pulseInput->s_BufferSize += nbytes;
        }
    }
#undef MAX_SAMPLES_EXP
    pa_stream_drop(stream);
}

static void stream_state_cb(pa_stream* stream, void* userdata)
{
    PulseAudioInput* pulseInput = (PulseAudioInput*)userdata;
    switch (pa_stream_get_state(stream)) {
        case PA_STREAM_UNCONNECTED:
        case PA_STREAM_CREATING:
        case PA_STREAM_TERMINATED:
        case PA_STREAM_READY:
            break;
        case PA_STREAM_FAILED:
            pa_mainloop_quit(pulseInput->s_Loop, STREAM_FAILED);
            break;
    }
}

// called when context state changes
static void context_state_cb(pa_context* hndl, void* userdata)
{
    PulseAudioInput* pulseInput = (PulseAudioInput*)userdata;
    switch (pa_context_get_state(hndl)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;
        case PA_CONTEXT_READY:

            if (!(pulseInput->s_Stream = pa_stream_new(hndl, STREAMNAME, &samplespec, NULL))) {
                std::cerr << "Pulse: Failed to create stream\n";
                pa_mainloop_quit(pulseInput->s_Loop, STREAM_FAILED);
                return;
            }

            // Setup callbacks here
            pa_stream_set_state_callback(pulseInput->s_Stream, stream_state_cb, userdata);
            pa_stream_set_read_callback(pulseInput->s_Stream, stream_read_cb, userdata);
            pa_stream_set_suspended_callback(pulseInput->s_Stream, stream_suspended_cb, userdata);
            // Skipped: pa_stream_set_moved_callback,pa_stream_set_event_callback,
            // pa_stream_set_buffer_attr_callback, pa_stream_set_latency_update_callback.

            // Get the default sink device and connect the stream in that cb.
            pa_operation_unref(pa_context_get_server_info(hndl, server_info_cb, userdata));

            break;
        case PA_CONTEXT_TERMINATED:
            // NOTE: There is no need explicitly disconect here
            break;
        case PA_CONTEXT_FAILED:
            pa_mainloop_quit(pulseInput->s_Loop, CONTEXT_CRASH);
            break;
    }
}

void PulseAudioInput::Init()
{
    PulseAudioInput* s_Instance = new PulseAudioInput();
    pa_mainloop_api* loopapi;

    if (!pa_sample_spec_valid(&samplespec)) {
        std::cerr << "Pulse: Invalid sample specification\n";
        return;
    }

    if (!(s_Loop = pa_mainloop_new())) {
        std::cerr << "Pulse: Failed to create mainloop\n";
        return;
    }

    loopapi = pa_mainloop_get_api(s_Loop);

    // hook signals here if there is not a direct way to shut down. use pa_signal...

    if (!(s_Handle = pa_context_new(loopapi, APPNAME))) {
        std::cerr << "Pulse: Failed to create new context\n";
        return;
    }

    pa_context_set_state_callback(s_Handle, context_state_cb, s_Instance); // You can specify userdata here

    if (pa_context_connect(s_Handle, SERVER, (pa_context_flags)0, NULL) < 0) {
        std::cerr << "Pulse: Failed to connect: " << pa_strerror(pa_context_errno(s_Handle)) << std::endl;
        return;
    }
}

int PulseAudioInput::Run()
{
    int ret, r;

    // if (pa_mainloop_run(s_Loop, &ret) < 0) {
    // 	std::cerr << "Pulse: Mainloop exited with error: " << ret << "\nShutting down...\n";
    // 	Shutdown();
    // 	return -1;
    // }
    while (!s_Close && (r = pa_mainloop_iterate(s_Loop, 1, &ret)) >= 0);
        // std::cout << "params: " << s_Loop << std::endl;

    if (r < 0)
        std::cerr << "Pulse: Mainloop errored with " << ret << "Shutting down...\n";
    Shutdown();
    return ret;
}

void PulseAudioInput::Shutdown()
{
    if (s_Stream) {
        pa_stream_disconnect(s_Stream);
        pa_stream_unref(s_Stream);
        s_Stream = nullptr;
    }
    if (s_Handle) {
        pa_context_disconnect(s_Handle);
        pa_context_unref(s_Handle);
        s_Handle = nullptr;
    }
    if (s_Loop) {
        pa_mainloop_free(s_Loop);
        s_Loop = nullptr;
    }
    delete s_Instance;
    s_Instance = nullptr;
}
