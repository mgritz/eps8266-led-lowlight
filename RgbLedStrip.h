#ifndef _RGBLEDSTRIP_H_
#define _RGBLEDSTRIP_H_

#include <Arduino.h>

class RgbColor {
  public:
  float r, g, b;
  RgbColor(float red, float green, float blue);
  RgbColor(String rgb);
  String toString(void);
  String toStepString(void);
  friend RgbColor operator+(const RgbColor& a, const RgbColor& b);
  friend RgbColor operator-(const RgbColor& a, const RgbColor& b);
  friend RgbColor operator/(const RgbColor& a, const float s);
};

class RgbLedStrip {
  public:
    RgbLedStrip(int pinR, int pinG, int pinB);
    void set_color(float r, float g, float b);
    void fade_to(float r, float g, float b, int steps);
    void fade_to(RgbColor c, int steps);
    void fade_step(void);
    String toString(void);
    inline bool isFading(void) const { return (stepsRem > 0); };

  private:
    RgbLedStrip();
    void set_color(const RgbColor& c);

    RgbColor io;
    RgbColor currentState;
    RgbColor fadeStep;
    int stepsRem;
};


#endif
