#include <bluefruit.h>

#define BUTTON_NEXT D0
#define DEBOUNCE_TIME 50
#define MAX_IDLE_MS (10UL * 60UL * 1000UL)  // 10 minutes

BLEHidAdafruit blehid;
BLEDis bledis;
const char SEND_CHR = char(39);

bool deviceConnected = false;
bool lastButtonState = HIGH;
unsigned long lastActivity = 0;

/* ---------------- Callbacks ---------------- */

void connect_callback(uint16_t conn_handle) {
  deviceConnected = true;
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  deviceConnected = false;
}

/* ---------------- Sleep ---------------- */

void goToSleep() {
  nrf_gpio_cfg_sense_input(
    digitalPinToPinName(BUTTON_NEXT),
    NRF_GPIO_PIN_PULLUP,
    NRF_GPIO_PIN_SENSE_LOW
  );

  delay(10);
  sd_power_system_off();
}

/* ---------------- Setup ---------------- */

void setup() {
  pinMode(BUTTON_NEXT, INPUT);
  lastActivity = millis();

  Bluefruit.begin();
  Bluefruit.setName("HarpFlip");
  Bluefruit.setTxPower(4);

  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // HID Keyboard
  blehid.begin();

  // Optional device info
  bledis.setManufacturer("DIY");
  bledis.setModel("BLE Page Turner");
  bledis.begin();

  // Advertising
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.start(0);
}

/* ---------------- Loop ---------------- */

void loop() {
  bool buttonState = digitalRead(BUTTON_NEXT);

  if (deviceConnected) {
    // Trigger on press (HIGH → LOW)
    if (buttonState == LOW && lastButtonState == HIGH) {
     uint8_t keys[6] = { HID_KEY_ARROW_RIGHT, 0, 0, 0, 0, 0 };
      blehid.keyboardReport(0, keys);
      delay(5);

      uint8_t none[6] = { 0, 0, 0, 0, 0, 0 };
      blehid.keyboardReport(0, none);

      lastActivity = millis();
    }
  }

  lastButtonState = buttonState;

  if (millis() - lastActivity > MAX_IDLE_MS) {
    goToSleep();
  }

  delay(DEBOUNCE_TIME);
}
