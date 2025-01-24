#include <Adafruit_NeoPixel.h>

#define PIN D4           // Ensure the data pin is correct
#define NUMPIXELS 10     // Number of LEDs

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();        // Initialize the NeoPixel strip
  pixels.show();         // Turn off all pixels initially

  // Set the first 5 LEDs to green
  for (int i = 0; i < 5; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // Green color
  }

  // Set the last 5 LEDs to white
  for (int j = 5; j < 10; j++) {
    pixels.setPixelColor(j, pixels.Color(255, 255, 255)); // White color
  }

  pixels.show();         // Update the LED strip to show the colors
}

void loop() {
  // Nothing to do here, as LEDs are static
}
