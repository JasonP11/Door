#include <ESP8266WiFi.h>
#include <espnow.h>

char myData[32] = "Ready";  // Use a character array for better compatibility

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  Serial.print("Bytes received: ");
  Serial.println(len);

  memcpy(myData, incomingData, len);
  myData[len] = '\0';  // Ensure null termination

  Serial.print("Received Data: ");
  Serial.println(myData);
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW Receiver Initialized");
}

void loop() {
  // Nothing needed in loop, ESP-NOW callback handles reception
}
