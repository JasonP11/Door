import psycopg2
from flask import Flask, render_template, jsonify, redirect, request, url_for, session, flash
from flask_socketio import SocketIO
from encryption import encrypt_password, verify_password
import os
from dotenv import load_dotenv
from manage_log import log_routes
from db import get_db_connection
from threading import Thread
from log_monitor import monitor_logs
  

load_dotenv()  # Load environment variables

app = Flask(__name__)
app.secret_key = os.getenv('SECRET_KEY')

# Initialize Socket.IO
socketio = SocketIO(app)

# Register Blueprint
app.register_blueprint(log_routes)


# 🔹 Home Route
@app.route('/')
def home():
    return render_template('login.html')


# 🔹 Login Route
@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']

        conn = get_db_connection()
        if conn is None:
            return jsonify({"error": "Database connection failed"}), 500

        try:
            cur = conn.cursor()
            cur.execute("SELECT password FROM login WHERE username ILIKE %s", (f"%{username}%",)) 
            user = cur.fetchone()

            if user and verify_password(user[0], password):
                session['username'] = username

                # Fetch logs after login
                cur.execute('SELECT * FROM access_logs ORDER BY id DESC')
                logs = cur.fetchall()

                cur.close()
                conn.close()

                if username == 'admin':
                    return render_template('admin_logs.html', logs=logs)
                else:
                    return render_template('user2.html', logs=logs)
            else:
                
                return render_template('login.html', error="Invalid username or password")

        except Exception as e:
            return jsonify({"error": f"Error during login: {e}"}), 500

    return render_template('login.html')


# 🔹 Signup Route
@app.route('/signup', methods=['GET', 'POST'])
def signup():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        encrypted_password = encrypt_password(password)

        conn = get_db_connection()
        if conn is None:
            return jsonify({"error": "Database connection failed"}), 500

        cur = conn.cursor()
        try:
            cur.execute("INSERT INTO login (username, password) VALUES (%s, %s)", (username, encrypted_password))
            conn.commit()
        except psycopg2.IntegrityError:
            conn.rollback()
            return render_template('login.html', error="Username already exits!")

        finally:
            cur.close()
            conn.close()

        return redirect(url_for('login'))

    return render_template('signup.html')


# 🔹 Fetch Logs Route (Supports Date Filtering)
@app.route('/fetch_logss', methods=['GET'])
def fetch_logs():
    date = request.args.get('date')
    name = request.args.get('name')
    #global last_logs
    
    conn = get_db_connection()
    cursor = conn.cursor()
    try:
        if date and name:
            cursor.execute("SELECT id,door_no, code, name, role, status, timestamp FROM access_logs WHERE DATE(timestamp) = %s AND name ILIKE %s ORDER BY id DESC", (date, f"%{name}%"))
        elif date:
            cursor.execute("SELECT id, door_no,code, name, role, status, timestamp FROM access_logs WHERE DATE(timestamp) = %s ORDER BY id DESC", (date,))
        elif name:
            cursor.execute("SELECT id, door_no,code, name, role, status, timestamp FROM access_logs WHERE name ILIKE %s ORDER BY id DESC",  (f"%{name}%",))
        else:
            cursor.execute("SELECT id,door_no, code, name, role, status, timestamp FROM access_logs ORDER BY id DESC")
        logs = cursor.fetchall()
        logs_list = [
            {"id": log[0], "door_no": log[1], "code": log[2], "name": log[3], "role": log[4], "status": log[5], "timestamp": log[6]}
            for log in logs
        ]

        
        return jsonify(logs_list), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 500
    finally:
        cursor.close()
        conn.close()

"""
# 🔹 Add Log Entry & Emit Update Event
def add_log_entry(code, name, role, status):
    conn = get_db_connection()
    if conn is None:
        return False

    try:
        cur = conn.cursor()
        cur.execute(
            "INSERT INTO access_logs (code, name, role, status, timestamp) VALUES (%s, %s, %s, %s, NOW())",
            (code, name, role, status)
        )
        conn.commit()
        cur.close()
        conn.close()

        # Emit real-time update event
        socketio.emit('update_logs')
        return True
    except Exception as e:
        print("Error inserting log:", e)
        return False
"""

# 🔹 Logout Route
@app.route('/logout', methods=['GET'])
def logout():
    session.clear()
    return redirect(url_for('login'))


# 🔹 Run Flask App with Socket.IO
if __name__ == '__main__':
    Thread(target=monitor_logs,args=(socketio,), daemon=True).start()
    socketio.run(app, host='0.0.0.0', port=5001, debug=True, allow_unsafe_werkzeug=True)
    
