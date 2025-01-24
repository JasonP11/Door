import psycopg2
from flask import Flask,Blueprint, render_template, jsonify, redirect, request, url_for, session, flash
from db import get_db_connection

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
        cursor.execute('INSERT INTO logs (uid, name, role) VALUES (%s, %s, %s)',
                       (data['uid'], data['name'], data.get('role', None)))
        conn.commit()
        return jsonify({'uid': data['uid'], 'name': data['name'], 'role': data.get('role', None)}), 201
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
        cursor.execute('SELECT * FROM logs')
        logs = cursor.fetchall()
        return jsonify([{'name': log[0], 'uid': log[1], 'role': log[2]} for log in logs]), 200
    except Exception as e:
        print("Error fetching logs:", e)
        return jsonify({'error': str(e)}), 500
    finally:
        cursor.close()
        conn.close()

# Route to update a log
@log_routes.route('/logs/<uid>', methods=['PUT'])
def update_log(uid):
    data = request.get_json()
    name = data.get('name')
    role = data.get('role')

    conn = get_db_connection()
    cursor = conn.cursor()

    try:
        cursor.execute(
            'UPDATE logs SET name = %s, role = %s WHERE uid = %s',
            (name, role, uid)
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

@log_routes.route('/logs/<uid>', methods=['DELETE'])
def delete_log(uid):
    print(f"Attempting to delete log with UID: {uid}")  # Debugging line
    conn = get_db_connection()
    cursor = conn.cursor()
    
    try:
        cursor.execute('DELETE FROM logs WHERE uid = %s', (uid,))
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




