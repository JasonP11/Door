#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.mode(WIFI_STA);
  Serial.print("ESP MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
}