import socket

# Create a UDP socket
udp_server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

# Bind to listen for broadcast messages on port 4210
udp_server.bind(("", 6000))
print("Listening for ESP requests...")

while True:
    # Receive data from ESP
    data, addr = udp_server.recvfrom(1024)
    message = data.decode("utf-8")

    if message == "DISCOVER_PI":
        pi_ip = socket.gethostbyname(socket.gethostname())
        print(f"Received request from {addr}, sending Pi IP: {pi_ip}")
        udp_server.sendto(pi_ip.encode(), addr)
