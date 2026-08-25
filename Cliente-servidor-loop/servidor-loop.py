"""
Servidor de sockets TCP con loop de mensajes.
Se queda escuchando en un puerto, acepta una conexión,
y mantiene una conversación (recibe y responde varias veces)
hasta que el cliente se desconecta o manda "salir".
"""


import socket

HOST = "127.0.0.1"  # localhost: solo tu propia máquina
PORT = 65432        # puerto arbitrario (por encima de 1024)

# AF_INET = usamos IPv4
# SOCK_STREAM = usamos TCP (conexión confiable, ordenada)
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:

    # Evita el error "address already in use" si reiniciás rápido el server
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    server_socket.bind((HOST, PORT))
    server_socket.listen()
    print(f"Servidor escuchando en {HOST}:{PORT}...")

    # accept() bloquea el programa hasta que llega una conexión
    conn, addr = server_socket.accept()
    with conn:
        print(f"Conectado por {addr}")
        
        while True:
            data = conn.recv(1024)
            
            if  not data:
                print("El cliente cerró la conexion")
                break
            
            mensaje = data.decode('utf-8')
            
            print(f"Mensaje recibido: {mensaje}")
            
            if mensaje.strip().lower() == "salir":
                print("El cliente pidio salir.")
                conn.sendall("Chau!".encode('utf-8'))
                break
            
            respuesta = f"Servidor recibió: {mensaje}"
            conn.sendall(respuesta.encode('utf-8'))
            
    print ("Conexion cerrada.")    
            
