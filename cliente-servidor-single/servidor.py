"""
Servidor básico de sockets TCP.
Se queda escuchando en un puerto, acepta una conexión,
recibe un mensaje y responde.
"""


import socket

HOST = "127.0.0.1"  # localhost
PORT = 65432        # puerto arbitrario

# AF_INET = IPv4
# SOCK_STREAM = TCP
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:

    # Evita el error "address already in use" si reiniciás rápido el server
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    server_socket.bind((HOST, PORT))
    server_socket.listen()
    print(f"Servidor escuchando en {HOST}:{PORT}...")

    # accept() bloquea el programa hasta que llega una conexión
    conn, addr = server_socket.accept()
    with conn:
        
        # recv(1024) espera datos, hasta 1024 bytes por vez
        data = conn.recv(1024)
        mensaje = data.decode("utf-8")
        print(f"Mensaje recibido: {mensaje}")

        # Responde al cliente
        respuesta = f"Servidor recibió: '{mensaje}'"
        conn.sendall(respuesta.encode("utf-8"))

    print("Conexión cerrada.")