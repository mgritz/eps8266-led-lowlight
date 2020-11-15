#ifndef _WEBSITE_H_
#define _WEBSITE_H_

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

static inline bool
website_buildup_complete(WiFiClient &client, const char nextChar, String &output4State)
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
      // Es folgen der CSS-Code um die Ein/Aus Buttons zu gestalten
      // Hier können Sie die Hintergrundfarge (background-color) und Schriftgröße (font-size) anpassen
      client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
      client.println(".button { background-color: #333344; border: none; color: white; padding: 16px 40px;");
      client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
      client.println(".button2 {background-color: #888899;}</style></head>");

      // Webseiten-Überschrift
      client.println("<body><h1>ESP8266 Web Server</h1>");

      // Zeige den aktuellen Status, und AN/AUS Buttons for GPIO 4
      client.println("<p>GPIO 4 - State " + output4State + "</p>");
      // Wenn output4State = off, zeige den EIN Button
      if (output4State == "off") {
        client.println("<p><a href=\"/4/on\"><button class=\"button\">EIN</button></a></p>");
      } else {
        client.println("<p><a href=\"/4/off\"><button class=\"button button2\">AUS</button></a></p>");
      }


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
