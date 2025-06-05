/************ ESP 8266******************/

#include <FS.h>      
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>
#include <EEPROM.h>

WiFiManager wifiManager;
char serverIP[40] ; // Set a default value
WiFiManagerParameter custom_server_ip("server", "Server IP Address", serverIP, 40);
String serverUrl = "";  // Declare as empty global


const int buttonPin = D4;  // Button connected to D4 and GND
bool lastButtonState = HIGH;  // Default to HIGH due to pull-up

const int resetPin = D3;  // GPIO0
bool lastResetPinState = HIGH;

// Function to save the server IP to EEPROM
void saveServerIP(const char* ip) {
  EEPROM.begin(512);
  for (int i = 0; i < 40; ++i) {
    EEPROM.write(i, ip[i]);
    if (ip[i] == '\0') break; // Stop at the null terminator
  }
  EEPROM.commit();
  Serial.println("Server IP saved to EEPROM.");
}

// Function to load the server IP from EEPROM
void loadServerIP() {
  EEPROM.begin(512);
  for (int i = 0; i < 40; ++i) {
    serverIP[i] = EEPROM.read(i);
  }
  if (strlen(serverIP) == 0) {
    strcpy(serverIP, "192.168.1.100"); // Set default only if EEPROM is empty
  }
  Serial.print("Loaded Server IP from EEPROM: ");
  Serial.println(serverIP);
}


void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  // Enable internal pull-up resistor
  pinMode(resetPin, INPUT_PULLUP);  // <-- Add this line
  Serial.begin(115200);
  loadServerIP();
  WiFiManagerParameter custom_server_ip("server", "Server IP Address", serverIP, 40);
  wifiManager.addParameter(&custom_server_ip);

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
 

    // Get the server IP from the WiFiManager portal
  const char* newServerIP = custom_server_ip.getValue();
  Serial.print("Server IP from Portal: ");
  Serial.println(newServerIP);

  // Save the server IP to EEPROM only if it has changed and not empty
  if (strlen(newServerIP) > 0 && strcmp(newServerIP, serverIP) != 0) {
    strcpy(serverIP, newServerIP);
    saveServerIP(serverIP);
  }
  
  serverUrl = "http://" + String(serverIP) + ":5000/";
  Serial.print("Final server URL: ");
  Serial.println(serverUrl);
}

void loop() {
  bool buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {  // Detects a button press (transition from HIGH to LOW)
    Serial.println("Button pressed, sending bypass request...");
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;
      http.begin(client, serverUrl+"/bypass");
      String postData = "{\"door_id\": \"1\", \"password\": \"blank\"}";

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
    bool resetPinState = digitalRead(resetPin);
  if (resetPinState == LOW && lastResetPinState == HIGH) {
    Serial.println("Reset pin triggered, starting WiFiManager config portal...");
    wifiManager.resetSettings();  // Clear saved WiFi
    delay(1000); // Debounce delay
    ESP.restart();  // Restart to trigger WiFiManager again in setup()
  }
  lastResetPinState = resetPinState;
  delay(100);  // Small delay to debounce the button
}
