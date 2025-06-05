from flask import Flask, request, jsonify
import psycopg2
import os
import threading
from datetime import datetime
import requests
import sys
import socket
import hashlib



app = Flask(__name__)

# PostgreSQL connection setup
DATABASE_URL = os.environ.get('DATABASE_URL', 'postgresql://') #<-- insert your database

passw = "blank"

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
    access_door = f"{reader_type} - Denied" 
    
    uid = hashlib.sha256(uid.encode()).hexdigest()

    # Query the database to check if the UID exists in the emp table
    conn = get_db_connection()
    cursor = conn.cursor()

    query = "SELECT name, level FROM access_users WHERE key_id = %s"
    cursor.execute(query, (uid,))
    result = cursor.fetchone()

    if result:
        access_status = "Granted"
        user_name = result[0]
        user_role = f"{result[1]}"
        access_door = f"{reader_type} - Granted" 

    
    # Log the access attempt in the access_logs table
    log_query = """
        INSERT INTO access_logs (door_no,code, name, role, status, timestamp)
        VALUES (%s,%s, %s, %s, %s, %s)
    """
    cursor.execute(log_query, (door,uid, user_name, user_role, access_door, datetime.now()))

    conn.commit()
    cursor.close()
    conn.close()

    if access_status == "Granted":
        app.logger.info(access_status)
        return jsonify({"message": "Access Granted", "uid": uid, "type": reader_type, "door": door}), 200
    else:
        app.logger.info(access_status)
        return jsonify({"message": "Access Denied", "uid": uid, "type": reader_type, "door": door}), 403



@app.route('/setup', methods=['POST'])
def IP():
    #relay_ip
    relay_ip = request.form.get("relay_ip")
    request_type = request.form.get("type")  # Get type (setup or normal)
    door_no = request.form.get("door")
    door_mac = request.form.get("MacAdd")
    if request_type == "setup":
        print(f"🔧 Setup Request Received! Relay IP: {relay_ip}, Door No: {door_no}")
    else:
        print(f"📡 Normal Data Received. Relay IP: {relay_ip}, Door No: {door_no}")
    conn = get_db_connection()
    cursor = conn.cursor()
    log_query = """
    INSERT INTO door (door_no, door_ip, door_mac)
    VALUES (%s, %s, %s)
    ON CONFLICT (door_no) 
    DO UPDATE SET door_ip = EXCLUDED.door_ip, 
                  door_mac = EXCLUDED.door_mac              
    """
    cursor.execute(log_query, (door_no, relay_ip, door_mac))

    conn.commit()
    cursor.close()
    conn.close()

    return "Data received", 200


@app.route('/bypass', methods=['POST'])
def bypass_relay():
    try:
        conn = get_db_connection()
        cursor = conn.cursor()


        data = request.get_json()
        door_id = data.get('door_id')
        password = data.get('password')


        if not door_id or not password:
            return jsonify({"error": "Missing door_id or password"}), 400
        
        query = "SELECT door_ip FROM door WHERE door_no = %s;"
        cursor.execute(query, (door_id,))
        IP = cursor.fetchone()[0] 

        url = f"http://{IP}/relay"
        
        if(password == passw):
            response = requests.post(
            url,  # Ensure it's a proper URL
            data={'plain': '{"command":"open"}'},
            headers={'Content-Type': 'application/x-www-form-urlencoded'},
            timeout=5
            )
        
            if response.status_code == 200:
                    #  Insert log into `access_logs` with only status
                    insert_query = """
                    INSERT INTO access_logs (door_id,code, name, role, status, timestamp)
                    VALUES (door_no, NULL, NULL, NULL, 'Bypass', NOW());
                    """
                    cursor.execute(insert_query)
                    conn.commit()

                    return jsonify({"message": "Relay Activated"}), 200
            else:
                    return jsonify({"error": "Failed to activate relay", "status": response.status_code}), 500
        else:
            return jsonify({"error": "Invalid password"}), 403

    except Exception as e:
        print("Exception:", e)
        return jsonify({"error": "Exception occurred", "details": str(e)}), 500


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)