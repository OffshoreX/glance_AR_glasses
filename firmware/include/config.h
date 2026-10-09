#pragma once

// ---- I2C / OLED ----
constexpr int SDA_PIN = 4;
constexpr int SCL_PIN = 5;
constexpr uint8_t OLED_I2C_ADDR = 0x3C;
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 32; // real GLANCE panel (not Wokwi's 128x64 default)

// ---- Buttons ----
constexpr int BTN1_PIN = 6;
constexpr int BTN2_PIN = 7;
constexpr unsigned long DEBOUNCE_MS = 30;

// ---- Debug LED ----
constexpr int LED_PIN = 10;
constexpr unsigned long LED_BLINK_INTERVAL_MS = 500;

// ---- Battery ----
constexpr int BATT_ADC_PIN = 3;
// 100k/100k divider halves Vbatt before it reaches the ADC pin, so multiply
// the measured voltage by 2 to recover the actual battery voltage.
constexpr float BATTERY_DIVIDER_RATIO = 2.0f;
constexpr unsigned long BATTERY_READ_INTERVAL_MS = 1000;
