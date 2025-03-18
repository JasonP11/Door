#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "CTPL_Guest";
const char* password = "P@ssw0rd";
const int relayPin = D4;

ESP8266WebServer server(80);

void handleRelay() {
  digitalWrite(relayPin, HIGH);
  if (server.hasArg("plain")) {
    String command = server.arg("plain");
    if (command == "{\"command\":\"open\"}") {
      //digitalWrite(relayPin, HIGH);  // Activate relay
        // Keep relay on for 5 seconds
      digitalWrite(relayPin, LOW);   // Deactivate relay
      delay(5000);
      digitalWrite(relayPin, HIGH); 
      server.send(200, "application/json", "{\"status\":\"Relay Opened\"}");
    } else {
      server.send(400, "application/json", "{\"status\":\"Invalid Command\"}");
    }
  }
}

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);  // Initialize relay as off

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP()); 

  server.on("/relay", HTTP_POST, handleRelay);
  server.begin();
}

void loop() {
  server.handleClient();
}
