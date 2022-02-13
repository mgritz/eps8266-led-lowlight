// Wir laden die uns schon bekannte WiFi Bibliothek
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include "led-lowlight-config.h"

// Hier geben wir den WLAN Namen (SSID) und den Zugansschlüssel ein
// Definiert "ssid" und "password"
#include "wifipass.h"

#include "website.h"

#include "RgbLedStrip.h"

const long utcOffsetInSeconds = 3600 * 1;
const unsigned long ntpUpdateInterval = 3600 * 1000; //ms

// Wir setzen den Webserver auf Port 80
WiFiServer server(80);
// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, ntpUpdateInterval);

// Eine Variable um den HTTP Request zu speichern
String http_rx_header;

LedConfig led_cfg = {
  .on_color = "#ff0000",
  .time_from_min = 0,
  .time_from_hr = 20,
  .time_to_min = 0,
  .time_to_hr = 8,
  .turn_off_delay_s = 10,
  .fader_speed = 240,
};

RgbLedStrip strip(D5, D4, D6);

void setup() {
  Serial.begin(115200);

  // initialize config settings if they are validly stored
  EEPROM.begin(4095);
  char magic = '#';
  EEPROM.get(0, magic);
  if (magic == '#') {
    Serial.println("Reading EEPROM");
    EEPROM.get(0, led_cfg);
    Serial.println("EEPROM read. Color: " + String(led_cfg.on_color));
  } else {
    EEPROM.put(0, led_cfg);
    EEPROM.commit();
  }

  // Per WLAN mit dem Netzwerk verbinden
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Die IP vom Webserver auf dem seriellen Monitor ausgeben
  Serial.println("");
  Serial.println("WLAN verbunden.");
  Serial.println("IP Adresse: ");
  Serial.println(WiFi.localIP());
  server.begin();
  timeClient.begin();
}

unsigned long fader_time = 0;
void fader_time_push(void) {
  fader_time = millis() + 25;
}

#define LED_COLOR_OFF RgbColor(0, 0, 0)

unsigned long last_movement = 0;
bool last_on = false;

inline bool
is_night(void)
{
  // with no NTP it's always night.
  if (!timeClient.isTimeSet())
    return true;

  const int now_h = timeClient.getHours();
  const int on_h = led_cfg.time_from_hr;
  const int off_h = led_cfg.time_to_hr;

  // we are in the middle of the day
  if ((now_h < on_h) && (now_h > off_h))
    return false;

  // we are in the middle of the night
  if ((now_h > on_h) || (now_h < off_h))
    return true;

  const int now_min = timeClient.getMinutes();
  const int on_min = led_cfg.time_from_min;
  const int off_min = led_cfg.time_to_min;

  // we are in the turn-on hour (evening)
  if (now_h == on_h)
    return (now_min >= on_min);

  // we are in the turn-off hour (morning)
  if (now_h == off_h)
    return (now_min < off_min);
}

void lowlight_loop_body() {

  //
  // Keep track of local time and decide about mode
  //

  if(timeClient.update())
    Serial.println("NTP: now " + timeClient.getFormattedTime());

  //
  // Update LED fading.
  //

  const unsigned long now = millis();
  const bool movement = (digitalRead(D0) == HIGH);

  if (is_night() && movement) {
    last_movement = now;
    if (!last_on) {
      Serial.println("On-Event. Fading to " + String(led_cfg.on_color));
      strip.fade_to(RgbColor(String(led_cfg.on_color)), led_cfg.fader_speed);
      last_on = true;
    }
  } else if (last_on && (!movement) && (now > last_movement + (led_cfg.turn_off_delay_s * 1000))) {
      Serial.println("Off-Event. Fading to " + LED_COLOR_OFF.toString());
    strip.fade_to(LED_COLOR_OFF, led_cfg.fader_speed);
    last_on = false;
  }

  if (now > fader_time) {
    strip.fade_step();
    fader_time_push();
    if (strip.isFading())
      Serial.println(strip.toString());
  }
}

// parse a string for a key and return value. Dividers are '&' and ' '.
// if something is found, trunkate it away from the input string.
String
parseForValue(String *rx, const String &name)
{
  if (rx->length() <= name.length())
    return String();

  int i = rx->indexOf(name);
  if (i < 0) {
    return String();
  }
  i += name.length();

  int e = rx->indexOf('&', i); // ...name=value&nextname...
  if (e < 0)
    e = rx->indexOf(' ', i); // ...name=value HTTP/... (last entry)

  if (e <= i + 1) // ...name=&nextname... (name found but no value) or not found
    return String();
  else {
    String rv = rx->substring(i, e);
    *rx = rx->substring(e);
    return rv;
  }
}

bool in_request = false;

void loop() {

  lowlight_loop_body();

  WiFiClient client = server.available();   // Hört auf Anfragen von Clients
  if (client) {                             // Falls sich ein neuer Client verbindet,
    Serial.println("Neuer Client.");        // Ausgabe auf den seriellen Monitor

    while (client.connected()) {            // wiederholen so lange der Client verbunden ist
      if (client.available()) {             // Fall ein Byte zum lesen da ist,
        char c = client.read();             // lese das Byte, und dann
        Serial.write(c);                    // gebe es auf dem seriellen Monitor aus
        http_rx_header += c;

        // circularly drop everything that is not a http request (starting with GET)
        // required because browsers may send all kinds of bull in addition to the
        // acutal request. This may even include someting that looks exactly like
        // the request but starts with Referer:, and is outdated.
        if (http_rx_header.indexOf("GET ") >= 0)
          in_request = true;
        else if (http_rx_header.length() > 4)
          http_rx_header = http_rx_header.substring(1);

        // parse the string once after a newline ends the GET
        if (in_request && c == '\n') {
          in_request = false;

          String v = parseForValue(&http_rx_header, String("ledcolor=%23"));
          if (v.length() > 0)
          {
            v = "#" + v;
            v.toCharArray(led_cfg.on_color, 8);
            Serial.println("Set color to " + v);
          }

          v = parseForValue(&http_rx_header, String("time_on="));
          if (v.length() > 0)
          {
            htmlTime2hm(v, &led_cfg.time_from_hr, &led_cfg.time_from_min);
            Serial.println("Set time on to " + hmString(led_cfg.time_from_hr, led_cfg.time_from_min));
          }

          v = parseForValue(&http_rx_header, String("time_to="));
          if (v.length() > 0)
          {
            htmlTime2hm(v, &led_cfg.time_to_hr, &led_cfg.time_to_min);
            Serial.println("Set time off to " + hmString(led_cfg.time_to_hr, led_cfg.time_to_min));
          }

          v = parseForValue(&http_rx_header, String("delay="));
          if (v.length() > 0) {
            led_cfg.turn_off_delay_s = v.toInt();
            Serial.println("Set delay to " + v);
          }

          v = parseForValue(&http_rx_header, String("speed="));
          if (v.length() > 0) {
            led_cfg.fader_speed= v.toInt();
            Serial.println("Set speed to " + v);
          }

          EEPROM.put(0, led_cfg);
          EEPROM.commit();
        }

        if (website_buildup_complete(client, c, &led_cfg))
          break;

      }

      lowlight_loop_body();

    }
    // Die Verbindung schließen
    client.stop();
    Serial.println("Client getrennt.");
    Serial.println("");
  }
}
