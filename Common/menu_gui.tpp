#include "menu_gui.h"

#include <cstdio>

namespace
{
constexpr uint8_t kDisplayLineCount = 6; // This is the amount of items that can be displayed on the screen at once
constexpr uint32_t kSplashScreenTimeMs = 2000;
} // namespace

template <typename PARAM_ID>
MenuGui<PARAM_ID>::MenuGui(OledDisplay<SSD130x4WireSpi128x64Driver>& display, Encoder& encoder,
                           MenuItem<PARAM_ID>* menuItems, uint8_t numMenuItems)
    : display_(display), encoder_(encoder), menuItems_(menuItems), numMenuItems_(numMenuItems)
{
}

template <typename PARAM_ID>
void MenuGui<PARAM_ID>::SetSplashScreen(const char* projectName, const char* version)
{
    uiState_ = UIState::SPLASH_SCREEN;
    splashStartTime_ = System::GetNow();
    strncpy(projectName_, projectName, sizeof(projectName_));
    strncpy(versionString_, version, sizeof(versionString_));
}

template <typename PARAM_ID>
void MenuGui<PARAM_ID>::ProcessUiEvent()
{
    if (uiState_ == UIState::SPLASH_SCREEN)
    {
        return;
    }

    int8_t enc_inc = encoder_.Increment();
    bool clicked = encoder_.FallingEdge();

    if (selectionType_ == MenuSelected::LABEL)
    {
        int16_t value = selectedMenuItem_ + enc_inc;
        if (value < 0)
        {
            value = 0;
        }
        else if (value >= numMenuItems_)
        {
            value = numMenuItems_ - 1;
        }

        selectedMenuItem_ = value;
    }
    else
    {
        int16_t value = menuItems_[selectedMenuItem_].value + enc_inc;

        if (value < menuItems_[selectedMenuItem_].min)
        {
            value = menuItems_[selectedMenuItem_].min;
        }
        else if (value > menuItems_[selectedMenuItem_].max)
        {
            value = menuItems_[selectedMenuItem_].max;
        }
        menuItems_[selectedMenuItem_].value = value;
    }

    if (clicked)
    {
        selectionType_ = selectionType_ == MenuSelected::LABEL ? MenuSelected::VALUE : MenuSelected::LABEL;
    }
}

template <typename PARAM_ID>
void MenuGui<PARAM_ID>::Draw()
{
    switch (uiState_)
    {
    case UIState::SPLASH_SCREEN:
        DrawSplashScreen();
        break;
    case UIState::MENU:
        DrawMenu();
        break;
    }
}

template <typename PARAM_ID>
void MenuGui<PARAM_ID>::DrawSplashScreen()
{
    display_.Fill(false);

    const Rectangle kProjectNameRect = {128, 64};
    display_.WriteStringAligned(projectName_, Font_11x18, kProjectNameRect, Alignment::centered, true);

    const Rectangle kVersionRect = {0, 32, 128, 32};
    display_.WriteStringAligned(versionString_, Font_6x8, kVersionRect, Alignment::centered, true);

    display_.Update();

    if (System::GetNow() - splashStartTime_ > kSplashScreenTimeMs)
    {
        uiState_ = UIState::MENU;
    }
}

template <typename PARAM_ID>
void MenuGui<PARAM_ID>::DrawMenu()
{
    constexpr uint8_t kVerticalOffset = 10;
    constexpr uint8_t kLeftMargin = 3;

    const Rectangle kLabelBoundingRect = {kMaxLabelLength * 6, kVerticalOffset};

    const int16_t kValueBoxWidth = 128 - kLabelBoundingRect.GetWidth();
    const Rectangle kValueBoundingRect =
        kLabelBoundingRect.WithX(kLabelBoundingRect.GetWidth()).WithWidth(kValueBoxWidth);

    const uint8_t start_index = selectedMenuItem_ >= kDisplayLineCount ? selectedMenuItem_ - kDisplayLineCount + 1 : 0;
    const uint8_t last_index =
        selectedMenuItem_ + kDisplayLineCount > numMenuItems_ ? numMenuItems_ : selectedMenuItem_ + kDisplayLineCount;

    display_.Fill(false);
    uint8_t row_offset = 0;
    for (int i = start_index; i < last_index; i++)
    {
        const Rectangle rect = kLabelBoundingRect.WithY(row_offset * kVerticalOffset);
        const Rectangle value_rect = kValueBoundingRect.WithY(row_offset * kVerticalOffset);
        bool labelSelected = false;
        bool valueSelected = false;
        if (i == selectedMenuItem_)
        {
            if (selectionType_ == MenuSelected::LABEL)
            {
                display_.DrawRect(rect, true, true);
                labelSelected = true;
            }
            else
            {
                display_.DrawRect(value_rect, true, true);
                valueSelected = true;
            }
        }

        display_.SetCursor(kLeftMargin, row_offset * kVerticalOffset + 2);
        display_.WriteStringAligned(menuItems_[i].label, Font_6x8, rect.WithTrimmedLeft(kLeftMargin),
                                    Alignment::bottomLeft, !labelSelected);

        char value_str[4];
        snprintf(value_str, sizeof(value_str), "%d", menuItems_[i].value);
        display_.WriteStringAligned(value_str, Font_6x8, value_rect, Alignment::bottomRight, !valueSelected);

        ++row_offset;
    }

    display_.Update();
}

template <typename PARAM_ID>
uint8_t MenuGui<PARAM_ID>::GetParamValue(PARAM_ID paramId)
{
    for (uint8_t i = 0; i < numMenuItems_; i++)
    {
        if (menuItems_[i].paramId == paramId)
        {
            return menuItems_[i].value;
        }
    }

    return 0;
}

template <typename PARAM_ID>
void MenuGui<PARAM_ID>::SetParamValue(PARAM_ID paramId, uint8_t value)
{
    for (uint8_t i = 0; i < numMenuItems_; i++)
    {
        if (menuItems_[i].paramId == paramId)
        {
            menuItems_[i].value = value;
            return;
        }
    }
}