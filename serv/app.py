from flask import Flask, request, jsonify
import psycopg2
import os

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

    print(uid)
#    if not uid or not reader_type:
#        return jsonify({"error": "Missing UID or Reader Type"}), 400

    # Query the database to check if the UID exists in the emp table
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

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
