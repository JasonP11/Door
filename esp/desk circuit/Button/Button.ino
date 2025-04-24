/*
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "CTPL_Office";
const char* password = "P@ssw0rd@1";
const char* serverUrl = "http://192.168.80.90:5000/bypass";  // Replace with server IP and port

const int buttonPin = D4;  // Button connected to D4 and GND
bool lastButtonState = HIGH;  // Default to HIGH due to pull-up

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  // Enable internal pull-up resistor
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  bool buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {  // Detects a button press (transition from HIGH to LOW)
    Serial.println("Button pressed, sending bypass request...");
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;
      http.begin(client, serverUrl);
      int httpResponseCode = http.POST("");
      if (httpResponseCode > 0) {
        Serial.println("Bypass request sent successfully");
      } else {
        Serial.println("Failed to send bypass request");
      }
      http.end();
    }
  }
  lastButtonState = buttonState;
  delay(100);  // Small delay to debounce the button
}
*/
#include <FS.h>      
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>
#include <EEPROM.h>

WiFiManager wifiManager;
//const char* ssid = "CTPL_Office";
//const char* password = "P@ssw0rd@1";
const char* serverUrl = "http://192.168.80.91:5000/bypass";  // Replace with server IP and port

const int buttonPin = D4;  // Button connected to D4 and GND
bool lastButtonState = HIGH;  // Default to HIGH due to pull-up

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  // Enable internal pull-up resistor
  Serial.begin(115200);
    if (!wifiManager.autoConnect("ESP_ConfigAP", "password")) {
    Serial.println("Failed to connect. Restarting...");
    delay(3000);
    ESP.restart();
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  bool buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {  // Detects a button press (transition from HIGH to LOW)
    Serial.println("Button pressed, sending bypass request...");
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;
      http.begin(client, serverUrl);
      String postData = "{\"door_id\": \"0\", \"password\": \"securepass\"}";

      Serial.println(postData);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(postData);

      if (httpResponseCode > 0) {
        Serial.println("Bypass request sent successfully");
      } else {
        Serial.println("Failed to send bypass request");
      }
      http.end();
    }
  }
  lastButtonState = buttonState;
  delay(100);  // Small delay to debounce the button
}