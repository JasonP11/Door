from flask import Flask, request, jsonify
import psycopg2
import os
import requests  # For server to relay ESP communication

app = Flask(__name__)

DATABASE_URL = os.environ.get('DATABASE_URL', 'postgresql://Jason Pereira:jason@localhost:5432/work')

RELAY_ESP_URL = 'http://192.168.0.188/relay'  # Replace <relay_esp_ip> with the relay ESP8266 IP

# Connect to PostgreSQL
def get_db_connection():
    conn = psycopg2.connect(DATABASE_URL)
    return conn

@app.route('/data', methods=['POST'])
def handle_nfc_data():
    uid = request.form.get('uid')
    reader_type = request.form.get('type')

    conn = get_db_connection()
    cursor = conn.cursor()
    query = "SELECT EXISTS(SELECT 1 FROM emp WHERE key_id = %s)"
    cursor.execute(query, (uid,))
    result = cursor.fetchone()
    cursor.close()
    conn.close()

    if result and result[0]:
        return jsonify({"message": "Access Granted", "uid": uid, "type": reader_type}), 200
    else:
        return jsonify({"message": "Access Denied", "uid": uid, "type": reader_type}), 403

# Endpoint to bypass and open the relay
@app.route('/bypass', methods=['POST'])
def bypass_relay():
    try:
        # Send data as plain JSON
        headers = {'Content-Type': 'application/json'}
        response = requests.post(RELAY_ESP_URL, data='{"command":"open"}', headers=headers)
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
