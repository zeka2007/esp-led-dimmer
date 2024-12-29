#include <Arduino.h>

class LedStrip {
  public: 
    uint16_t brightness = 0;
    LedStrip(byte pin) {
      _pin = pin;
    }

    void setState(bool);
    void setBrightness(uint16_t);

  private:
    byte _pin;
    uint32_t _timer = 0;
};

void LedStrip::setBrightness(uint16_t newBr) {
  if (millis() - _timer >= 2) {

    if (newBr > brightness) brightness++;
    if (brightness > newBr) brightness--;
    analogWrite(_pin, brightness);
    _timer = millis();
  }
}