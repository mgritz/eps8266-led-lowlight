// Wir laden die uns schon bekannte WiFi Bibliothek
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
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

// Die verwendeted GPIO Pins
// D1 = GPIO5 und D2 = GPIO4 - einfach bei Google nach "Amica Pinout" suchen
#define output4 D4
#define input0 D0

LedConfig led_cfg = {
  .on_color = "#ff0000",
  .time_from_min = 0,
  .time_from_hr = 20,
  .time_to_min = 0,
  .time_to_hr = 8,
  .turn_off_delay_s = 10,
  .fader_speed = 24,
};

RgbLedStrip strip(D5, D4, D6);

void setup() {
  Serial.begin(115200);
  // Die definierten GPIO Pins als output definieren ...
  pinMode(output4, OUTPUT);
  // ... und erstmal auf LOW setzen
  digitalWrite(output4, LOW);

  // initialize config settings
  // TODO load from eeprom

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
  fader_time = millis() + 250;
}

#define LED_COLOR_OFF RgbColor(0, 0, 0)

unsigned long last_movement = 0;
bool last_on = false;

void lowlight_loop_body() {

  //
  // Keep track of local time and decide about mode
  //

  if(timeClient.update())
    Serial.println("NTP: now " + timeClient.getFormattedTime());
  // Even if false, time may be correct - NTPClient::update complies with the
  // update period configured in the constructor. The time getters extrapolate
  // from the last update using millis().

  const bool is_night = true; // TODO select from time range using timeClinet.getHours / .getMinutes

  // enable lowlight if it is night, or if NTP was never successful.
  const bool lowlight_enabled = (is_night || (! timeClient.isTimeSet()));

  //
  // Update LED fading.
  //

  const unsigned long now = millis();
  const bool movement = (digitalRead(input0) == HIGH);

  if (lowlight_enabled && movement) {
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
