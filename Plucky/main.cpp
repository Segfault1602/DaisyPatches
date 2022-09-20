#include "daisy_patch.h"
#include "daisysp.h"
#include "util/CpuLoadMeter.h"

#include "Waveguide.h"

using namespace daisy;
using namespace daisysp;

DaisyPatch m_patch;
CpuLoadMeter cpuLoadMeter;
char m_cpuLoadStr[16] = {0};

Tone m_lp;

Parameter m_delayLengthCtrl, m_pluckLocationCtrl, m_pickupLocationCtrl;

float m_delayLength = 40;
float m_pluckLength = m_delayLength / 2.f;
float m_pluckLocation = m_delayLength / 2.f;
float m_pickupLocation = m_delayLength / 2.f;

constexpr size_t MAX_DELAY = 1024;

marguerite::Waveguide<MAX_DELAY> m_waveguide;

void UpdateOled();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    cpuLoadMeter.OnBlockStart();

    m_patch.ProcessAllControls();

    float noteFreq = mtof(std::floor(m_delayLengthCtrl.Process()));
    m_delayLength = m_patch.AudioSampleRate() / noteFreq / 2.f;
    m_waveguide.SetDelay(m_delayLength);
    m_pluckLocation = m_pluckLocationCtrl.Process();
    m_pickupLocation = m_pickupLocationCtrl.Process();

    const float threshold = 0.1f;
    float actualPluckLocation = (m_pluckLocation * m_delayLength);
    for (size_t i = 0; i < size; i++)
    {
        if (in[0][i] > threshold)
        {
            m_waveguide.AddIn(in[0][i] * 0.5f, actualPluckLocation);
        }

        out[0][i] = m_waveguide.Process();
        out[1][i] = in[1][i];
        out[2][i] = in[2][i];
        out[3][i] = in[3][i];
    }

    cpuLoadMeter.OnBlockEnd();
}

int main(void)
{
    m_patch.Init();
    m_patch.SetAudioBlockSize(4); // number of samples handled per callback
    m_patch.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    cpuLoadMeter.Init(m_patch.AudioSampleRate(), m_patch.AudioBlockSize());

    m_delayLengthCtrl.Init(m_patch.controls[m_patch.CTRL_1], 10.0, 110.0f, Parameter::LINEAR);
    m_pluckLocationCtrl.Init(m_patch.controls[m_patch.CTRL_2], 0.f, 1.f, Parameter::LINEAR);
    m_pickupLocationCtrl.Init(m_patch.controls[m_patch.CTRL_3], 0.0f, 1.f, Parameter::LINEAR);

    m_lp.Init(m_patch.AudioSampleRate());
    m_lp.SetFreq(100);

    m_patch.StartAdc();
    m_patch.StartAudio(AudioCallback);
    while (1)
    {
        m_patch.DelayMs(1);
        UpdateOled();
    }
}

void UpdateOled()
{
    m_patch.display.Fill(false);

    uint8_t col = m_patch.display.Width() - 7 * 3;
    m_patch.display.SetCursor(col, 0);
    uint16_t avgCpuLoad = cpuLoadMeter.GetAvgCpuLoad() * 100;
    memset(m_cpuLoadStr, 0, sizeof(m_cpuLoadStr));
    sprintf(m_cpuLoadStr, "%d%%", avgCpuLoad);

    m_patch.display.WriteString(m_cpuLoadStr, Font_7x10, true);

    m_patch.display.Update();
}
