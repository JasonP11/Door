import os
import psycopg2

# Get the database URL from environment variables or use the default
DATABASE_URL = os.environ.get('DATABASE_URL', 'postgresql://postgres:smith@192.168.0.192:5432/access_logs_db')

# Connect to the database
try:
    connection = psycopg2.connect(DATABASE_URL)
    print("Connected to the database successfully.")
except Exception as e:
    print("Failed to connect to the database.")
    print(e)
