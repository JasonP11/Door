
# 🚪 Door Access System using ESP8266 and Raspberry Pi

This project implements a **smart door access system** combining the **ESP8266 microcontroller** and a **Raspberry Pi** server. The system ensures secure, remote-controlled entry access using Wi-Fi communication between the ESP8266 and Raspberry Pi.

## ✨ Features

- 🛡️ Secure door lock/unlock system
- 📡 Wireless communication (ESP8266 → Raspberry Pi)
- 🔐 Authentication/verification logic handled by Raspberry Pi
- 📈 Real-time status updates
- 💾 Logging of access events
- 🛠️ Easy to set up and expand

## 🛠️ Tech Stack

- **ESP8266** (NodeMCU / similar)
- **Raspberry Pi** (any model with Wi-Fi support)
- **Wi-Fi Communication** (HTTP/MQTT based)
- **Python** (for Raspberry Pi server)
- **Arduino IDE** (for programming ESP8266)

## 🚀 Getting Started

### Requirements

- ESP8266 board (NodeMCU, Wemos D1 mini, etc.)
- Raspberry Pi (with Raspbian OS installed)
- Door Lock Mechanism (e.g., solenoid lock, relay module)
- Jumper wires, Breadboard
- Internet access / Wi-Fi router

### Setup Instructions

1. **ESP8266 Setup**
   - Flash the provided Arduino sketch to your ESP8266 using Arduino IDE.
   - Configure Wi-Fi SSID and Password using the Portal.
   - Set the IP address of the Raspberry Pi server - make sure to use a static IP.

2. **Raspberry Pi Setup**
   - Install Python 3 and necessary libraries (`flask`, `requests`, etc.).
   - Run the server script on boot or as a service.
   - Set up local database (optional) to log access events.

3. **Connect Hardware**
   - Connect the ESP8266 to a relay controlling your door lock using the connection diagram given below.
   - Power the ESP8266, Relay, Led, via the same external adapter (or atleast have a common ground).
   - Power the MagnetLock with the lock rated power supply.
   
4. **Test the System**
   - Send unlock commands from Raspberry Pi.
   - Monitor door status from logs or dashboard (optional extension).

## 📂 Project Structure

```
/esp8266-code/         # Arduino sketches
/raspberry-pi-server/  # Python server scripts
README.md              # Project documentation
```

## 📸 Screenshots
WAIT


## 🙌 Acknowledgements

- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [Flask Web Framework](https://flask.palletsprojects.com/)
- [Raspberry Pi Foundation](https://www.raspberrypi.org/)
