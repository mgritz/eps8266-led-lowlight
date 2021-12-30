#ifndef _RGBLEDSTRIP_H_
#define _RGBLEDSTRIP_H_

#include <Arduino.h>

class RgbColor {
  public:
  int r, g, b;
  RgbColor(int red, int green, int blue);
  RgbColor(String rgb);
  String toString(void);
  friend RgbColor operator+(const RgbColor& a, const RgbColor& b);
  friend RgbColor operator-(const RgbColor& a, const RgbColor& b);
  friend RgbColor operator/(const RgbColor& a, const int s);
};

class RgbLedStrip {
  public:
    RgbLedStrip(int pinR, int pinG, int pinB);
    void set_color(int r, int g, int b);
    void fade_to(int r, int g, int b, int steps);
    void fade_to(RgbColor c, int steps);
    void fade_step(void);
    String toString(void);
    inline bool isFading(void) const { return (stepsRem > 0); };

  private:
    RgbLedStrip();
    void set_color(const RgbColor& c);

    RgbColor io;
    RgbColor currentState;
    RgbColor fadeState;
    int stepsRem;
};


#endif
