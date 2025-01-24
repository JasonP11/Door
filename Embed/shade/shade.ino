#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
int resetPin = D7;
String tagId = "None";

void setup() {

  digitalWrite(resetPin, HIGH);  // Bring reset high to restart PN532
  delay(2000);  // Wait for 2000 ms (2 seconds) for the PN532 to initialize


  // Start Serial Monitor
  Serial.begin(9600);        
  Serial.println('\n');
  Serial.println("System initialized");

  // Initialize I2C with custom SDA and SCL pins
  Wire.begin(D4, D3);  // SDA = D4 (GPIO2), SCL = D3 (GPIO0)

  // Initialize NFC
  nfc.begin();
}

void loop() {
  // Check if NFC tag is present and print the UID
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    tag.print();  // This prints the details of the tag including UID
    tagId = tag.getUidString();
    Serial.println("Tag UID:");
    Serial.println(tagId);
  }

if (tagId == "4A 2F 3E 1F") {
    Serial.println("Access Granted");
  } else {
    Serial.println("Denied");
  }

  delay(1000);  // Small delay between readings
}
