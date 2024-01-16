#pragma once

#include <dev/oled_ssd130x.h>
#include <hid/disp/oled_display.h>
#include <hid/encoder.h>

using namespace daisy;

constexpr uint8_t kMaxLabelLength = 16;
constexpr uint8_t kMaxProjectNameLength = 10;
constexpr uint8_t kMaxVersionLength = 16;

template <typename PARAM_ID>
struct MenuItem
{
    const char label[kMaxLabelLength];
    PARAM_ID paramId;
    uint8_t value;
    uint8_t min = 0;
    uint8_t max = 100;
};

template <typename PARAM_ID>
class MenuGui
{
  public:
    MenuGui(OledDisplay<SSD130x4WireSpi128x64Driver>& display, Encoder& encoder, MenuItem<PARAM_ID>* menuItems,
            uint8_t numMenuItems);
    ~MenuGui() = default;

    void SetSplashScreen(const char* projectName, const char* version);

    void ProcessUiEvent();

    void Draw();

    uint8_t GetParamValue(PARAM_ID paramId);
    void SetParamValue(PARAM_ID paramId, uint8_t value);

  private:
    void DrawMenu();
    void DrawSplashScreen();

    enum class MenuSelected
    {
        LABEL,
        VALUE
    } selectionType_ = MenuSelected::LABEL;

    uint8_t selectedMenuItem_ = 0;

    OledDisplay<SSD130x4WireSpi128x64Driver>& display_;
    Encoder& encoder_;

    MenuItem<PARAM_ID>* menuItems_;
    const uint8_t numMenuItems_;

    char projectName_[kMaxProjectNameLength + 1];
    char versionString_[kMaxVersionLength + 1];

    uint32_t splashStartTime_ = 0;

    enum class UIState
    {
        SPLASH_SCREEN,
        MENU
    } uiState_ = UIState::SPLASH_SCREEN;
};

#include "menu_gui.tpp"