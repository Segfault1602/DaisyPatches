#pragma once

#include <cstdint>

enum class ParamId
{
    TEST_1,
    TEST_2,
    TEST_3,
    TEST_4,
    TEST_5,
    TEST_6,
    TEST_7,
    TEST_8,
    NUM_PARAMS
};

enum class MenuSelected
{
    LABEL,
    VALUE
};

constexpr uint8_t kNumMenuItem = static_cast<uint8_t>(ParamId::NUM_PARAMS);

constexpr uint8_t kDisplayLineCount = 6; // This is the amount of items that can be displayed on the screen at once

constexpr uint8_t kMaxLabelLength = 16;

struct MenuItem
{
    const char label[kMaxLabelLength];
    ParamId paramId;
    uint8_t value;
    uint8_t min = 0;
    uint8_t max = 100;
};