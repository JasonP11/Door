#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "CTPL_Guest";
const char* password = "P@ssw0rd";
const char* serverUrl = "http://192.168.80.84:5000/bypass";  // Replace with server IP and port

const int buttonPin = D4;  // Button connected to D1 and GND
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
