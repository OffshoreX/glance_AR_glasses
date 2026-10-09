// GLANCE board bring-up: proves each piece of the custom PCB works
// (I2C bus, OLED, both buttons, debug LED, battery ADC) before any
// BLE/display/input/power logic is layered on top.
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include "config.h"

// Adafruit_SSD1306 (+ Adafruit GFX) over U8g2: this is one fixed 128x32
// monochrome panel for bring-up, so U8g2's multi-device/multi-font
// abstraction buys nothing here, and this library is already used
// elsewhere in the project (wokwi-sim/sketch.ino) for consistency.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool ledState = false;
unsigned long ledLastToggle = 0;

int btn1RawLast = HIGH, btn1Stable = HIGH;
unsigned long btn1LastChange = 0;
int btn2RawLast = HIGH, btn2Stable = HIGH;
unsigned long btn2LastChange = 0;

unsigned long lastBatteryRead = 0;
float batteryVolts = 0.0f;

bool needsRedraw = false;

void scanI2C() {
  Serial.println("Scanning I2C bus...");
  byte found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  Found device at 0x");
      Serial.println(addr, HEX);
      found++;
    }
  }
  if (found == 0) Serial.println("  No I2C devices found.");
}

// Debounces one button. Returns true exactly once when the debounced
// (stable) state actually changes, on either a press or a release edge.
bool updateButton(int pin, int &rawLast, int &stable, unsigned long &lastChange) {
  int raw = digitalRead(pin);
  if (raw != rawLast) {
    lastChange = millis();
    rawLast = raw;
  }
  if ((millis() - lastChange) > DEBOUNCE_MS && stable != raw) {
    stable = raw;
    return true;
  }
  return false;
}

void updateLed() {
  if (millis() - ledLastToggle >= LED_BLINK_INTERVAL_MS) {
    ledLastToggle = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  }
}

float readBatteryVolts() {
  // analogReadMilliVolts() applies the ESP32's per-chip ADC calibration
  // (eFuse Vref/TP) and returns calibrated millivolts directly. Plain
  // analogRead() returns a raw 0-4095 code whose mapping to volts is
  // non-linear near the ADC's rails, so it needs manual calibration we'd
  // otherwise have to do ourselves.
  uint32_t mv = analogReadMilliVolts(BATT_ADC_PIN);
  return (mv / 1000.0f) * BATTERY_DIVIDER_RATIO;
}

// Re-reads the battery on a timer (not every loop iteration, since the
// loop runs far faster than the battery voltage can meaningfully move)
// and only triggers a redraw if the value actually moved.
void updateBattery() {
  if (millis() - lastBatteryRead < BATTERY_READ_INTERVAL_MS) return;
  lastBatteryRead = millis();

  float v = readBatteryVolts();
  Serial.print("Battery: ");
  Serial.print(v, 2);
  Serial.println(" V");

  if (fabsf(v - batteryVolts) > 0.01f) {
    batteryVolts = v;
    needsRedraw = true;
  }
}

void drawScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("B1:");
  display.print(btn1Stable == LOW ? "DN" : "UP");
  display.print(" B2:");
  display.println(btn2Stable == LOW ? "DN" : "UP");

  display.setCursor(0, 10);
  display.print("Batt: ");
  display.print(batteryVolts, 2);
  display.println("V");

  display.display();
}

void setup() {
  // a. Serial over native USB CDC. Baud rate is meaningless to USB CDC
  // itself (there's no physical UART); it's kept so Serial Monitor tools
  // that still ask for one stay happy.
  Serial.begin(115200);
  Serial.println();
  Serial.println("=== GLANCE board bring-up ===");

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // b. I2C bus + scan.
  Wire.begin(SDA_PIN, SCL_PIN);
  scanI2C();

  // c. OLED init + boot message.
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println("ERROR: SSD1306 init failed");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("GLANCE");
    display.display();
  }

  // Seed debounce state from the current reading so a button already held
  // at boot doesn't register as a false state change on the first loop.
  btn1Stable = btn1RawLast = digitalRead(BTN1_PIN);
  btn2Stable = btn2RawLast = digitalRead(BTN2_PIN);

  // Seed the battery baseline synchronously so the first periodic read in
  // loop() doesn't look like a "change" purely because it differs from an
  // initial placeholder value, which would wipe the GLANCE boot message
  // immediately. The live bring-up screen replaces it once a button is
  // pressed/released or the battery reading actually drifts.
  batteryVolts = readBatteryVolts();
  lastBatteryRead = millis();
  Serial.print("Battery: ");
  Serial.print(batteryVolts, 2);
  Serial.println(" V");
}

void loop() {
  // d. Debug LED blink, millis-based so it never blocks button polling
  // or OLED updates the way delay() would.
  updateLed();

  // e. Buttons: debounce + edge detection, print + redraw only on change.
  if (updateButton(BTN1_PIN, btn1RawLast, btn1Stable, btn1LastChange)) {
    needsRedraw = true;
    Serial.print("Button 1: ");
    Serial.println(btn1Stable == LOW ? "DOWN" : "UP");
  }
  if (updateButton(BTN2_PIN, btn2RawLast, btn2Stable, btn2LastChange)) {
    needsRedraw = true;
    Serial.print("Button 2: ");
    Serial.println(btn2Stable == LOW ? "DOWN" : "UP");
  }

  // f. Battery: read, print, and conditionally redraw.
  updateBattery();

  if (needsRedraw) {
    drawScreen();
    needsRedraw = false;
  }
}
