#include "RgbLedStrip.h"
#include <Arduino.h>

RgbColor::RgbColor(float red, float green, float blue)
: r(red), g(green), b(blue)
{}

RgbColor::RgbColor(String rgb)
{
  if (rgb[0] == '#')
    rgb = rgb.substring(1);

  r = strtol(rgb.substring(0,2).c_str(), NULL, 16);
  g = strtol(rgb.substring(2,4).c_str(), NULL, 16);
  b = strtol(rgb.substring(4,6).c_str(), NULL, 16);
}

String
RgbColor::toString(void)
{
  String rv = "#";

  const String sr = String(int(r), HEX);
  rv += (sr.length() == 1) ? ("0" + sr) : sr;
  const String sg = String(int(g), HEX);
  rv += (sg.length() == 1) ? ("0" + sg) : sg;
  const String sb = String(int(b), HEX);
  rv += (sb.length() == 1) ? ("0" + sb) : sb;

  return rv;
}

String
RgbColor::toStepString(void)
{
  return "R(" + String(r, 3) + ")G(" + String(g, 3) + ")B(" + String(b, 3) + ")";
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
, fadeStep(0,0,0)
, stepsRem(0)
{
  analogWriteRange(255);
  set_color(currentState);
}

String
RgbLedStrip::toString(void)
{
  return "LED   " + currentState.toString() + '\n'
       + String(stepsRem) + " steps of " + fadeStep.toStepString() + '\n';
}

void
RgbLedStrip::set_color(const RgbColor& c)
{
  currentState = c;
  analogWrite(io.r, int(currentState.r));
  analogWrite(io.g, int(currentState.g));
  analogWrite(io.b, int(currentState.b));
}

void
RgbLedStrip::set_color(float r, float g, float b)
{
  const RgbColor newstate(r, g, b);
  set_color(newstate);
}

void
RgbLedStrip::fade_to(RgbColor c, int steps)
{
  fadeStep = (currentState - c) / steps;
  stepsRem = steps;
}

void
RgbLedStrip::fade_to(float r, float g, float b, int steps)
{
  fade_to(RgbColor(r, g, b), steps);
}

void
RgbLedStrip::fade_step(void)
{
  if (stepsRem <= 0)
    return;
  set_color(currentState - fadeStep);
  stepsRem--;
}
