#include "daisy_patch.h"
#include "daisysp.h"
#include "menu_gui.h"
#include "util/CpuLoadMeter.h"

#include "buchla_lpg.h"
#include "dsp_utils.h"
#include "filter.h"

using namespace daisy;
using namespace daisysp;

constexpr uint8_t kBlockSize = 32;

enum class ParamId : uint8_t
{
    AVG_CPU,
    NUM_PARAMS
};
constexpr uint8_t kNumMenuItem = static_cast<uint8_t>(ParamId::NUM_PARAMS);

MenuItem<ParamId> menuItems[] = {
    {"Avg CPU", ParamId::AVG_CPU, 0},
};

DaisyPatch gPatch;
CpuLoadMeter cpuLoadMeter;
MenuGui<ParamId> gMenuGui(gPatch.display, gPatch.encoder, menuItems, kNumMenuItem);

constexpr uint8_t kNumLpgs = 2;

sfdsp::BuchlaLPG gLpg[kNumLpgs];
Parameter gCvsIn[kNumLpgs];
sfdsp::OnePoleFilter gSmoothingFilter[kNumLpgs];

void UpdateOled();
void UpdateControls();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    cpuLoadMeter.OnBlockStart();

    gPatch.ProcessAnalogControls();
    gPatch.ProcessDigitalControls();
    UpdateControls();

    for (size_t i = 0; i < kNumLpgs; i++)
    {
        float cv = gCvsIn[i].Process();
        float smoothedCv[kBlockSize];
        for (size_t i = 0; i < size; i++)
        {
            smoothedCv[i] = cv;
        }
        gLpg[i].ProcessBlock(smoothedCv, in[i], out[i], size);
    }

    cpuLoadMeter.OnBlockEnd();
}

int main(void)
{
    gPatch.Init();
    gMenuGui.SetSplashScreen("Daisy LPG", "v0.0.1");

    gPatch.SetAudioBlockSize(kBlockSize); // number of samples handled per callback
    gPatch.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);

    cpuLoadMeter.Init(gPatch.AudioSampleRate(), gPatch.AudioBlockSize());

    for (size_t i = 0; i < kNumLpgs; i++)
    {
        gLpg[i].Init(gPatch.AudioSampleRate());
        gSmoothingFilter[i].SetDecayFilter(-6, 1, gPatch.AudioSampleRate());
        gCvsIn[i].Init(gPatch.controls[i], 0.f, 1.f, Parameter::Curve::LINEAR);
    }

    gPatch.StartAdc();
    gPatch.StartAudio(AudioCallback);
    while (1)
    {
        gPatch.DelayMs(30);
        UpdateOled();
    }
}

void UpdateControls()
{
    gMenuGui.SetParamValue(ParamId::AVG_CPU, cpuLoadMeter.GetAvgCpuLoad() * 100.f);
    gMenuGui.ProcessUiEvent();
}

void UpdateOled()
{
    gMenuGui.Draw();
}
