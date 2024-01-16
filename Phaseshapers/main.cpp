#include "../Common/VOctCalibrationSettings.h"
#include "daisy_patch.h"
#include "daisysp.h"
#include "filter.h"
#include "util/CpuLoadMeter.h"
#include "util/PersistentStorage.h"

#include "dsp_utils.h"
#include "phaseshapers.h"

using namespace daisy;
using namespace daisysp;

constexpr uint8_t kMidiNoteOffset = 32;

DaisyPatch gPatch;
CpuLoadMeter cpuLoadMeter;
char gCpuLoadStr[16] = {0};

char gCurrentFrequencyStr[16] = {0};

VoctCalibration gVoctCalibration;
PersistentStorage<CalibrationData> gCalibrationDataPersistentStorage(gPatch.seed.qspi);

sfdsp::Phaseshaper m_osc[2];
sfdsp::Biquad m_outputFilter;

Parameter m_freqCtrl, m_fineCtrl, m_waveCtrl, m_modCtrl;

float gPhase = 0.f;
float gPhaseInc = 0.f;
float gBaseFreq = 120.f;
float gCurrentFrequency = 0.f;

void UpdateOled();
void UpdateControls();
float ScaleWaveform(float in);

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    cpuLoadMeter.OnBlockStart();

    gPatch.ProcessAllControls();

    UpdateControls();

    float coarse = gVoctCalibration.ProcessInput(gPatch.controls[gPatch.CTRL_1].Process());
    coarse += kMidiNoteOffset;
    float freq = sfdsp::FastMidiToFreq(coarse + m_fineCtrl.Process());
    gCurrentFrequency = freq;

    float waveform = ScaleWaveform(m_waveCtrl.Process());
    float mod = m_modCtrl.Process();
    for (size_t i = 0; i < 2; i++)
    {
        m_osc[i].SetFreq(freq);
        m_osc[i].SetWaveform(waveform);
        m_osc[i].SetMod(mod);
    }

    m_osc[0].ProcessBlock(out[0], size);
    m_osc[1].ProcessBlock(out[1], size);

    m_outputFilter.ProcessBlock(out[1], out[1], size);

    // Pass through the other channels
    for (size_t i = 0; i < size; i++)
    {
        out[2][i] = in[2][i];
        out[3][i] = in[3][i];
    }

    cpuLoadMeter.OnBlockEnd();
}

float ScaleWaveform(float in)
{
    // Add a +/- 0.l0 deadzone around each waveform to make it easier to select a waveform without crossfading
    // between them.
    uint8_t wave_int = static_cast<uint8_t>(in);
    float wave_frac = in - wave_int;

    if (wave_frac < 0.1f)
    {
        wave_frac = 0.0f;
    }
    else if (wave_frac > 0.9f)
    {
        wave_frac = 1.0f;
    }
    else
    {
        // Rescale the fractional part to be between 0.0 and 1.0
        wave_frac = wave_frac / 0.8f;
    }

    return static_cast<float>(wave_int) + wave_frac;
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

    m_outputFilter.SetCoefficients(0.29289322, 0.58578644, 0.29289322, 0, 0.171572875);

    for (size_t i = 0; i < 2; i++)
    {
        m_osc[i].Init(gPatch.AudioSampleRate());
        m_osc[i].SetWaveform(0.f);
    }

    m_freqCtrl.Init(gPatch.controls[gPatch.CTRL_1], 10.0, 110.0f, Parameter::LINEAR);
    m_fineCtrl.Init(gPatch.controls[gPatch.CTRL_2], 0.f, 7.f, Parameter::LINEAR);
    m_waveCtrl.Init(gPatch.controls[gPatch.CTRL_3], 0.0,
                    static_cast<uint8_t>(sfdsp::Phaseshaper::Waveform::NUM_WAVES) - 1, Parameter::LINEAR);
    m_modCtrl.Init(gPatch.controls[gPatch.CTRL_4], 0.f, 1.f, Parameter::LINEAR);

    gPatch.StartAdc();
    gPatch.StartAudio(AudioCallback);
    while (1)
    {
        gPatch.DelayMs(50);
        UpdateOled();
    }
}

void UpdateControls()
{
    int32_t encoder_val = gPatch.encoder.Increment();

    gBaseFreq += encoder_val;
    if (gBaseFreq < 10.f)
    {
        gBaseFreq = 10.f;
    }
    else if (gBaseFreq > 1000.f)
    {
        gBaseFreq = 1000.f;
    }
}

void DisplayCpuUsage()
{
    uint8_t col = gPatch.display.Width() - 7 * 8;
    gPatch.display.SetCursor(col, 0);
    uint16_t avgCpuLoad = cpuLoadMeter.GetAvgCpuLoad() * 100;
    memset(gCpuLoadStr, 0, sizeof(gCpuLoadStr));
    sprintf(gCpuLoadStr, "Avg: %d%%", avgCpuLoad);
    gPatch.display.WriteString(gCpuLoadStr, Font_7x10, true);

    gPatch.display.SetCursor(col, 10);
    uint16_t maxCpuLoad = cpuLoadMeter.GetMaxCpuLoad() * 100;
    memset(gCpuLoadStr, 0, sizeof(gCpuLoadStr));
    sprintf(gCpuLoadStr, "Max: %d%%", maxCpuLoad);
    gPatch.display.WriteString(gCpuLoadStr, Font_7x10, true);

    gPatch.display.SetCursor(col, 20);
    uint16_t minCpuLoad = cpuLoadMeter.GetMinCpuLoad() * 100;
    memset(gCpuLoadStr, 0, sizeof(gCpuLoadStr));
    sprintf(gCpuLoadStr, "Max: %d%%", minCpuLoad);
    gPatch.display.WriteString(gCpuLoadStr, Font_7x10, true);
}

void DisplayCurrentFrequency()
{
    uint8_t col = 0;
    gPatch.display.SetCursor(col, 0);

    float currentPitch = gCurrentFrequency;

    memset(gCurrentFrequencyStr, 0, sizeof(gCurrentFrequencyStr));
    sprintf(gCurrentFrequencyStr, "%d Hz", static_cast<int>(currentPitch));
    gPatch.display.WriteString(gCpuLoadStr, Font_7x10, true);
}

void UpdateOled()
{
    gPatch.display.Fill(false);
    DisplayCpuUsage();
    DisplayCurrentFrequency();

    gPatch.display.Update();
}
