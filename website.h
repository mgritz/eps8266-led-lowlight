#ifndef _WEBSITE_H_
#define _WEBSITE_H_

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "led-lowlight-config.h"

static inline bool
website_buildup_complete(WiFiClient &client, const char nextChar, LedConfig* cfg)
{
  static String currentLine = "";
  if (nextChar == '\n') {                    // wenn das Byte eine Neue-Zeile Char ist
    // wenn die aktuelle Zeile leer ist, kamen 2 in folge.
    // dies ist das Ende der HTTP-Anfrage vom Client, also senden wir eine Antwort:
    if (currentLine.length() == 0) {
      // HTTP-Header fangen immer mit einem Response-Code an (z.B. HTTP/1.1 200 OK)
      // gefolgt vom Content-Type damit der Client weiss was folgt, gefolgt von einer Leerzeile:
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      client.println();

      // Hier wird nun die HTML Seite angezeigt:
      client.println("<!DOCTYPE html><html>");
      client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
      client.println("<link rel=\"icon\" href=\"data:,\">");

      // Webseiten-Überschrift
      client.println("<body><h1>LED unterm Bett</h1>");
      client.println("<form>");
      client.println("<label for=\"ledcolor\">LED strip color</label>");
      client.println("<input id=\"ledcolor\" type=\"color\" name=\"ledcolor\" value=\"" + String(cfg->on_color) + "\"><br>");

      client.println("<label for=\"time_on\">Start time</label>");
      client.println("<input id=\"time_on\" type=\"time\" name=\"time_on\" value=\"" + hmString(cfg->time_from_hr, cfg->time_from_min) + "\"><br>");

      client.println("<label for=\"time_to\">End time</label>");
      client.println("<input id=\"time_to\" type=\"time\" name=\"time_to\" value=\"" + hmString(cfg->time_to_hr, cfg->time_to_min) + "\"><br>");

      client.println("<label for=\"delay\">Turn-off delay</label>");
      client.println("<input id=\"delay\" type=\"number\" name=\"delay\" min=\"0\" value=\"" + String(cfg->turn_off_delay_s) + "\"><br>");

      client.println("<label for=\"speed\">Fading speed</label>");
      client.println("<input id=\"speed\" type=\"number\" name=\"speed\" min=\"0\" max=\"255\" value=\"" + String(cfg->fader_speed) + "\"><br>");

      client.println("<input type=\"submit\">");
      client.println("</form>");

      client.println("</body></html>");

      // Die HTTP-Antwort wird mit einer Leerzeile beendet
      client.println();
      // und wir verlassen mit einem break die Schleife
      return true;
    } else { // falls eine neue Zeile kommt, lösche die aktuelle Zeile
      currentLine = "";
    }
  } else if (nextChar != '\r') {  // wenn etwas kommt was kein Zeilenumbruch ist,
    currentLine += nextChar;      // füge es am Ende von currentLine an
  }
  return false;
}

#endif /* include guard */
