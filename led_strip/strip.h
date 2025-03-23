#include <sys/_stdint.h>
#include <Arduino.h>

class LedStrip {
  public: 
    uint16_t brightness = 0;
    LedStrip(byte pin) {
      _pin = pin;
    }

    void setState(bool);
    void setBrightness(uint16_t);
    void tick();

  private:
    byte _pin;
    uint8_t _aimBrightness = 0;
    uint32_t _timer = 0;
};

void LedStrip::tick() {
  if (millis() - _timer >= 2) {
    if (_aimBrightness > brightness) brightness++;
    if (brightness > _aimBrightness) brightness--;
    analogWrite(_pin, brightness);
    _timer = millis();
  }
}

void LedStrip::setBrightness(uint16_t newBr) {
  _aimBrightness = newBr;
}