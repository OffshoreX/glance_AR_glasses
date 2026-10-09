// Glance - display module test (Wokwi simulation only)
// ESP32-C3, Arduino framework. No BLE/WiFi/power management here on purpose.

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---- Pins (fixed by the Wokwi wiring) ----
const int SDA_PIN = 4;
const int SCL_PIN = 5;
const int BTN1_PIN = 7;
const int BTN2_PIN = 6;
const int LED_PIN = 10;

// ---- Screen ----
// Wokwi's OLED is 128x64; the real Glance module is 128x32.
// Changing SCREEN_HEIGHT is the only edit needed to switch.
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const uint8_t OLED_ADDR = 0x3C;

const unsigned long DEBOUNCE_MS = 30;
const unsigned long BOOT_SCREEN_MS = 2000;
const unsigned long FAST_BLINK_MS = 100;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Debounced state per button: last raw read, last debounced (stable) value,
// and when the raw read last changed.
int btn1RawLast = HIGH, btn1Stable = HIGH;
unsigned long btn1LastChange = 0;

int btn2RawLast = HIGH, btn2Stable = HIGH;
unsigned long btn2LastChange = 0;

int counter = 0;
bool ledState = false;
bool needsRedraw = true;

// Debounces one button and advances its stable state.
// Returns true if the stable state changed this call; when it does,
// `pressed` tells the caller whether the change was a HIGH->LOW press edge
// (true) or a release edge (false).
bool updateButton(int pin, int &rawLast, int &stable, unsigned long &lastChange, bool &pressed) {
  pressed = false;
  int raw = digitalRead(pin);

  if (raw != rawLast) {
    lastChange = millis();
    rawLast = raw;
  }

  if ((millis() - lastChange) > DEBOUNCE_MS && stable != raw) {
    int prevStable = stable;
    stable = raw;
    pressed = (prevStable == HIGH && stable == LOW);
    return true;
  }
  return false;
}

// Blinks the debug LED forever. Used when display init fails so the board
// never hangs silently.
void haltBlinking(const char *msg) {
  Serial.println(msg);
  while (true) {
    digitalWrite(LED_PIN, HIGH);
    delay(FAST_BLINK_MS);
    digitalWrite(LED_PIN, LOW);
    delay(FAST_BLINK_MS);
  }
}

void drawMainScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("B1: ");
  display.println(btn1Stable == LOW ? "DOWN" : "UP");

  display.setCursor(0, 10);
  display.print("B2: ");
  display.println(btn2Stable == LOW ? "DOWN" : "UP");

  display.setCursor(0, 20);
  display.print("Count: ");
  display.println(counter);

  display.display();
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    haltBlinking("ERROR: SSD1306 init failed");
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Glance");
  display.println("display test");
  display.display();
  delay(BOOT_SCREEN_MS); // boot screen only, not the main loop
  display.clearDisplay();
  display.display();

  // Seed stable state from the current reading so a button already held
  // at boot doesn't register as a false press on the first loop.
  btn1Stable = btn1RawLast = digitalRead(BTN1_PIN);
  btn2Stable = btn2RawLast = digitalRead(BTN2_PIN);
}

void loop() {
  bool pressed;

  if (updateButton(BTN1_PIN, btn1RawLast, btn1Stable, btn1LastChange, pressed)) {
    needsRedraw = true;
    if (pressed) {
      counter++;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
      Serial.println("Button 1 pressed");
    }
  }

  if (updateButton(BTN2_PIN, btn2RawLast, btn2Stable, btn2LastChange, pressed)) {
    needsRedraw = true;
    if (pressed) {
      counter = 0;
      Serial.println("Button 2 pressed");
    }
  }

  if (needsRedraw) {
    drawMainScreen();
    needsRedraw = false;
  }
}