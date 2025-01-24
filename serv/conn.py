from flask import Flask, request, jsonify
import psycopg2
import os
from datetime import datetime

app = Flask(__name__)

# PostgreSQL connection setup
DATABASE_URL = os.environ.get('DATABASE_URL', 'postgresql://Jason Pereira:jason@localhost:5432/work')

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
        user_role = f"Level {result[1]}"

    # Log the access attempt in the access_logs table
    log_query = """
        INSERT INTO access_logs (code, name, role, status, timestamp)
        VALUES (%s, %s, %s, %s, %s)
    """
    cursor.execute(log_query, (uid, user_name, user_role, access_status, datetime.now()))

    print(log_query)
    conn.commit()
    cursor.close()
    conn.close()

    # Prepare response with Keep-Alive header
    response = jsonify({
        "message": access_status,
        "uid": uid,
        "type": reader_type
    })
    response.headers['Connection'] = 'keep-alive'
    
    return response, 200 if access_status == "Granted" else 403

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
