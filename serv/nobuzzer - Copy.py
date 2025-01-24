from flask import Flask, request, jsonify
import psycopg2
import os
from datetime import datetime
import requests

app = Flask(__name__)

# PostgreSQL connection setup
DATABASE_URL = os.environ.get('DATABASE_URL', 'postgresql://postgres:123@localhost:5432/work')

# URLs for relays
RELAY_ESP_URL_BYPASS = 'http://192.168.0.188/relay'  # Replace with bypass relay IP
RELAY_ESP_URL_DOOR_1 = 'http://192.168.0.188/relay'  # Replace with Door 1 relay ESP8266 IP
RELAY_ESP_URL_DOOR_2 = 'http://192.168.0.192/relay'  # Replace with Door 2 relay ESP8266 IP

# Connect to PostgreSQL
def get_db_connection():
    conn = psycopg2.connect(DATABASE_URL)
    return conn

def reply():
    return jsonify({"message": "Access Granted", "uid": uid, "type": reader_type, "door": door}), 200

# Endpoint to receive NFC UID and compare with the database
@app.route('/data', methods=['POST'])
def handle_nfc_data():

        #data = request.get_json()
        uid = request.form.get('uid')
        reader_type = request.form.get('type')
        door = request.form.get('door')
        
        # Initialize variables for user information and access status
        user_name = ""
        user_role = ""
        access_status = "Denied"  # Default status

        # Check if UID, reader_type, or door is missing
        if not uid or not reader_type or not door:
            return jsonify({"error": "Missing UID, Reader Type, or Door"}), 400

        # Query the database to check if the UID exists in the emp table
        conn = get_db_connection()
        cursor = conn.cursor()

        query = "SELECT name, level FROM emp WHERE key_id = %s"
        cursor.execute(query, (uid,))
        result = cursor.fetchone()

        if result:
            access_status = "Granted"
            user_name = result[0]  # Use the name from emp table if UID exists
            user_role = f"{result[1]}"
            reply
        # Log the access attempt in the access_logs table
        log_query = """
            INSERT INTO access_logs (code, name, role, status, timestamp)
            VALUES (%s, %s, %s, %s, %s)
        """
        cursor.execute(log_query, (uid, user_name, user_role, access_status, datetime.now()))

        conn.commit()
        cursor.close()
        conn.close()

        app.logger.info(access_status)
        # If access is granted, send a JSON request to activate the appropriate relay
        if access_status == "Granted":
            '''try:
                headers = {'Content-Type': 'application/json'}
                
                # Determine which relay to activate based on the door
                if door == '1':
                    relay_url = RELAY_ESP_URL_DOOR_1
                elif door == '2':
                    relay_url = RELAY_ESP_URL_DOOR_2
                else:
                    return jsonify({"error": "Invalid Door ID"}), 400
'''

                # Send data as plain JSON
                #headers = {'Content-Type': 'application/json'}
                #response = requests.post(relay_url, data='{"command":"open"}', headers=headers)
                #print(response.status_code)
                #if response.status_code == 200:
                #    return jsonify({"message": "Relay Activated"}), 200
                #else:
                #    print("Error in relay response:", response.status_code, response.text)
                #    return jsonify({"error": "Failed to activate relay", "status": response.status_code}), 500
            #except Exception as e:
            #        print("Exception:", e)
            #        return jsonify({"error": "Exception occurred", "details": str(e)}), 500


            return jsonify({"message": "Access Granted", "uid": uid, "type": reader_type, "door": door}), 200
        else:
            return jsonify({"message": "Access Denied", "uid": uid, "type": reader_type, "door": door}), 403



@app.route('/bypass', methods=['POST'])
def bypass_relay():
    try:
        # Send data as plain JSON
        headers = {'Content-Type': 'application/json'}
        response = requests.post(RELAY_ESP_URL_BYPASS, data='{"command":"open"}', headers=headers)
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