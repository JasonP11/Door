#include <PN532_SPI.h>
#include <PN532.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Define Chip Select (CS) pins for NFC Readers
#define CS_PIN_1 D2  // Chip Select for NFC Reader 1 (IN)
#define CS_PIN_2 D1  // Chip Select for NFC Reader 2 (OUT)
//MOSI: D7 
//MISO: D6 
//SCK: D5 

// Create SPI instances for NFC readers
PN532_SPI pn532spi_1(SPI, CS_PIN_1);  
PN532 nfc_1(pn532spi_1);

PN532_SPI pn532spi_2(SPI, CS_PIN_2);  
PN532 nfc_2(pn532spi_2);

// Replace with your network credentials
const char* ssid = "Neelam 21";
const char* password = "Jason1234";

// Setup Raspberry Pi server IP address and port
const String serverUrl = "http://192.168.0.194:5000/data";  // Replace with your Raspberry Pi's IP

WiFiClient wifiClient;  // Create a WiFiClient object

void setup() {
  // Initialize Serial communication for debugging
  Serial.begin(115200);

  // Initialize SPI for NFC Readers
  SPI.begin();  // Start SPI bus

  // Initialize NFC Reader 1 (IN)
  nfc_1.begin();
  uint32_t versiondata_1 = nfc_1.getFirmwareVersion();
  if (!versiondata_1) {
    Serial.println("Didn't find NFC Reader 1 (IN)");
    while (1);
  }
  nfc_1.SAMConfig();  // Configure NFC Reader 1

  // Initialize NFC Reader 2 (OUT)
  nfc_2.begin();
  uint32_t versiondata_2 = nfc_2.getFirmwareVersion();
  if (!versiondata_2) {
    Serial.println("Didn't find NFC Reader 2 (OUT)");
    while (1);
  }
  nfc_2.SAMConfig();  // Configure NFC Reader 2

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // Check if a new card is present on NFC Reader 1 (IN)
    handleNFCReader(nfc_1, "IN");
    delay(500);
  // Check if a new card is present on NFC Reader 2 (OUT)
    handleNFCReader(nfc_2, "OUT");
   delay(500);

}

// Function to handle NFC reader input and send data to server
void handleNFCReader(PN532& nfc, String readerType) {
  // Read the NFC tag's UID
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;  // Length of the UID (4 or 7 bytes depending on the card type)
  
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
    // Convert UID to String
    String uidString = getUIDString(uid, uidLength);
    Serial.print("Card UID from NFC Reader ");
    Serial.print(readerType);
    Serial.print(": ");
    Serial.println(uidString);

    // Send the UID and reader type to the server
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(wifiClient, serverUrl);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      // Send a POST request with the UID and reader type (IN/OUT)
      String postData = "uid=" + uidString + "&type=" + readerType;
      int httpResponseCode = http.POST(postData);

      // Check for success and print response
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response from server:");
        Serial.println(response);
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }
      http.end();  // Close connection
    } else {
      Serial.println("WiFi not connected");
    }
  }
}

// Function to get the UID as a plain hex string
String getUIDString(uint8_t* uid, uint8_t uidSize) {
  String uidString = "";
  for (uint8_t i = 0; i < uidSize; i++) {
    if (uid[i] < 0x10) uidString += "0";  // Add leading zero for single digit hex values
    uidString += String(uid[i], HEX);     // Convert byte to hex string
  }
  uidString.toUpperCase();  // Convert to uppercase
  return uidString;
}