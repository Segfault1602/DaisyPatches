#include <daisy_patch.h>
#include <daisysp.h>
#include <util/PersistentStorage.h>
#include <util/VoctCalibration.h>

#include <algorithm>

#include "../Common/VOctCalibrationSettings.h"

using namespace daisy;
using namespace daisysp;

enum class CalibrationState
{
    WaitingFor1V,
    Recording1V,
    WaitingFor3V,
    Recording3V,
    SAVING,
    DONE
};

float g1V[kNumCvInputs];
float g3V[kNumCvInputs];

// Do 50 readings and average them
constexpr size_t kNumCalibrationReadings = 50;
float gADCReadings[kNumCvInputs][kNumCalibrationReadings];
size_t gCurrentReadingNum = 0;

DaisyPatch gPatch;
VoctCalibration gVoctCalibration;
CalibrationState gState = CalibrationState::WaitingFor1V;

PersistentStorage<CalibrationData> gCalibrationDataPersistentStorage(gPatch.seed.qspi);

void UpdateOled();
void ProcessControls();

void DisplayWaitingFor1V();
void DisplayRecording1V();
void DisplayWaitingFor3V();
void DisplayRecording3V();
void DisplaySaving();
void DisplayDone();

// Returns the average of the readings, excluding the max and min values.
float CalculateAverageReading(float readings[kNumCalibrationReadings]);

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    ProcessControls();
}

void ProcessControls()
{
    gPatch.ProcessAllControls();

    switch (gState)
    {
    case CalibrationState::WaitingFor1V:
    {
        if (gPatch.encoder.FallingEdge())
        {
            gCurrentReadingNum = 0;
            memset(gADCReadings, 0, sizeof(gADCReadings));
            gState = CalibrationState::Recording1V;
        }
        break;
    }
    case CalibrationState::Recording1V:
    {
        gADCReadings[DaisyPatch::CTRL_1][gCurrentReadingNum] = gPatch.controls[DaisyPatch::CTRL_1].Process();
        gADCReadings[DaisyPatch::CTRL_2][gCurrentReadingNum] = gPatch.controls[DaisyPatch::CTRL_2].Process();
        gADCReadings[DaisyPatch::CTRL_3][gCurrentReadingNum] = gPatch.controls[DaisyPatch::CTRL_3].Process();
        gADCReadings[DaisyPatch::CTRL_4][gCurrentReadingNum] = gPatch.controls[DaisyPatch::CTRL_4].Process();

        ++gCurrentReadingNum;

        if (gCurrentReadingNum >= kNumCalibrationReadings)
        {
            for (auto i = 0; i < kNumCvInputs; ++i)
            {
                g1V[i] = CalculateAverageReading(gADCReadings[i]);
                gPatch.seed.PrintLine("Average 1V reading for cv input %d: %f", i, g1V[i]);
            }
            gState = CalibrationState::WaitingFor3V;
            gCurrentReadingNum = 0;
        }
        break;
    }
    case CalibrationState::WaitingFor3V:
    {
        if (gPatch.encoder.FallingEdge())
        {
            gCurrentReadingNum = 0;
            memset(gADCReadings, 0, sizeof(gADCReadings));
            gState = CalibrationState::Recording3V;
        }
        break;
    }
    case CalibrationState::Recording3V:
    {
        gADCReadings[DaisyPatch::CTRL_1][gCurrentReadingNum] = gPatch.controls[DaisyPatch::CTRL_1].Process();
        gADCReadings[DaisyPatch::CTRL_2][gCurrentReadingNum] = gPatch.controls[DaisyPatch::CTRL_2].Process();
        gADCReadings[DaisyPatch::CTRL_3][gCurrentReadingNum] = gPatch.controls[DaisyPatch::CTRL_3].Process();
        gADCReadings[DaisyPatch::CTRL_4][gCurrentReadingNum] = gPatch.controls[DaisyPatch::CTRL_4].Process();

        ++gCurrentReadingNum;

        if (gCurrentReadingNum >= kNumCalibrationReadings)
        {
            for (auto i = 0; i < kNumCvInputs; ++i)
            {
                g3V[i] = CalculateAverageReading(gADCReadings[i]);
                gPatch.seed.PrintLine("Average 1V reading for cv input %d: %f", i, g1V[i]);
            }
            gState = CalibrationState::SAVING;
            gCurrentReadingNum = 0;
        }
        break;
    }
    case CalibrationState::SAVING:
    {
        CalibrationData& calibrationData = gCalibrationDataPersistentStorage.GetSettings();
        for (uint8_t i = 0; i < kNumCvInputs; i++)
        {
            gVoctCalibration.Record(g1V[i], g3V[i]);
            gVoctCalibration.GetData(calibrationData.scale[i], calibrationData.offset[i]);
            gPatch.seed.PrintLine("Calibration data for cv input %d: scale: %f, offset: %f", i,
                                  calibrationData.scale[i], calibrationData.offset[i]);
        }

        gCalibrationDataPersistentStorage.Save();

        gState = CalibrationState::DONE;
        break;
    }
    case CalibrationState::DONE:
    {
        DisplayDone();
        break;
    }
    }
}

int main(void)
{
    gPatch.Init();
    gPatch.SetAudioBlockSize(64);
    gPatch.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    gPatch.seed.StartLog(false);

    gCalibrationDataPersistentStorage.Init(kDefaultCalibrationData, kCalibrationAddressOffset);

    if (gCalibrationDataPersistentStorage.GetState() == PersistentStorage<CalibrationData>::State::USER)
    {
        gPatch.seed.PrintLine("Previous calibration data found. Previous calibration will be overwritten.");
    }

    gPatch.StartAdc();
    gPatch.StartAudio(AudioCallback);
    while (1)
    {
        gPatch.DelayMs(30);
        UpdateOled();
    }
}

void UpdateOled()
{
    gPatch.display.Fill(false);

    switch (gState)
    {
    case CalibrationState::WaitingFor1V:
    {
        DisplayWaitingFor1V();
        break;
    }
    case CalibrationState::Recording1V:
    {
        DisplayRecording1V();
        break;
    }
    case CalibrationState::WaitingFor3V:
    {
        DisplayWaitingFor3V();
        break;
    }
    case CalibrationState::Recording3V:
    {
        DisplayRecording3V();
        break;
    }
    case CalibrationState::SAVING:
    {
        DisplaySaving();
        break;
    }
    case CalibrationState::DONE:
    {
        DisplayDone();
        break;
    }
    }

    gPatch.display.Update();
}

void DisplayWaitingFor1V()
{
    gPatch.display.SetCursor(0, 0);
    gPatch.display.WriteString("Calibrator", Font_6x8, true);

    gPatch.display.SetCursor(0, 10);
    gPatch.display.WriteString("Send 1V signal to all", Font_6x8, true);
    gPatch.display.SetCursor(0, 20);
    gPatch.display.WriteString("4 CV inputs then", Font_6x8, true);
    gPatch.display.SetCursor(0, 30);
    gPatch.display.WriteString("press the encoder to", Font_6x8, true);
    gPatch.display.SetCursor(0, 40);
    gPatch.display.WriteString("continue to next step", Font_6x8, true);
}

void DisplayRecording1V()
{
    gPatch.display.SetCursor(0, 0);
    gPatch.display.WriteString("Processing 1V signal...", Font_6x8, true);
}

void DisplayWaitingFor3V()
{
    gPatch.display.SetCursor(0, 10);
    gPatch.display.WriteString("Send 3V signal to all", Font_6x8, true);
    gPatch.display.SetCursor(0, 20);
    gPatch.display.WriteString("4 CV inputs then", Font_6x8, true);
    gPatch.display.SetCursor(0, 30);
    gPatch.display.WriteString("press the encoder to", Font_6x8, true);
    gPatch.display.SetCursor(0, 40);
    gPatch.display.WriteString("continue to next step", Font_6x8, true);
}

void DisplayRecording3V()
{
    gPatch.display.SetCursor(0, 0);
    gPatch.display.WriteString("Processing 3V signal...", Font_6x8, true);
}

void DisplaySaving()
{
    gPatch.display.SetCursor(0, 0);
    gPatch.display.WriteString("Saving calibration to memory...", Font_6x8, true);
}

void DisplayDone()
{
    gPatch.display.SetCursor(0, 0);
    gPatch.display.WriteString("Calibration complete!", Font_6x8, true);
}

float CalculateAverageReading(float readings[kNumCalibrationReadings])
{
    float max = *std::max_element(readings, readings + kNumCalibrationReadings);
    float min = *std::min_element(readings, readings + kNumCalibrationReadings);

    float sum = std::accumulate(readings, readings + kNumCalibrationReadings, 0.f);
    sum -= (max + min); // Remove the max and min values from the sum before averaging.

    return sum / (kNumCalibrationReadings - 2);
}
