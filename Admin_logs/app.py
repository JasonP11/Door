import psycopg2
from flask import Flask, render_template, jsonify, redirect, request, url_for, session, flash, abort
from flask_socketio import SocketIO
from encryption import encrypt_password, verify_password 
import os
from dotenv import load_dotenv
from manage_log import log_routes
from db import get_db_connection
load_dotenv()  # Load variables from .env file

app = Flask(__name__)
app.secret_key = os.getenv('SECRET_KEY')
socketio = SocketIO(app)
app.register_blueprint(log_routes)
  
# Database connection


# Home route
@app.route('/')
def home():
    return render_template('login.html')

# Modified Login route to fetch logs after successful login
@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        # Get the username and password from the form
        username = request.form['username']
        password = request.form['password']
        
        # Get the database connection
        conn = get_db_connection()
        if conn is None:
            return jsonify({"error": "Error connecting to the database"}), 500

        try:
            # Create a cursor to execute the query
            cur = conn.cursor()
            cur.execute("SELECT password FROM login_info_table WHERE username = %s", (username,))
            user = cur.fetchone()

            # Check if the user exists and verify the password
            if user and verify_password(user[0], password):
                # Set session variable for the logged-in user
                session['username'] = username
                
                # Fetch logs from the database
                cur.execute('SELECT * FROM access_logs ORDER BY id DESC')
                logs = cur.fetchall()

                cur.close()
                conn.close()
                if session.get('username') == 'admin':
                    return render_template('admin_logs.html', logs=logs)  # Render admin logs page
                else:
                    return render_template('user_logs.html', logs=logs)  # Render user logs page    
                # Render the logs.html page with the fetched logs
                
            else:
                # Flash an error message and render the login template with the error
                flash("Invalid username or password", 'error')
                return render_template('login.html')

        except Exception as e:
            return jsonify({"error": f"Error during login process: {e}"}), 500

    # If the request method is GET, just render the login page
    return render_template('login.html')

# Sign Up route
@app.route('/signup', methods=['GET', 'POST'])
def signup():
    if request.method == 'POST':
        # Handle sign up logic here (e.g., create new user)
        username = request.form['username']
        password = request.form['password']
        encrypted_password = encrypt_password(password)

        # Save the username and encrypted password to the database
        conn = get_db_connection()
        if conn is None:
            return jsonify({"error": "Error connecting to the database"}), 500

        cur = conn.cursor()
        try:
            cur.execute("INSERT INTO login_info_table (username, password) VALUES (%s, %s)", (username, encrypted_password))
            conn.commit()
        except psycopg2.IntegrityError:
            conn.rollback()
            return jsonify({"error": "Username already exists"}), 400
        finally:
            cur.close()
            conn.close()
        # Redirect to login after sign up
        return redirect(url_for('login'))

    return render_template('signup.html')
@app.route('/fetch_logs', methods=['GET'])
def fetch_logs():
    date = request.args.get('date')
    conn = get_db_connection()
    cursor = conn.cursor()
    try:
        cursor.execute('SELECT * FROM access_logs ORDER BY id DESC')
        if date:
            cursor.execute("SELECT id, code, name, role, status, timestamp FROM access_logs WHERE DATE(timestamp) = %s", (date,))
        else:
            cursor.execute("SELECT id, code, name, role, status, timestamp FROM access_logs")
        
        logs = cursor.fetchall()
        logs_list = [
            {"id": log[0], "code": log[1], "name": log[2], "role": log[3], "status": log[4], "timestamp": log[5]}
            for log in logs
        ]
        return jsonify(logs_list), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 500
    finally:
        cursor.close()
        conn.close()
# Logout route
@app.route('/logout', methods=['GET'])
def logout():
    # Clear the user session
    session.clear()
    return redirect(url_for('login'))

@socketio.on('new_log')
def handle_new_log():
    socketio.emit('update_logs')

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5001, debug=True,allow_unsafe_werkzeug=True)
