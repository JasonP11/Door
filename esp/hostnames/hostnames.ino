#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

const char* ssid = "Neelam_21";
const char* password = "Jason1234";

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");

    if (MDNS.begin("esp8266")) {  // ESP8266 will be accessible as esp8266.local
        Serial.println("mDNS responder started");
    }
}

void loop() {
    MDNS.update();
}
