#include <PN532_SPI.h>
#include <PN532.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>

// Define Chip Select (CS) pins for NFC Readers
#define CS_PIN_1 D0  // Chip Select for NFC Reader 1 (IN)
#define CS_PIN_2 D1  // Chip Select for NFC Reader 2 (OUT)

// Define Buzzer Pin
#define BUZZER_PIN D3  
#define PIN D2         
Adafruit_NeoPixel pixels(14, PIN, NEO_GRB + NEO_KHZ800);
#define relayPin D4 

PN532_SPI pn532spi_1(SPI, CS_PIN_1);  
PN532 nfc_1(pn532spi_1);
PN532_SPI pn532spi_2(SPI, CS_PIN_2);  
PN532 nfc_2(pn532spi_2);

// WiFi credentials
const char* ssid = "Neelam_21";
const char* password = "Jason1234";

// MQTT broker details
const char* mqtt_server = "192.168.0.42";
const int mqtt_port = 1883;
const char* mqtt_topic = "nfc/access";
const char* mqtt_response_topic = "nfc/response";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);  
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  SPI.begin();
  pixels.begin();
  pixels.show();
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);

  nfc_1.begin();
  if (!nfc_1.getFirmwareVersion()) {
    Serial.println("Didn't find NFC Reader 1 (IN)");
    while (1);
  }
  nfc_1.SAMConfig();

  nfc_2.begin();
  if (!nfc_2.getFirmwareVersion()) {
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

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
  reconnectMQTT();
}

void loop() {
  LED();

  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  digitalWrite(CS_PIN_1, LOW);
  delay(5);
  handleNFCReader(nfc_1, "IN");
  digitalWrite(CS_PIN_1, HIGH);
  delay(100);

  digitalWrite(CS_PIN_2, LOW);
  delay(5);
  handleNFCReader(nfc_2, "OUT");
  digitalWrite(CS_PIN_2, HIGH);
  delay(100);
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266_Client")) {
      Serial.println("connected");
      client.subscribe(mqtt_response_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println("Message received: " + message);

  if (message.indexOf("Access Granted") >= 0) {
    for (int j = 0; j < 7; j++) {
      pixels.setPixelColor(j, pixels.Color(0, 255, 0));
    }
    pixels.show();
    grantAccessBuzzer();
    digitalWrite(relayPin, LOW);
    delay(5000);
    digitalWrite(relayPin, HIGH);
  } else {
    for (int i = 0; i < 13; i++) {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    }
    pixels.show();
    denyAccessBuzzer();
  }
}

void handleNFCReader(PN532& nfc, String readerType) {
  nfc.SAMConfig();
  uint8_t uid[7];
  uint8_t uidLength;

  String door = (readerType == "IN") ? "1" : "2";

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200)) {
    String uidString = getUIDString(uid, uidLength);
    Serial.println("Card UID: " + uidString);

    String payload = "{\"uid\":\"" + uidString + "\", \"type\":\"" + readerType + "\", \"door\":\"" + door + "\"}";
    client.publish(mqtt_topic, payload.c_str());
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


void LED() {
  // Set all LEDs to green
  for (int i = 9; i < 14; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 255)); // blue color
  }
      // Update the LED strip to show the green color


  // Set all LEDs to green
  for (int j = 0; j < 9; j++) {
    pixels.setPixelColor(j, pixels.Color(255, 255, 255)); // white color
  }
  pixels.show();         // Update the LED strip to show the green color
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
