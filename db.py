import psycopg2


# Database connection
def get_db_connection():
    try:
        conn = psycopg2.connect(
            host="localhost",
            database="",
            user="",  # Replace with your username
            password=""  # Replace with your password
        )
        
        return conn
    except Exception as e:
        print("Error connecting to the database:", e)
        return None
