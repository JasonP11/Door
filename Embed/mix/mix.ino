#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define BUTTON_PIN D3
#define PIN D4           // NeoPixel data pin
#define NUMPIXELS 1      // Number of LEDs in NeoPixel
#define RELAY_PIN D2     // Relay pin

const char* ssid = "Neelam 21";
const char* password = "Jason1234";
const String serverUrl = "http://192.168.0.194:5000/data";

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
WiFiClient wifiClient;

void setup() {
  Serial.begin(115200);

  pixels.begin();        // Initialize the NeoPixel strip
  pixels.setBrightness(20); // Set brightness
  pixels.show();         // Turn off all pixels initially
  
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set button pin as input with internal pull-up resistor
  pinMode(RELAY_PIN, OUTPUT);        // Set relay pin as output
  digitalWrite(RELAY_PIN, HIGH);     // Keep relay pin high initially

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN); // Read the button state
  
  // Default NeoPixel color
  pixels.setPixelColor(0, pixels.Color(0, 0, 255)); 
  pixels.show();

  // Check if button is pressed
  if (buttonState == LOW) {
    digitalWrite(RELAY_PIN, LOW);           // Set relay pin low when button is pressed
    pixels.setPixelColor(0, pixels.Color(255, 255, 255)); // Change LED color
    pixels.show();
    
    delay(1000); // Debounce delay for the button press
    
    Test(); // Call Test function to send data to server
    Serial.println("Button pressed");

    delay(1000); // Additional delay for button press
    digitalWrite(RELAY_PIN, HIGH);          // Set relay pin back to high
  }
}

void Test() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(wifiClient, serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = String("uid=") + "4A 2F 3E 1F" + "&type=" + "IN";
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response from server:");
      Serial.println(response);
      pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Success color
      pixels.show();
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Error color
      pixels.show();
    }
    http.end();  // Close connection
  } else {
    Serial.println("WiFi not connected");
  }
}
