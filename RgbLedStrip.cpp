#include "RgbLedStrip.h"
#include <Arduino.h>

RgbColor::RgbColor(int red, int green, int blue)
: r(red), g(green), b(blue)
{}

String
RgbColor::toString(void)
{
  return "R: " + String(r) + " G: " + String(g) + " B: " + String(b);
}

RgbColor operator+(const RgbColor& a, const RgbColor& b)
{
  RgbColor res (a.r + b.r,
                a.g + b.g,
                a.b + b.b);
  return res;
}

RgbColor operator-(const RgbColor& a, const RgbColor& b)
{
  RgbColor res (a.r - b.r,
                a.g - b.g,
                a.b - b.b);
  return res;
}

RgbColor operator/(const RgbColor& a, const int s)
{
  RgbColor res (a.r / s,
                a.g / s,
                a.b / s);
  return res;
}


RgbLedStrip::RgbLedStrip(int pinR, int pinG, int pinB)
: io(pinR, pinG, pinB)
, currentState(0,0,0)
, fadeState(0,0,0)
, stepsRem(0)
{
  analogWriteRange(255);
  set_color(currentState);
}

String
RgbLedStrip::toString(void)
{
  return "LED: " + currentState.toString() + '\n'
       + "to   " + fadeState.toString() + "in " + String(stepsRem) + '\n';
}

void
RgbLedStrip::set_color(const RgbColor& c)
{
  currentState = c;
  analogWrite(io.r, currentState.r);
  analogWrite(io.g, currentState.g);
  analogWrite(io.b, currentState.b);
}

void
RgbLedStrip::set_color(int r, int g, int b)
{
  const RgbColor newstate(r, g, b);
  set_color(newstate);
}

void
RgbLedStrip::fade_to(RgbColor c, int steps)
{
  fadeState = (currentState - c) / steps;
  stepsRem = steps;
}

void
RgbLedStrip::fade_to(int r, int g, int b, int steps)
{
  fade_to(RgbColor(r, g, b), steps);
}

void
RgbLedStrip::fade_step(void)
{
  if (stepsRem <= 0)
    return;
  set_color(currentState - fadeState);
  stepsRem--;
}
