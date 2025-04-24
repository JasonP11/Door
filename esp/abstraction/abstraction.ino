#include <PN532_SPI.h>
#include <PN532.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WebServer.h>
#include <espnow.h>

// Define Chip Select (CS) pins for NFC Readers
#define CS_PIN_1 D0  // Chip Select for NFC Reader 1 (IN)
#define CS_PIN_2 D1  // Chip Select for NFC Reader 2 (OUT)
#define LED_COUNT 14 

// Define Buzzer Pin
#define BUZZER_PIN D3  // GPIO pin connected to the buzzer (MH-FMD)
#define PIN D2           // Led
Adafruit_NeoPixel pixels(14, PIN, NEO_GRB + NEO_KHZ800);
#define relayPin D4 

/*
// Function Prototypes (Declare before setup)
void runningLight(uint8_t red, uint8_t green, uint8_t blue, int wait);
void handleRelay();
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len);
void sendIP();
void LED();
void handleNFCReader(PN532& nfc, String readerType);
void checkAndReinitializeNFC(PN532& nfc, String readerName);
void checkRelayTimeout();
String getUIDString(uint8_t* uid, uint8_t uidSize);
void grantAccessBuzzer();
void denyAccessBuzzer();

*/


PN532_SPI pn532spi_1(SPI, CS_PIN_1);  
PN532 nfc_1(pn532spi_1);
PN532_SPI pn532spi_2(SPI, CS_PIN_2);  
PN532 nfc_2(pn532spi_2);

// Replace with your network credentials
const char* ssid = "CTPL_Office";
const char* password = "P@ssw0rd@1";
// const char* ssid = "Neelam_21";
// const char* password = "Jason1234";

// Setup Raspberry Pi server IP address and port
const String serverUrl = "http://192.168.80.91:5000/data";  // Replace with your Raspberry Pi's IP

WiFiClient wifiClient;  // Create a WiFiClient object
ESP8266WebServer server(80);


void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Initialize relay as off

  pinMode(LED_BUILTIN, OUTPUT);  // Set built-in LED as an output

  Serial.begin(115200);
  SPI.begin();
  pixels.begin();        // Initialize the NeoPixel strip
  pixels.show();         // Turn off all pixels initially
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);  // Turn buzzer off initially
  
  // Initialize NFC Reader 1 (IN)
  nfc_1.begin();
/*
  uint32_t versiondata_1 = nfc_1.getFirmwareVersion();
  if (!versiondata_1) {
    Serial.println("Didn't find NFC Reader 1 (IN)");
    while (1);
  }
*/
  nfc_1.SAMConfig();

  // Initialize NFC Reader 2 (OUT)
  nfc_2.begin();

  uint32_t versiondata_2 = nfc_2.getFirmwareVersion(); 
/*
  if (!versiondata_2) {
    Serial.println("Didn't find NFC Reader 2 (OUT)");
    while (1);
  }
*/
  nfc_2.SAMConfig();
 



  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    runningLight(128, 0, 128, 50);  // Show running light until WiFi connects
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  server.on("/relay", HTTP_POST, handleRelay);
  server.begin();
  Serial.begin(115200);
  
  Serial.println(WiFi.localIP()); 

  int wifiChannel = WiFi.channel();
  Serial.print("WiFi Channel: ");
  Serial.println(wifiChannel);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW Receiver Initialized");

  Serial.println(WiFi.macAddress());
  sendIP();
}

void loop() {
    server.handleClient();
    LED();
    checkRelayTimeout();

    // Fully disable both readers before enabling the required one
    digitalWrite(CS_PIN_1, HIGH);
    digitalWrite(CS_PIN_2, HIGH);
    
    digitalWrite(CS_PIN_1, LOW);  // Select Reader 1
    delay(5); // Small delay to allow SPI stabilization
    handleNFCReader(nfc_1, "IN");  // IN reader (Door 1)
    digitalWrite(CS_PIN_1, HIGH);  // Fully disable Reader 1
    // checkAndReinitializeNFC(nfc_1, "IN");
    delay(100); // Small buffer time between switching readers
    
    digitalWrite(CS_PIN_2, LOW);  // Select Reader 2
    delay(5);  // Allow time for SPI stabilization
    handleNFCReader(nfc_2, "OUT");  // OUT reader (Door 2)
    digitalWrite(CS_PIN_2, HIGH);  // Fully disable Reader 2
    // checkAndReinitializeNFC(nfc_2, "OUT");
    delay(100);
}




  // Use a character array for better compatibility

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  char myData[32] = "Ready";
  Serial.print("Bytes received: ");
  Serial.println(len);

  memcpy(myData, incomingData, len);
  myData[len] = '\0';  // Ensure null termination

  Serial.print("Received Data: ");
  Serial.println(myData);
  Serial.println();
}


void sendIP(){
  String IP = WiFi.localIP().toString();
  String MacAdd = WiFi.macAddress();
  const String serverUrl1 = "http://192.168.80.91:5000/setup"; 
  if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(wifiClient, serverUrl1);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      //http.POST(IP);
      String payload = "type=setup&relay_ip=" + IP + "&door=4 " + "&MacAdd=" + MacAdd ;   // Include type parameter
      Serial.println(payload);  
      int httpResponseCode = http.POST(payload);
  }
}

void runningLight(uint8_t red, uint8_t green, uint8_t blue, int wait) {
  for (int i = 4; i < LED_COUNT; i++) {
    pixels.clear();
    pixels.setPixelColor(i, pixels.Color(red, green, blue));
    pixels.show();
    delay(100);
    if (WiFi.status() == WL_CONNECTED){
      break;
    }
  }
}


void handleRelay() {
  digitalWrite(relayPin, HIGH);
  if (server.hasArg("plain")) {
    String command = server.arg("plain");
    if (command == "{\"command\":\"open\"}") {
      digitalWrite(relayPin, HIGH);   // Activate relay
      //delay(5000);
      grantAccessBuzzer();
      for (int j = 0; j < 7; j++) {
            pixels.setPixelColor(j, pixels.Color(0, 255, 0)); // Green color
      }
      unsigned long relayStartTime = millis();
      while (millis() - relayStartTime < 5000) { 
                loop();  // Call loop to keep NFC scanning
                server.send(200, "application/json", "{\"status\":\"Relay Opened\"}");
            }
      digitalWrite(relayPin, LOW);  // Deactivate relay
    } else {
      server.send(400, "application/json", "{\"status\":\"Invalid Command\"}");
    }
  }
}

void checkAndReinitializeNFC(PN532& nfc, String readerName) {
  uint32_t versiondata = nfc.getFirmwareVersion(); // Check if NFC is still responding
  if (!versiondata) {
      SPI.begin(); 
      nfc_1.begin();
      nfc_2.begin();
      nfc_1.SAMConfig();
      nfc_2.SAMConfig();
  }
}


unsigned long relayStartTime = 0;
bool relayActive = false;

void checkRelayTimeout() {
    if (relayActive && millis() - relayStartTime >= 4000) {
        digitalWrite(relayPin, LOW);  // Turn OFF relay after 5 seconds
        relayActive = false;
    }
}


// Function to handle NFC reader input and send data to server
void handleNFCReader(PN532& nfc, String readerType) {
   nfc.SAMConfig(); 
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  
  // Set the door variable based on which reader is being used
  String door = "4";  // Door 1 for IN, Door 2 for OUT

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200)) {
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
                
            digitalWrite(relayPin, HIGH);   // Activate relay
            for (int j = 0; j < 7; j++) {
            pixels.setPixelColor(j, pixels.Color(0, 255, 0)); // Green color
          }
            pixels.show();
            grantAccessBuzzer();
            relayStartTime = millis();
            relayActive = true;          
        } else {
            for (int i = 0; i < 13; i++) {
            pixels.setPixelColor(i, pixels.Color( 255, 0, 0)); // Red color
          }
          pixels.show();
          denyAccessBuzzer();
        }
      } else {
        Serial.print("Error on sending POST: ");
        for (int i = 0; i < 13; i++) {
          pixels.setPixelColor(i, pixels.Color( 255, 255, 0)); // Yellow color
        }
        pixels.show();
        Serial.println(httpResponseCode);
        delay(200);
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
  delay(100);  // Long beep for 1 second
 
  digitalWrite(BUZZER_PIN, HIGH);  // Deactivate buzzer
  for (int j = 0; j < 14; j++) {
    pixels.setPixelColor(j, pixels.Color(0, 255, 0)); // Green color
  }
  pixels.show();

  delay(1000);  // Keep green for 1 second

  LED();  // Reset LEDs back to normal state
}

// Function to beep twice short for access denied
// Function to beep twice short for access denied and blink red light
void denyAccessBuzzer() {
  for (int i = 0; i < 3; i++) {
    // Set all LEDs to red
    for (int j = 0; j < 14; j++) {
      pixels.setPixelColor(j, pixels.Color(255, 0, 0)); // Red color
    }
    pixels.show();
    digitalWrite(BUZZER_PIN, LOW);  // Activate buzzer
    delay(200);  // Short beep for 0.2 seconds

    // Turn off LEDs
    for (int j = 0; j < 14; j++) {
      pixels.setPixelColor(j, pixels.Color(0, 0, 0)); // Turn off
    }
    pixels.show();
    digitalWrite(BUZZER_PIN, HIGH);  // Deactivate buzzer
    delay(50);  // Short pause between beeps and blinks
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

void LED() {
  for (int i = 0; i < 8; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 255)); // blue color
  }
  for (int j = 8; j < 14; j++) {
    pixels.setPixelColor(j, pixels.Color(255, 255, 255)); // white color
  }
  pixels.show();
}


//SCK  5
//MISO 6
//Mosi 7


//#define NUMPIXELS 10     // Number of LEDs


