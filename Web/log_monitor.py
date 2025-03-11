import time
from flask_socketio import SocketIO
from db import get_db_connection



# Initialize Socket.IO without Redis
socketio = SocketIO()

def monitor_logs(socketio,date=None):
    
    last_log_id = None  
    while True:
        conn = get_db_connection()
        if conn is None:
            time.sleep(5)
            continue

        try:
            cur = conn.cursor()
            if date:
                cur.execute("SELECT id FROM access_logs WHERE DATE(timestamp) = %s ORDER BY id DESC", (date,))
            else:
                cur.execute("SELECT id FROM access_logs ORDER BY id DESC")
            latest_log = cur.fetchone()

            if latest_log and (last_log_id is None or latest_log[0] > last_log_id):
                last_log_id = latest_log[0]
                socketio.emit('update_logs')  # Notify clients to fetch new logs

            cur.close()
            conn.close()
        except Exception as e:
            print("Error monitoring logs:", e)

        time.sleep(3)  # Check for new logs every 3 seconds
