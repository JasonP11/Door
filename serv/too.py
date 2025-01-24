from flask import Flask, request, jsonify
import psycopg2
import os
from datetime import datetime
import requests

app = Flask(__name__)

# PostgreSQL connection setup
DATABASE_URL = os.environ.get('DATABASE_URL', 'postgresql://Jason Pereira:jason@localhost:5432/work')

RELAY_ESP_URL = 'http://192.168.0.193/relay' 

# Connect to PostgreSQL
def get_db_connection():
    conn = psycopg2.connect(DATABASE_URL)
    return conn

# Endpoint to receive NFC UID and compare with the database
@app.route('/data', methods=['POST'])
def handle_nfc_data():
    uid = request.form.get('uid')
    reader_type = request.form.get('type')

    # For demonstration, you could replace these with actual user data
    user_name = ""    # Example name, replace with actual
    user_role = ""    # Example role, replace with actual
    access_status = "Denied"  # Default status

    # Check if UID or reader_type is missing
    if not uid or not reader_type:
        return jsonify({"error": "Missing UID or Reader Type"}), 400

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

    # Log the access attempt in the access_logs table
    log_query = """
        INSERT INTO access_logs (code, name, role, status, timestamp)
        VALUES (%s, %s, %s, %s, %s)
    """
    cursor.execute(log_query, (uid, user_name, user_role, access_status, datetime.now()))

    #print(log_query)
    conn.commit()
    cursor.close()
    conn.close()

    if access_status == "Granted":
        return jsonify({"message": "Access Granted", "uid": uid, "type": reader_type}), 200
    else:
        return jsonify({"message": "Access Denied", "uid": uid, "type": reader_type}), 403
    

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
