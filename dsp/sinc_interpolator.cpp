// =============================================================================
// sinc_interpolator.cpp -- Windowed sinc interpolation
//
// Based on the CCRMA implementation: https://ccrma.stanford.edu/~jos/resample/
// =============================================================================

#include "sinc_interpolator.h"

namespace marguerite
{
    SincInterpolator::SincInterpolator()
    {
        left_history_.Fill(0);
    }
}
