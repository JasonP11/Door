from flask import Flask, request, jsonify
import psycopg2
import os
import threading
from datetime import datetime
import requests

app = Flask(__name__)

# PostgreSQL connection setup
DATABASE_URL = os.environ.get('DATABASE_URL', 'postgresql://postgres:123@localhost:5432/work')

# URLs for relays
RELAY_ESP_URL_BYPASS = 'http://192.168.80.48/relay'  # Replace with bypass relay IP
#RELAY_ESP_URL_DOOR_1 = 'http://192.168.80.88/relay'  # Replace with Door 1 relay ESP8266 IP
#RELAY_ESP_URL_DOOR_2 = 'http://192.168.0.192/relay'  # Replace with Door 2 relay ESP8266 IP

# Connect to PostgreSQL
def get_db_connection():
    conn = psycopg2.connect(DATABASE_URL)
    return conn

# Function to activate the relay in a separate thread
def activate_relay(relay_url):
    try:
        headers = {'Content-Type': 'application/json'}
        response = requests.post(relay_url, data='{"command":"open"}', headers=headers)
        
        if response.status_code != 200:
            print(f"Error in relay response: {response.status_code}, {response.text}")
    except Exception as e:
        print(f"Exception occurred while activating relay: {e}")


@app.route('/data', methods=['POST'])
def handle_nfc_data():
    uid = request.form.get('uid')
    reader_type = request.form.get('type')
    door = request.form.get('door')

    # Check if UID, reader_type, or door is missing
    if not uid or not reader_type or not door:
        return jsonify({"error": "Missing UID, Reader Type, or Door"}), 400

    # Initialize variables for user information and access status
    user_name = ""
    user_role = ""
    access_status = "Denied"  # Default status

    # Query the database to check if the UID exists in the emp table
    conn = get_db_connection()
    cursor = conn.cursor()

    query = "SELECT name, level FROM emp WHERE key_id = %s"
    cursor.execute(query, (uid,))
    result = cursor.fetchone()

    if result:
        access_status = "Granted"
        user_name = result[0]
        user_role = f"{result[1]}"

    # Log the access attempt in the access_logs table
    log_query = """
        INSERT INTO access_logs (code, name, role, status, timestamp)
        VALUES (%s, %s, %s, %s, %s)
    """
    cursor.execute(log_query, (uid, user_name, user_role, access_status, datetime.now()))

    conn.commit()
    cursor.close()
    conn.close()

    if access_status == "Granted":
        app.logger.info(access_status)
        return jsonify({"message": "Access Granted", "uid": uid, "type": reader_type, "door": door}), 200
    else:
        app.logger.info(access_status)
        return jsonify({"message": "Access Denied", "uid": uid, "type": reader_type, "door": door}), 403





@app.route('/')
def fetch_logs():
    conn = get_db_connection()
    try:
        cur = conn.cursor()
        cur.execute('SELECT * FROM access_logs ORDER BY id DESC')
        logs = cur.fetchall()
        cur.close()
        return jsonify(logs)
    finally:
        if conn:
            conn.close()



@app.route('/bypass', methods=['POST'])
def bypass_relay():
    try:
        # Send data as plain JSON
        headers = {'Content-Type': 'application/json'}
        response = requests.post(
        RELAY_ESP_URL_BYPASS,
        data={'plain': '{"command":"open"}'},
        headers={'Content-Type': 'application/x-www-form-urlencoded'},
        timeout=3
        )

        if response.status_code == 200:
            return jsonify({"message": "Relay Activated"}), 200
        else:
            print("Error in relay response:", response.status_code, response.text)
            return jsonify({"error": "Failed to activate relay", "status": response.status_code}), 500
    except Exception as e:
        print("Exception:", e)
        return jsonify({"error": "Exception occurred", "details": str(e)}), 500


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)