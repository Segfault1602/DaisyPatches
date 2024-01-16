#include "daisy_patch.h"
#include "daisysp.h"
#include "util/CpuLoadMeter.h"

#define RTT_USE_ASM 1
#include "SEGGER_RTT.h"

#include "menu_gui.h"

enum class ParamId : uint8_t
{
    TEST_1,
    TEST_2,
    TEST_3,
    TEST_4,
    TEST_5,
    TEST_6,
    TEST_7,
    TEST_8,
    AVG_CPU,
    NUM_PARAMS
};

constexpr uint8_t kNumMenuItem = static_cast<uint8_t>(ParamId::NUM_PARAMS);

MenuItem<ParamId> menuItems[] = {
    {"Test 1", ParamId::TEST_1, 10},     {"Test 2", ParamId::TEST_2, 1},           {"Test 3", ParamId::TEST_3, 0},
    {"LongTest4", ParamId::TEST_4, 100}, {"LongTest5abcdef", ParamId::TEST_5, 0},  {"LongTest6", ParamId::TEST_6, 99},
    {"LongTest7", ParamId::TEST_7, 0},   {"LongTest8dddddd", ParamId::TEST_8, 55}, {"Avg CPU", ParamId::AVG_CPU, 0},
};

using namespace daisy;
using namespace daisysp;

DaisyPatch gPatch;
CpuLoadMeter cpuLoadMeter;
MenuGui<ParamId> menuGui(gPatch.display, gPatch.encoder, menuItems, kNumMenuItem);

void UpdateOled();
void UpdateControls();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    cpuLoadMeter.OnBlockStart();
    gPatch.ProcessAnalogControls();
    gPatch.ProcessDigitalControls();
    UpdateControls();

    // Pass through the other channels
    for (size_t i = 0; i < size; i++)
    {
        out[0][i] = in[0][i] * in[1][i];
        out[1][i] = in[1][i] * in[2][i];
        out[2][i] = in[2][i] * in[3][i];
        out[3][i] = in[3][i] * in[0][i];
    }
    cpuLoadMeter.OnBlockEnd();
    SEGGER_RTT_WriteString(0, "AudioCallback\n");
}

int main(void)
{
    gPatch.Init();
    menuGui.SetSplashScreen("UI Test", "v1.0.0");

    SEGGER_RTT_ConfigUpBuffer(0, "My Buffer", NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
    SEGGER_RTT_WriteString(0, "Hello World\n");

    gPatch.SetAudioBlockSize(128); // number of samples handled per callback
    gPatch.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);

    cpuLoadMeter.Init(gPatch.AudioSampleRate(), gPatch.AudioBlockSize());

    gPatch.StartAdc();
    gPatch.StartAudio(AudioCallback);
    while (1)
    {
        gPatch.DelayMs(33);
        UpdateOled();
        SEGGER_RTT_WriteString(0, "While loop\n");
    }
}

void UpdateControls()
{
    menuGui.SetParamValue(ParamId::AVG_CPU, cpuLoadMeter.GetAvgCpuLoad() * 100.f);
    menuGui.ProcessUiEvent();
}

void UpdateOled()
{
    menuGui.Draw();
}
