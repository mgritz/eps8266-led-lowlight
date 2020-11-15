#ifndef _RGBLEDSTRIP_H_
#define _RGBLEDSTRIP_H_

class RgbLedStrip {
public:
  RgbLedStrip(int pinR, int pinG, int pinB);
  void set_color(int r, int g, int b);

private:
  RgbLedStrip();
  int mpr, mpg, mpb;
  bool minvert;
};


#endif
