// Wir laden die uns schon bekannte WiFi Bibliothek
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Hier geben wir den WLAN Namen (SSID) und den Zugansschlüssel ein
// Definiert "ssid" und "password"
#include "wifipass.h"

const long utcOffsetInSeconds = 3600 * 2;
#include "website.h"


// Wir setzen den Webserver auf Port 80
WiFiServer server(80);
// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Eine Variable um den HTTP Request zu speichern
String http_rx_header;

// Hier wird der aktuelle Status des Relais festgehalten
String output4State = "off";

// Die verwendeted GPIO Pins
// D1 = GPIO5 und D2 = GPIO4 - einfach bei Google nach "Amica Pinout" suchen
#define output4 D4

void setup() {
  Serial.begin(115200);
  // Die definierten GPIO Pins als output definieren ...
  pinMode(output4, OUTPUT);
  // ... und erstmal auf LOW setzen
  digitalWrite(output4, LOW);

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

void loop() {
  WiFiClient client = server.available();   // Hört auf Anfragen von Clients

  if (client) {                             // Falls sich ein neuer Client verbindet,
    Serial.println("Neuer Client.");        // Ausgabe auf den seriellen Monitor
    // Zeige Uhrzeit
    timeClient.update();
    Serial.println(timeClient.getFormattedTime());

    while (client.connected()) {            // wiederholen so lange der Client verbunden ist
      if (client.available()) {             // Fall ein Byte zum lesen da ist,
        char c = client.read();             // lese das Byte, und dann
        Serial.write(c);                    // gebe es auf dem seriellen Monitor aus
        http_rx_header += c;

        // Hier werden die GPIO Pins ein- oder ausgeschaltet
        if (http_rx_header.indexOf("GET /4/on") >= 0) {
          Serial.println("GPIO 4 on");
          output4State = "on";
          digitalWrite(output4, HIGH);
          http_rx_header = String();
        } else if (http_rx_header.indexOf("GET /4/off") >= 0) {
          Serial.println("GPIO 4 off");
          output4State = "off";
          digitalWrite(output4, LOW);
          http_rx_header = String();
        }

        if (website_buildup_complete(client, c, output4State))
          break;

      }
    }
    // Die Verbindung schließen
    client.stop();
    Serial.println("Client getrennt.");
    Serial.println("");
  }
}
