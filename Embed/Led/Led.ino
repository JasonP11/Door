#include <PN532_SPI.h>
#include <PN532.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>


// Define Chip Select (CS) pins for NFC Readers
#define CS_PIN_1 D2  // Chip Select for NFC Reader 1 (IN)
#define CS_PIN_2 D1  // Chip Select for NFC Reader 2 (OUT)

// Define Buzzer Pin
#define BUZZER_PIN D3  // GPIO pin connected to the buzzer (MH-FMD)
#define PIN D4           // Led
Adafruit_NeoPixel pixels(10, PIN, NEO_GRB + NEO_KHZ800);

PN532_SPI pn532spi_1(SPI, CS_PIN_1);  
PN532 nfc_1(pn532spi_1);
PN532_SPI pn532spi_2(SPI, CS_PIN_2);  
PN532 nfc_2(pn532spi_2);

// Replace with your network credentials
const char* ssid = "CTPL_Guest";
const char* password = "P@ssw0rd";

// Setup Raspberry Pi server IP address and port
const String serverUrl = "http://192.168.80.84:5000/data";  // Replace with your Raspberry Pi's IP

WiFiClient wifiClient;  // Create a WiFiClient object

void setup() {
  
  Serial.begin(115200);
  SPI.begin();
  pixels.begin();        // Initialize the NeoPixel strip
  pixels.show();         // Turn off all pixels initially
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);  // Turn buzzer off initially
  
  // Initialize NFC Reader 1 (IN)
  nfc_1.begin();
  uint32_t versiondata_1 = nfc_1.getFirmwareVersion();
  if (!versiondata_1) {
    Serial.println("Didn't find NFC Reader 1 (IN)");
    while (1);
  }
  nfc_1.SAMConfig();

  // Initialize NFC Reader 2 (OUT)
  nfc_2.begin();
  uint32_t versiondata_2 = nfc_2.getFirmwareVersion();
  if (!versiondata_2) {
    Serial.println("Didn't find NFC Reader 2 (OUT)");
    while (1);
  }
  nfc_2.SAMConfig();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  
  LED();

  digitalWrite(CS_PIN_1, LOW);  // Select Reader 1
  digitalWrite(CS_PIN_2, HIGH); // Deselect Reader 2
  handleNFCReader(nfc_1, "IN");  // IN reader (Door 1)
  delay(500);
  digitalWrite(CS_PIN_1, HIGH);  // Deselect Reader 2
  digitalWrite(CS_PIN_2, LOW);  // Select Reader 1
  handleNFCReader(nfc_2, "OUT");  // OUT reader (Door 2)
  delay(500);

}

// Function to handle NFC reader input and send data to server
void handleNFCReader(PN532& nfc, String readerType) {
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  
  // Set the door variable based on which reader is being used
  String door = "1";  // Door 1 for IN, Door 2 for OUT

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

      // Send both type and door in postData
      String postData = "uid=" + uidString + "&type=" + readerType + "&door=" + door;
      int httpResponseCode = http.POST(postData);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response from server:");
        Serial.println(response);

        if (response.indexOf("Access Granted") >= 0) {
            for (int j = 0; j < 5; j++) {
            pixels.setPixelColor(j, pixels.Color(0, 255, 0)); // Green color
          }
          pixels.show();
          //delay(1000); 
          grantAccessBuzzer();
        } else {
            for (int i = 0; i < 10; i++) {
            pixels.setPixelColor(i, pixels.Color( 255, 0, 0)); // Green color
          }
          pixels.show();
          denyAccessBuzzer();
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


// Function to beep once long for access granted
void grantAccessBuzzer() {
  digitalWrite(BUZZER_PIN, LOW);  // Activate buzzer
  delay(1000);  // Long beep for 1 second
  digitalWrite(BUZZER_PIN, HIGH);  // Deactivate buzzer
}

// Function to beep twice short for access denied
// Function to beep twice short for access denied and blink red light
void denyAccessBuzzer() {
  for (int i = 0; i < 2; i++) {
    // Set all LEDs to red
    for (int j = 0; j < 10; j++) {
      pixels.setPixelColor(j, pixels.Color(255, 0, 0)); // Red color
    }
    pixels.show();
    digitalWrite(BUZZER_PIN, LOW);  // Activate buzzer
    delay(200);  // Short beep for 0.2 seconds

    // Turn off LEDs
    for (int j = 0; j < 10; j++) {
      pixels.setPixelColor(j, pixels.Color(0, 0, 0)); // Turn off
    }
    pixels.show();
    digitalWrite(BUZZER_PIN, HIGH);  // Deactivate buzzer
    delay(200);  // Short pause between beeps and blinks
  }
}


// Function to get the UID as a plain hex string
String getUIDString(uint8_t* uid, uint8_t uidSize) {
  String uidString = "";
  for (uint8_t i = 0; i < uidSize; i++) {
    if (uid[i] < 0x10) uidString += "0";
    uidString += String(uid[i], HEX);
  }
  uidString.toUpperCase();
  return uidString;
}


//SCK  5
//MISO 6
//Mosi 7







//#define NUMPIXELS 10     // Number of LEDs



void LED() {
  // Set all LEDs to green
  for (int i = 0; i < 5; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 255)); // Green color
  }
      // Update the LED strip to show the green color


  // Set all LEDs to green
  for (int j = 5; j < 10; j++) {
    pixels.setPixelColor(j, pixels.Color(255, 255, 255)); // Green color
  }
  pixels.show();         // Update the LED strip to show the green color
}

