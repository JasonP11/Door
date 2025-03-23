#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "Neelam_21";
const char* password = "Jason1234";

WiFiUDP udp;
const int udpPort = 6000;
IPAddress broadcastIP(192, 168, 0, 255);  // Change "192.168.1" to match your network



void FindIP(){
      // Start UDP
    udp.begin(udpPort);

    // Send broadcast message
    Serial.println("Sending broadcast to find Raspberry Pi...");
    udp.beginPacket(broadcastIP, udpPort);
    udp.print("DISCOVER_PI");
    udp.endPacket();

    // Wait for response
    Serial.println("Waiting for response...");
    unsigned long startTime = millis();
    while (millis() - startTime < 10000) {  // Timeout after 5 seconds
        int packetSize = udp.parsePacket();
        if (packetSize) {
            char buffer[16];
            udp.read(buffer, packetSize);
            buffer[packetSize] = '\0';
            Serial.print("Raspberry Pi IP: ");
            Serial.println(buffer);
            return buffer;
        }
        delay(500);
    }

    Serial.println("No response from Raspberry Pi.");
}


void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");
/*
    // Start UDP
    udp.begin(udpPort);

    // Send broadcast message
    Serial.println("Sending broadcast to find Raspberry Pi...");
    udp.beginPacket(broadcastIP, udpPort);
    udp.print("DISCOVER_PI");
    udp.endPacket();

    // Wait for response
    Serial.println("Waiting for response...");
    unsigned long startTime = millis();
    while (millis() - startTime < 10000) {  // Timeout after 5 seconds
        int packetSize = udp.parsePacket();
        if (packetSize) {
            char buffer[16];
            udp.read(buffer, packetSize);
            buffer[packetSize] = '\0';
            Serial.print("Raspberry Pi IP: ");
            Serial.println(buffer);
            return;
        }
        delay(500);
    }

    Serial.println("No response from Raspberry Pi.");
*/
}

void loop() {
  FindIP();
}
