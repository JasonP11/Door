/*
#include <Adafruit_NeoPixel.h>

#define PIN D2           // Ensure the data pin is correct
#define NUMPIXELS 14     // Number of LEDs

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();        // Initialize the NeoPixel strip
  pixels.show();         // Turn off all pixels initially

}  

void loop() {
  // Set the first 5 LEDs to green
  for (int i = 0; i < 5; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // Green color
  }

  // Set the last 5 LEDs to white
  for (int j = 5; j < 10; j++) {
    pixels.setPixelColor(j, pixels.Color(255, 255, 255)); // White color
  }

  pixels.show();         // Update the LED strip to show the colors
    delay(1000);  
}


#include <Adafruit_NeoPixel.h>

#define PIN D2          // Use GPIO4 (D2) for better stability
#define NUMPIXELS 14    // Ensure this matches your LED strip length

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();
}

void loop() {
  pixels.clear();  // Clear all LEDs

  // Set the first 7 LEDs to green
  for (int i = 0; i < 7; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // Green
  }

  // Set the last 7 LEDs to white
  for (int j = 7; j < NUMPIXELS; j++) {
    pixels.setPixelColor(j, pixels.Color(255, 255, 255)); // White
  }

  pixels.show();   // Update the LED strip
  delay(1000);     // Delay to prevent excessive refresh rate
}
*/

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#define LED_PIN D2    // Pin connected to the data pin of the NeoPixel
#define LED_COUNT 14   // Number of LEDs in the strip

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// WiFi credentials
const char* ssid = "Neelam_21";
const char* password = "Jason1234";

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    runningLight(128, 0, 128, 50);  // Show running light until WiFi connects
    Serial.println("Connecting to WiFi...");
    //delay(1000);
  }
  strip.clear();
  strip.show(); // Turn off LEDs once connected
}

void loop() {
  // Normal operation after WiFi connects
}

void runningLight(uint8_t red, uint8_t green, uint8_t blue, int wait) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.clear();
    strip.setPixelColor(i, strip.Color(red, green, blue));
    strip.show();
    delay(100);
    if (WiFi.status() == WL_CONNECTED){
      break;
    }
  }
}

