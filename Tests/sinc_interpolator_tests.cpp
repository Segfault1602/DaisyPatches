#include "gtest/gtest.h"

#include "sinc_interpolator.h"

#include <numeric>

TEST(SincInterpolateTest, NoInterpolation)
{
    marguerite::SincInterpolator sinc;

    constexpr size_t array_size = 512;
    std::array<float, array_size> input{0};
    std::iota(input.begin(), input.end(), 0.f);

    std::array<float, array_size> output{0};

    for (size_t i = 0; i < input.size(); ++i)
    {
        output[i] = sinc.Process(input[i]);
    }

    ASSERT_EQ(output[0], 0);
}