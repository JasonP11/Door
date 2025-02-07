#include <PN532_SPI.h>
#include <PN532.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>

//SCK  5
//MISO 6
//Mosi 7

// Define Chip Select (CS) pins for NFC Readers
#define CS_PIN_1 D2  // Chip Select for NFC Reader 1 (IN)
#define CS_PIN_2 D1  // Chip Select for NFC Reader 2 (OUT)

// Define Buzzer, LED, and Relay Pins
#define BUZZER_PIN D3  // GPIO pin connected to the buzzer
#define LED_PIN D4      // GPIO pin for NeoPixel LEDs
#define RELAY_PIN D0    // GPIO pin connected to the relay

Adafruit_NeoPixel pixels(10, LED_PIN, NEO_GRB + NEO_KHZ800);
PN532_SPI pn532spi_1(SPI, CS_PIN_1);
PN532 nfc_1(pn532spi_1);
PN532_SPI pn532spi_2(SPI, CS_PIN_2);
PN532 nfc_2(pn532spi_2);

// Replace with your network credentials
const char* ssid = "CTPL_Guest";
const char* password = "P@ssw0rd";

// Replace with your Raspberry Pi server's IP
const String serverUrl = "http://192.168.80.90:5000/data";

WiFiClient wifiClient;  // Create a WiFiClient object

void setup() {
  // Initialize Serial Communication
  Serial.begin(115200);
  SPI.begin();

  // Initialize NeoPixel LEDs
  pixels.begin();
  pixels.show();  // Turn off all LEDs initially

  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);  // Buzzer off initially
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Relay off initially

  // Initialize NFC Readers
  initializeNFC(nfc_1, "IN");
  initializeNFC(nfc_2, "OUT");

  // Connect to WiFi
  connectToWiFi();
}

void loop() {
  // Maintain WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  // Update LED animations
  updateLEDs();

  // Handle NFC Reader 1 (IN)
  digitalWrite(CS_PIN_1, LOW);
  digitalWrite(CS_PIN_2, HIGH);
  handleNFCReader(nfc_1, "IN", "1");
  delay(500);

  // Handle NFC Reader 2 (OUT)
  digitalWrite(CS_PIN_1, HIGH);
  digitalWrite(CS_PIN_2, LOW);
  handleNFCReader(nfc_2, "OUT", "2");
  delay(500);
}

void initializeNFC(PN532& nfc, String readerType) {
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("Didn't find NFC Reader ");
    Serial.println(readerType);
    while (1);
  }
  nfc.SAMConfig();
  Serial.print("NFC Reader ");
  Serial.print(readerType);
  Serial.println(" initialized.");
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void handleNFCReader(PN532& nfc, String readerType, String door) {
  uint8_t uid[7];
  uint8_t uidLength;

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
    String uidString = getUIDString(uid, uidLength);
    Serial.print("Card UID from NFC Reader ");
    Serial.print(readerType);
    Serial.print(" (Door ");
    Serial.print(door);
    Serial.print("): ");
    Serial.println(uidString);

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(wifiClient, serverUrl);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String postData = "uid=" + uidString + "&type=" + readerType + "&door=" + door;
      int httpResponseCode = http.POST(postData);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response from server:");
        Serial.println(response);

        if (response.indexOf("Access Granted") >= 0) {
          grantAccess();
        } else {
          denyAccess();
        }
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("WiFi not connected");
    }
  }
}

void grantAccess() {
  Serial.println("Access Granted");
  pixels.fill(pixels.Color(0, 255, 0));  // Set LEDs to green
  pixels.show();
  buzz(1000);  // Long beep
  digitalWrite(RELAY_PIN, LOW);  // Activate relay
  delay(5000);  // Keep door unlocked for 5 seconds
  digitalWrite(RELAY_PIN, HIGH);  // Deactivate relay
  pixels.clear();  // Turn off LEDs
  pixels.show();
}

void denyAccess() {
  Serial.println("Access Denied");
  for (int i = 0; i < 2; i++) {
    pixels.fill(pixels.Color(255, 0, 0));  // Set LEDs to red
    pixels.show();
    buzz(200);  // Short beep
    delay(200);
    pixels.clear();  // Turn off LEDs
    pixels.show();
    delay(200);
  }
}

void buzz(int duration) {
  digitalWrite(BUZZER_PIN, LOW);  // Activate buzzer
  delay(duration);
  digitalWrite(BUZZER_PIN, HIGH);  // Deactivate buzzer
}

String getUIDString(uint8_t* uid, uint8_t uidSize) {
  String uidString = "";
  for (uint8_t i = 0; i < uidSize; i++) {
    if (uid[i] < 0x10) uidString += "0";
    uidString += String(uid[i], HEX);
  }
  uidString.toUpperCase();
  return uidString;
}

void updateLEDs() {
  for (int i = 0; i < 5; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 255));  // Blue
  }
  for (int i = 5; i < 10; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255));  // White
  }
  pixels.show();
}
