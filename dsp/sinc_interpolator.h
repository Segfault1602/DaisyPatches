// =============================================================================
// sinc_interpolator.h -- Windowed sinc interpolation
//
// Based on the CCRMA implementation: https://ccrma.stanford.edu/~jos/resample/
// =============================================================================
#pragma once

#include <stdlib.h>

#include "circular_buffer.h"
#include "sinc_table.h"

namespace marguerite
{
class SincInterpolator
{
  public:
    SincInterpolator();
    ~SincInterpolator() = default;

    float Process(const float in)
    {
        // We need to first fill the right history buffer before we can interpolate anything.
        if (!right_history_.IsFull())
        {
            right_history_.Write(in);
            return 0;
        }

        float current_sample = right_history_.Read();
        right_history_.Write(in);

        float sum = 0;

        // Compute left wing
        size_t filter_offset = 0;
        size_t filter_step = samples_per_crossing;
        for (size_t i = left_history_.Count() - 1; i > 0; --i)
        {
            sum += left_history_[i] * sinc_table[filter_offset + filter_step * i];
        }
        sum += left_history_[0] * sinc_table[filter_offset + filter_step * 0];

        // Compute right wing
        filter_offset = 0;
        filter_step = samples_per_crossing;
        for (size_t i = 0; i < right_history_.Count(); ++i)
        {
            sum += right_history_[i] * sinc_table[filter_offset + filter_step * i];
        }

        return sum;
    }

  private:
    Buffer<SINC_ZERO_COUNT> left_history_;
    Buffer<SINC_ZERO_COUNT> right_history_;
};
} // namespace marguerite