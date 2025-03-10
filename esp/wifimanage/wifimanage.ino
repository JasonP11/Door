#include <FS.h>                    // SPIFFS for saving credentials (Optional)
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <EEPROM.h>

// WiFiManager and Server IP configuration
WiFiManager wifiManager;
char serverIP[40] = "192.168.1.100"; // Set a default value

// Create a custom parameter for the server IP address
WiFiManagerParameter custom_server_ip("server", "Server IP Address", serverIP, 40);

// Function to save the server IP to EEPROM
void saveServerIP(const char* ip) {
  EEPROM.begin(512);
  for (int i = 0; i < 40; ++i) {
    EEPROM.write(i, ip[i]);
    if (ip[i] == '\0') break; // Stop at the null terminator
  }
  EEPROM.commit();
  Serial.println("Server IP saved to EEPROM.");
}

// Function to load the server IP from EEPROM
void loadServerIP() {
  EEPROM.begin(512);
  for (int i = 0; i < 40; ++i) {
    serverIP[i] = EEPROM.read(i);
  }
  if (strlen(serverIP) == 0) {
    strcpy(serverIP, "192.168.1.100"); // Set default only if EEPROM is empty
  }
  Serial.print("Loaded Server IP from EEPROM: ");
  Serial.println(serverIP);
}

void setup() {
  Serial.begin(115200);

  // Load server IP from EEPROM on startup
  loadServerIP();

  // Add the server IP parameter to WiFiManager with a default value
  WiFiManagerParameter custom_server_ip("server", "Server IP Address", serverIP, 40);
  wifiManager.addParameter(&custom_server_ip);

  // Uncomment to reset WiFi credentials (for testing)
  // wifiManager.resetSettings();

  // Start WiFiManager portal if no connection is found
  if (!wifiManager.autoConnect("ESP_ConfigAP", "password")) {
    Serial.println("Failed to connect. Restarting...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Connected to WiFi!");

  // Get the server IP from the WiFiManager portal
  const char* newServerIP = custom_server_ip.getValue();
  Serial.print("Server IP from Portal: ");
  Serial.println(newServerIP);

  // Save the server IP to EEPROM only if it has changed and not empty
  if (strlen(newServerIP) > 0 && strcmp(newServerIP, serverIP) != 0) {
    strcpy(serverIP, newServerIP);
    saveServerIP(serverIP);
  }
}

void printMemoryUsage() {
  Serial.println(F("===== Memory Usage ====="));
  
  // Heap (Dynamic memory)
  Serial.print(F("Free Heap: "));
  Serial.println(ESP.getFreeHeap());

  // Sketch size
  Serial.print(F("Sketch Size: "));
  Serial.println(ESP.getSketchSize());
  
  Serial.print(F("Free Sketch Space: "));
  Serial.println(ESP.getFreeSketchSpace());

  // Flash chip information
  Serial.print(F("Flash Chip Size: "));
  Serial.println(ESP.getFlashChipSize());
  
  Serial.print(F("Real Flash Size: "));
  Serial.println(ESP.getFlashChipRealSize());

  Serial.print(F("Flash Chip Speed: "));
  Serial.println(ESP.getFlashChipSpeed());

  // SDK Version
  Serial.print(F("SDK Version: "));
  Serial.println(ESP.getSdkVersion());

  Serial.println(F("========================"));
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected");
    Serial.println(serverIP);
    delay(20000);
    printMemoryUsage();
  }
}
