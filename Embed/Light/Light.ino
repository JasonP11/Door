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
*/

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
