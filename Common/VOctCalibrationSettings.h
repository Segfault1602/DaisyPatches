#pragma once

#include "daisy_patch.h"

// This is the offset when the calibration data will be stored in the QSPI chip.
// If you use the QSPI chip in your project, make sure this won't conflict with your other data.
constexpr uint32_t kCalibrationAddressOffset = 0;

constexpr uint8_t kNumCvInputs = daisy::DaisyPatch::CTRL_LAST;

struct CalibrationData
{
    float scale[kNumCvInputs];
    float offset[kNumCvInputs];

    bool operator==(const CalibrationData& other) const
    {
        for (uint8_t i = 0; i < kNumCvInputs; i++)
        {
            if (scale[i] != other.scale[i] || offset[i] != other.offset[i])
            {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const CalibrationData& other) const
    {
        return !(*this == other);
    }
};

CalibrationData kDefaultCalibrationData = {{1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}};
