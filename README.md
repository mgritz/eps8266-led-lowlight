# eps8266-led-lowlight
Arduino firmware for "LED_unterm_Bett" Low Light Controller

## What is it supposed to do?

During day time the device should do nothing. The LED strips stay off.

At night time, the device is supposed to detect movement from a person
stepping out of bed or entering the bedroom.
A dim light should be faded on smoothly in that case.

The light is supposed to stay on for as long as movement is detected, and a bit
longer. It should smoothly fade to off after a grace period when movement is no
longer detected.

Current time should be accurately tracked.

Configuration options are supposed to be available for the following parameters:
* Color of the LED strip when on, RGB.
* Definition of "night time", two hour/minute settings.
* Duration of the grace period after which the light is to be turned off, seconds.

## How we do it

* For movement detection there are three infrared movement sensors that connect
  to a single digital input through diodes. This input is high as long as any
  of the sensors detects movement.
* LED strip is controlled through three transistors for the color lines.
* Time tracking is done using NTP.
* Configuration is availabe through a website and stored in internal EEPROM.

## Dependencies

* Arduino Library `NTPClient`, for the time.
* esp8266 Board Package, provided by [http://arduino.esp8266.com/stable/package_esp8266com_index.json]
