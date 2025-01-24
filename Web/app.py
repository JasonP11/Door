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
    return render_template('home.html')

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
@app.route('/fetch_logs')
def fetch_logs():
    conn = get_db_connection()
    if conn is None:
        abort(500, "Error connecting to the database")

    try:
        cur = conn.cursor()
        cur.execute('SELECT * FROM access_logs ORDER BY id DESC')
        logs = cur.fetchall()
        cur.close()
        return jsonify(logs)
    finally:
        if conn:
            conn.close()
# Logout route
@app.route('/logout', methods=['GET'])
def logout():
    # Clear the user session
    session.clear()
    return redirect(url_for('home'))

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5001, debug=True)
