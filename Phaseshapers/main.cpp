#include "daisy_patch.h"
#include "daisysp.h"
#include "util/CpuLoadMeter.h"

#include "phaseshapers.h"

using namespace daisy;
using namespace daisysp;

DaisyPatch m_patch;
CpuLoadMeter cpuLoadMeter;
char m_cpuLoadStr[16] = {0};

dsp::Phaseshaper m_osc;
Parameter m_freqCtrl, m_fineCtrl, m_waveCtrl, m_modCtrl;

void UpdateOled();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    cpuLoadMeter.OnBlockStart();

    m_patch.ProcessAllControls();
    float freq = mtof(m_freqCtrl.Process() + m_fineCtrl.Process());
    m_osc.SetFreq(freq);

    m_osc.SetWaveform(m_waveCtrl.Process());

    float mod = m_modCtrl.Process();
    m_osc.SetMod(mod);

    for (size_t i = 0; i < size; i++)
    {
        out[0][i] = m_osc.Process();
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

    m_osc.Init(m_patch.AudioSampleRate());
    m_osc.SetWaveform(0.f);

    m_freqCtrl.Init(m_patch.controls[m_patch.CTRL_1], 10.0, 110.0f, Parameter::LINEAR);
    m_fineCtrl.Init(m_patch.controls[m_patch.CTRL_2], 0.f, 7.f, Parameter::LINEAR);
    m_waveCtrl.Init(m_patch.controls[m_patch.CTRL_3], 0.0, static_cast<uint8_t>(dsp::Phaseshaper::Waveform::NUM_WAVES)-1,
                    Parameter::LINEAR);
    m_modCtrl.Init(m_patch.controls[m_patch.CTRL_4], -1.f, 1.f, Parameter::LINEAR);

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
