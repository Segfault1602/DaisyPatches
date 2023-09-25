#include "../Common/VOctCalibrationSettings.h"
#include "daisy_patch.h"
#include "daisysp.h"
#include "util/CpuLoadMeter.h"
#include "util/PersistentStorage.h"

#include "dsp_base.h"
#include "phaseshapers.h"

using namespace daisy;
using namespace daisysp;

DaisyPatch gPatch;
CpuLoadMeter cpuLoadMeter;
char m_cpuLoadStr[16] = {0};

VoctCalibration gVoctCalibration;
PersistentStorage<CalibrationData> gCalibrationDataPersistentStorage(gPatch.seed.qspi);

dsp::Phaseshaper m_osc;
Parameter m_freqCtrl, m_fineCtrl, m_waveCtrl, m_modCtrl;

float gPhase = 0.f;
float gPhaseInc = 0.f;

void UpdateOled();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    cpuLoadMeter.OnBlockStart();

    gPatch.ProcessAllControls();
    float coarse = gVoctCalibration.ProcessInput(gPatch.controls[gPatch.CTRL_1].Process());
    coarse += 36;
    float freq = dsp::FastMidiToFreq(coarse + m_fineCtrl.Process());
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
    gPatch.Init();
    gPatch.SetAudioBlockSize(128); // number of samples handled per callback
    gPatch.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);

    gPhaseInc = 440.f / gPatch.AudioSampleRate();

    gCalibrationDataPersistentStorage.Init(kDefaultCalibrationData, kCalibrationAddressOffset);

    if (gCalibrationDataPersistentStorage.GetState() == PersistentStorage<CalibrationData>::State::FACTORY)
    {
        gPatch.seed.PrintLine("No calibration data found. Using default calibration.");
    }

    const CalibrationData& calibrationData = gCalibrationDataPersistentStorage.GetSettings();
    // Only CV1 is needs to be calibrated for this project.
    gVoctCalibration.SetData(calibrationData.scale[0], calibrationData.offset[0]);

    cpuLoadMeter.Init(gPatch.AudioSampleRate(), gPatch.AudioBlockSize());

    m_osc.Init(gPatch.AudioSampleRate());
    m_osc.SetWaveform(0.f);

    m_freqCtrl.Init(gPatch.controls[gPatch.CTRL_1], 10.0, 110.0f, Parameter::LINEAR);
    m_fineCtrl.Init(gPatch.controls[gPatch.CTRL_2], 0.f, 7.f, Parameter::LINEAR);
    m_waveCtrl.Init(gPatch.controls[gPatch.CTRL_3], 0.0,
                    static_cast<uint8_t>(dsp::Phaseshaper::Waveform::NUM_WAVES) - 1, Parameter::LINEAR);
    m_modCtrl.Init(gPatch.controls[gPatch.CTRL_4], -1.f, 1.f, Parameter::LINEAR);

    gPatch.StartAdc();
    gPatch.StartAudio(AudioCallback);
    while (1)
    {
        gPatch.DelayMs(1);
        UpdateOled();
    }
}

void UpdateOled()
{
    gPatch.display.Fill(false);

    uint8_t col = gPatch.display.Width() - 7 * 3;
    gPatch.display.SetCursor(col, 0);
    uint16_t avgCpuLoad = cpuLoadMeter.GetAvgCpuLoad() * 100;
    memset(m_cpuLoadStr, 0, sizeof(m_cpuLoadStr));
    sprintf(m_cpuLoadStr, "%d%%", avgCpuLoad);

    gPatch.display.WriteString(m_cpuLoadStr, Font_7x10, true);

    gPatch.display.Update();
}
