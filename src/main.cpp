#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h"

#include "window.h"
#include "timer.h"
#include "glarrays.h"
#include "audioinput.h"
#include "fft.h"
// #include "objects.h"
#include <cassert>
#include <cstring>
#include <array>
#include <cmath>

#define WIDTH 1920
#define HEIGHT 1080

#define SAMPLES_TO_READ 2048
#define NUM_POINTS 1024
// This includes both the channels. so it should be an even number!!
// for other reasons it should also be in the form 2^n.
// This has to be less than 64, shaders dont support over 64
#define NUM_BARS 32

Shader* amp_shader = nullptr;
Shader* bar_shader = nullptr;

static void framebuffer_resize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    assert(amp_shader);
    assert(bar_shader);
    amp_shader->Bind();
    amp_shader->UploadUniform2i("u_Resolution", width, height);
    bar_shader->Bind();
    bar_shader->UploadUniform2i("u_Resolution", width, height);
}

constexpr std::array<float, NUM_BARS> barney_starsoup_is_stupid()
{
    std::array<float, NUM_BARS> barney_the_moron = {};
    for (int i = 0; i < NUM_BARS; i++)
        barney_the_moron[i] = 1.0f;
    return barney_the_moron;
}

static float* audio_engine_getbars(const float* audio_data)
{
    static std::array<float, NUM_BARS> heights;
    static std::array<float, NUM_BARS> nHeights;
    static auto maxHeights = barney_starsoup_is_stupid();

    // To get the number of points required for the i'th bar in a simple exponential
    // function is as the follows f(x) = e^(n-x), when x <= n
    //                       and  f(x) = e^(x-n), when x > n
    //    Here n is the number of bars for one channel also it doesnt have to be e.
    // NOTE: for 32 bars (16 on each side) i've found the number 1.3953 (1.36124 actually) to be quite close
    // for 1024 samples (512 on each side)
    const int n = NUM_BARS/2;
    int current_point = 0;
    for (int i = 0; i < NUM_BARS; i++) {
        nHeights[i] = 0.0f;
        int points_for_bar = (i >= n ? powf(1.36124f, (i+1)-n) : powf(1.36124f, n-i));
        for (int j = 0; j < points_for_bar; j++) {
            nHeights[i] += audio_data[current_point + j];
        }

        current_point += points_for_bar;
        nHeights[i] /= points_for_bar;

        if (nHeights[i] > maxHeights[i]) maxHeights[i] = nHeights[i];

        nHeights[i] /= maxHeights[i];
        // smoothing effect
        heights[i] += (nHeights[i] * 2.0f - heights[i]) / 8.0f;
    }

    return heights.data();
}

constexpr std::array<float, 2*2*(NUM_BARS+1)> VB_get_data()
{
    std::array<float, 2*2*(NUM_BARS+1)> verts = {0.0f};
    float width_per_bar = 2.0f / NUM_BARS;
    float y_vals[] = {-1.0f, 1.0f};
    for (int _y = 0, i = 0; _y < 2; _y++) {
        float y = y_vals[_y];
        for (int _x = 0; _x < (NUM_BARS + 1); _x++) {
            verts[i] = y - y * (width_per_bar * _x);
            verts[i + 1] = y;
            i+=2;
        }
    }
    return verts;
}

constexpr std::array<unsigned int, 6*NUM_BARS> IB_get_data()
{
    std::array<unsigned int, 6*NUM_BARS> indicies = {0};
    unsigned int num_verts = (sizeof(float)*(2*2*(NUM_BARS+1)))/sizeof(float)/2;
    unsigned int max_vert = num_verts-1;
    for (int b = 0, i = 0; b < NUM_BARS; b++) {
        indicies[i] = indicies[i + 5] = b;
        indicies[i + 1] = b + 1;
        indicies[i + 2] = indicies[i + 3] = max_vert-b-1;
        indicies[i + 4] = max_vert-b;
        i+=6;
    }
    return indicies;
}

constexpr std::array<void*, NUM_BARS> VB_get_offset()
{
    std::array<void*, NUM_BARS> x = { NULL };
    for (int i = 0; i < NUM_BARS; i++)
        x[i] = (void *)(4 * 6 * i);
    return x;
}

int main()
{
    char shader_ids[NUM_BARS][16] = {0};
    Window window(WIDTH, HEIGHT, "Snowmeter");
    AudioInput::Create();

    VertexArray amp_va;
    VertexBuffer amp_vb;
    amp_va.Bind();
    amp_vb.UploadData(NULL, SAMPLES_TO_READ * sizeof(float), GL_DYNAMIC_DRAW);
    amp_va.SetLayout({
        {ShaderDataType::Float2}
    });

    amp_shader = new Shader("./src/amplitude.glsl.vs", "./src/amplitude.glsl.fs");
    amp_shader->Bind();
    amp_shader->UploadUniform1f("u_MidSample", NUM_POINTS / 2.0f);
    amp_shader->UploadUniform2i("u_Resolution", WIDTH, HEIGHT);
    window.SetResizeCb(framebuffer_resize);

    auto bar_verticies = VB_get_data();
    auto bar_indicies = IB_get_data();

    VertexArray bar_va;
    VertexBuffer bar_vb;
    bar_va.Bind();
    bar_vb.UploadData(bar_verticies.data(), sizeof(bar_verticies));
    IndexBuffer bar_ib(bar_indicies.data(), 6 * NUM_BARS);
    bar_va.SetLayout({
        {ShaderDataType::Float2}
    });
    bar_shader = new Shader("./src/bar.glsl.vs", "./src/bar.glsl.fs");
    bar_shader->Bind();
    bar_shader->UploadUniform1i("u_NumBars", NUM_BARS);
    bar_shader->UploadUniform2i("u_Resolution", WIDTH, HEIGHT);
    bar_shader->UploadUniform3f("u_BaseGradient", 0.3f, 0.3f, 0.65f);
    bar_shader->UploadUniform3f("u_MaxGradient", 0.7f, 0.0f, 0.0f);
    bar_shader->UploadUniform1f("u_Offset", 0.01f);

    for (int i = 0; i < NUM_BARS; i++) {
        strcpy(shader_ids[i], "u_Heights[x]");
        if (i < 10) {
            shader_ids[i][10] = '0' + i;
        } else {
            shader_ids[i][10] = '0' + i/10;
            shader_ids[i][11] = '0' + i%10;
            shader_ids[i][12] = ']';
            shader_ids[i][13] = 0;
        }
    }

    FFT fft;
    FpsTimer fps;
    std::array<int, NUM_BARS> counts;
    counts.fill(6);
    auto indices = VB_get_offset();

    while (window.Running()) {
        // FuncTimer timer("main loop");
        float* audio_data = AudioInput::GetInput(SAMPLES_TO_READ);
        fft.Compute(audio_data);
        float* bar_heights = audio_engine_getbars(audio_data);
        // amp_vb.UpdateData(0, audio_data, SAMPLES_TO_READ * sizeof(float));

        glClearColor(0.0f, 0.0f, 0.0f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        bar_shader->Bind();
        for (int i = 0; i < NUM_BARS; i++) {
            bar_shader->UploadUniform1f(shader_ids[i], bar_heights[i]);
        }

        bar_va.Bind();
        glMultiDrawElements(GL_TRIANGLES, counts.data(), GL_UNSIGNED_INT, indices.data(), NUM_BARS);

        // amp_va.Bind();
        // amp_shader->Bind();

        // first channel
        // amp_shader->UploadUniform1f("u_Offset", 0.5f);
        // glDrawArrays(GL_LINE_STRIP, 0, NUM_POINTS);
        // second channel
        // amp_shader->UploadUniform1f("u_Offset", -0.5f);
        // glDrawArrays(GL_LINE_STRIP, 0, NUM_POINTS);

        // using namespace std::chrono_literals;
        // std::this_thread::sleep_for(25ms);

        window.Update();
        fps.Update();
    }
    AudioInput::Quit();

    delete amp_shader;
    delete bar_shader;
    return 0;
}
