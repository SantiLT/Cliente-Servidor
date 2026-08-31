"""
Cliente básico de sockets TCP.
Se conecta al servidor, manda un mensaje y espera la respuesta.
"""


import socket

HOST = "127.0.0.1"  # tiene que coincidir con el HOST del servidor
PORT = 65432        # tiene que coincidir con el PORT del servidor

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:

    # connect() intenta conectarse al servidor (falla si no está escuchando)
    client_socket.connect((HOST, PORT))
    print ("Conectado al servidor.")
    
    mensaje = "Hola servidor, soy el cliente!"
    client_socket.sendall(mensaje.encode("utf-8"))

    # Espera la respuesta del servidor
    data = client_socket.recv(1024)
    print(f"Respuesta del servidor: {data.decode('utf-8')}")
    
print("Conexion cerrada.")
