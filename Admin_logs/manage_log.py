import psycopg2
from flask import Flask,Blueprint, render_template, jsonify, redirect, request, url_for, session, flash
from db import get_db_connection
from datetime import datetime

log_routes = Blueprint('log_routes', __name__)
# Database connection

# Route to serve the admin page

# Route to add a new log
@log_routes.route('/logs', methods=['POST'])
def add_log():
    data = request.json
    conn = get_db_connection() 
    cursor = conn.cursor()
    try:
        cursor.execute('INSERT INTO access_users (key_id, name, level) VALUES (%s, %s, %s)',
                       (data['key_id'], data['name'], data.get('level', None)))
        conn.commit()
        return jsonify({'uid': data['key_id'], 'name': data['name'], 'role': data.get('level', None)}), 201
    except Exception as e:
        print("Error adding log:", e)
        return jsonify({'error': str(e)}), 400
    finally:
        cursor.close()
        conn.close()

# Route to get all logs
@log_routes.route('/logs', methods=['GET'])
def get_logs():
    conn = get_db_connection()
    cursor = conn.cursor()
    try:
        cursor.execute('SELECT * FROM access_users')
        logs = cursor.fetchall()
        return jsonify([{'id': log[0], 'name': log[1], 'key_id': log[2], 'level': log[3]} for log in logs]), 200
    except Exception as e:
        print("Error fetching logs:", e)
        return jsonify({'error': str(e)}), 500
    finally:
        cursor.close()
        conn.close()
# Route to update a log
@log_routes.route('/logs/<key_id>', methods=['PUT'])
def update_log(key_id):
    data = request.get_json()
    name = data.get('name')
    level = data.get('level')

    conn = get_db_connection()
    cursor = conn.cursor()

    try:
        cursor.execute(
            'UPDATE access_users SET name = %s, level = %s WHERE key_id = %s',
            (name, level, key_id)
        )
        conn.commit()

        if cursor.rowcount == 0:
            return jsonify({'error': 'Log not found'}), 404

        return jsonify({'message': 'Log updated successfully'}), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 400
    finally:
        cursor.close()
        conn.close()

@log_routes.route('/logs/<key_id>', methods=['DELETE'])
def delete_log(key_id):
    print(f"Attempting to delete log with UID: {key_id}")  # Debugging line
    conn = get_db_connection()
    cursor = conn.cursor()
    
    try:
        cursor.execute('DELETE FROM access_users WHERE key_id = %s', (key_id,))
        conn.commit()
        
        if cursor.rowcount == 0:
            print("No log found with the specified UID.")  # Debugging line
            return jsonify({'error': 'Log not found'}), 404
        
        print("Log deleted successfully.")  # Debugging line
        return jsonify({'message': 'Log deleted'}), 204
    except Exception as e:
        print("Error deleting log:", e)  # Debugging line
        return jsonify({'error': str(e)}), 400
    finally:
        cursor.close()
        conn.close()




