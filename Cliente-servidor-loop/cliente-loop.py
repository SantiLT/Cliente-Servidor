"""
Cliente de sockets TCP con loop de mensajes.
Se conecta al servidor y permite mandar varios mensajes seguidos
desde la terminal, hasta escribir "salir".
"""


import socket

HOST = "127.0.0.1"  # tiene que coincidir con el HOST del servidor
PORT = 65432        # tiene que coincidir con el PORT del servidor

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:

    # connect() intenta conectarse al servidor (falla si no está escuchando)
    client_socket.connect((HOST, PORT))
    print ("Conectado al servidor. Escribi mensajes (o salir para terminar).")


    while True:
        mensaje = input("> ")
        client_socket.sendall(mensaje.encode('utf-8'))
        
        if mensaje.strip().lower() == "salir":
            data = client_socket.recv(1024)
            
            print(f"Servidor: {data.decode('utf-8')}")
            break
        
        data = client_socket.recv(1024)
        print(f"Servidor: {data.decode('utf-8')}")
               
print("Conexion cerrada.")
        
