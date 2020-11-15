#include "RgbLedStrip.h"

#include <Arduino.h>

RgbLedStrip::RgbLedStrip(int pinR, int pinG, int pinB)
{
  mpr = pinR; mpg = pinG; mpb = pinB;
  set_color(0,0,0);
}

void
RgbLedStrip::set_color(int r, int g, int b)
{
  analogWrite(mpr, r);
  analogWrite(mpg, g);
  analogWrite(mpb, b);
}
