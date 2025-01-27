import psycopg2


# Database connection
def get_db_connection():
    try:
        conn = psycopg2.connect(
            host="localhost",
            database="work",
            user="postgres",  # Replace with your username
            password="123"  # Replace with your password
        )
        print("Database connection established")  # Debugging line
        return conn
    except Exception as e:
        print("Error connecting to the database:", e)
        return None