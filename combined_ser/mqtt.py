'''
import paho.mqtt.client as mqtt
import time

# MQTT Broker Configuration
MQTT_BROKER = "localhost"  # Change to your MQTT broker IP if needed
MQTT_PORT = 1883
MQTT_TOPIC = "test/topic"
MQTT_KEEPALIVE = 60  # Keep connection alive

# Callback when the client connects to the broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("✅ Connected to MQTT Broker!")
        client.subscribe(MQTT_TOPIC)
    else:
        print(f"❌ Failed to connect, return code {rc}")

# Callback when a message is received
def on_message(client, userdata, msg):
    print(f"📩 Message received: Topic={msg.topic}, Message={msg.payload.decode()}")

# Callback when the client disconnects
def on_disconnect(client, userdata, rc):
    print("🔴 Disconnected from MQTT Broker")
    while rc != 0:
        print("♻️ Reconnecting...")
        try:
            client.reconnect()
            print("✅ Reconnected!")
            break
        except:
            time.sleep(5)

# Create MQTT Client
mqtt_client = mqtt.Client()

# Attach callbacks
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.on_disconnect = on_disconnect

# Connect to MQTT Broker
try:
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, MQTT_KEEPALIVE)
except Exception as e:
    print(f"❌ Connection failed: {e}")
    exit(1)

# Start the MQTT loop to maintain connection
mqtt_client.loop_start()

# Keep script running
try:
    while True:
        time.sleep(1)  # Keeps the script alive
except KeyboardInterrupt:
    print("🛑 Exiting...")
    mqtt_client.loop_stop()
    mqtt_client.disconnect()

'''

from flask import Flask, request, jsonify
import paho.mqtt.client as mqtt
import psycopg2
import json
from datetime import datetime

app = Flask(__name__)

# Database Configuration
db_params = {
    "dbname": "work",
    "user": "postgres",
    "password": "123",
    "host": "localhost",
    "port": "5432"
}

# MQTT Configuration
MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_TOPIC = "nfc/access"  # Fixed topic
RELAY_TOPIC = "bypass_relay"

client = mqtt.Client()
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Connect to PostgreSQL
def db_connect():
    return psycopg2.connect(**db_params)

def log_access(uid, name, role, status):
    conn = db_connect()
    cur = conn.cursor()
    cur.execute(
        "INSERT INTO access_logs (code, name, role, status, timestamp) VALUES (%s, %s, %s, %s, %s)",
        (uid, name, role, status, datetime.now())
    )
    conn.commit()
    cur.close()
    conn.close()

def check_access(uid):
    conn = db_connect()
    cur = conn.cursor()
    cur.execute("SELECT name, level FROM emp WHERE key_id = %s", (uid,))
    result = cur.fetchone()
    conn.close()
    return result

def on_message(client, userdata, msg):
    payload = msg.payload.decode("utf-8")
    print(f"Received: {payload}")  # Debugging

    try:
        data = json.loads(payload)  # FIXED
        uid = data.get("uid")
        door = data.get("door")

        access_info = check_access(uid)
        if access_info:
            name, level = access_info
            log_access(uid, name, level, "GRANTED")
            client.publish(RELAY_TOPIC, "open")
        else:
            log_access(uid, "Unknown", "None", "DENIED")
    except json.JSONDecodeError:
        print("Invalid JSON received")

client.on_message = on_message
client.subscribe(MQTT_TOPIC)  # FIXED
client.loop_start()

@app.route("/bypass", methods=["POST"])
def bypass():
    client.publish(RELAY_TOPIC, "open")
    return jsonify({"message": "Relay activated"}), 200

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
