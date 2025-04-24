#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <map>

const char* ssid = "Neelam_21";
const char* password = "Jason1234";
const char* serverUrl = "http://192.168.0.42:5000/get_uids";
// String scannedUid = "072776B4"; // Example UID to check

std::map<String, String> uidMap;

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
    return;
  }

  // Fetch JSON from server and save to file
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;
    http.begin(client, serverUrl);
    int httpCode = http.GET();

    if (httpCode == 200) {
      String payload = http.getString();
      Serial.println("Received JSON:\n" + payload);
      // Save to file
      File file = LittleFS.open("/uids.json", "w");
      if (!file) {
        Serial.println("Failed to open file for writing");
      } else {
        file.print(payload);
        file.close();
        Serial.println("Saved to /uids.json");
      }
    } else {
      Serial.println("HTTP GET failed");
    }

    http.end();
  }
  printFromFile();
  delay(1000);
  // check();
}

void printFromFile() {
   File file = LittleFS.open("/uids.json", "r");
  if (!file) {
    Serial.println("Failed to open /uids.json for reading");
    return;
  }

  // Read entire file into a string
  String jsonData;
  while (file.available()) {
    jsonData += char(file.read());
  }
  file.close();

  Serial.println("\nContents of /uids.json:");
  Serial.println(jsonData);

  // Now parse the JSON string
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, jsonData);

  if (error) {
    Serial.println("Failed to parse JSON");
    return;
  }

  // Store in map
  for (JsonObject user : doc.as<JsonArray>()) {
    String uid = user["key_id"];
    String role = user["level"];
    uidMap[uid] = role;
  }

  Serial.println("UIDs loaded to map");
}

/*
void check(){
  File file = LittleFS.open("/uids.json", "r");

  if (!file) {
    Serial.println("Failed to open /uids.json for checking");
    return;
  }

  DynamicJsonDocument doc(2048);  // Enough for 100+ entries
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.println("JSON parse failed");
    return;
  }

  bool found = false;
  for (JsonObject user : doc.as<JsonArray>()) {
    String uid = user["key_id"];
    String role = user["level"];

    if (uid == scannedUid) {
      Serial.println("✅ UID matched: " + uid + " - Role: " + role);
      found = true;
      break;
    }
  }

  if (!found) {
    Serial.println("❌ UID not found in the list");
  }
}
*/
void checkUID(const String& scannedUid) {
  if (uidMap.count(scannedUid)) {
    Serial.println("✅ Match: " + scannedUid + " | Role: " + uidMap[scannedUid]);
  } else {
    Serial.println("❌ UID not found: " + scannedUid);
  }
}


void loop() {
  // Nothing for now
  checkUID("072776B4");  // quick!
  delay(2000);
}
