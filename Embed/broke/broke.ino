#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
int resetPin = D7;  // Define reset pin
String tagId = "None";

// Replace with your network credentials
const char* ssid = "Neelam 21";
const char* password = "Jason1234";

// Setup Flask server URL
const String serverUrl = "http://192.168.0.194:5000/data"; // Replace with your computer's IP address

void setup() {
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, HIGH);
  delay(2000);

  Serial.begin(9600);
  Serial.println("System initialized");

  Wire.begin(D4, D3);  // SDA = D4, SCL = D3
  nfc.begin();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    tagId = tag.getUidString();  // Get UID as a string
    Serial.print("Found a tag: ");
    Serial.println(tagId);

    // Send UID to the server
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      WiFiClient wifiClient;  // Create a WiFiClient object
      http.begin(wifiClient, serverUrl);  // Use the WiFiClient object

      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String postData = "uid=" + tagId;  // Send UID
      int httpResponseCode = http.POST(postData);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response from server: " + response);
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }
      http.end();  // Close connection
    } else {
      Serial.println("WiFi not connected");
    }

    delay(1000);  // Small delay between readings
  }
}
