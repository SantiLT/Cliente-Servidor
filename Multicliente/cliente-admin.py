"""
Cliente admin: se conecta al servidor y manda el comando especial
de apagado, con contraseña.
"""


import socket

HOST = "127.0.0.1"
PORT = 65432

def main():
    password = input("Contraseña de admin: ")
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as admin_socket:
        
        admin_socket.connect((HOST,PORT))
        
        admin_socket.sendall(f"ADMIN:{password}".encode("utf-8"))
        respuesta = admin_socket.recv(1024).decode("utf-8")
        print(f"Servidor: {respuesta}")
        
        if respuesta.startswith("Contraseña incorrecta"):
            return
        
        print("Escribi mensajes normales, o 'shutdown' para apagar el servidor")
        
        while True:
            mensaje = input("> ")
            admin_socket.sendall(mensaje.encode("utf-8"))
            
            respuesta = admin_socket.recv(1024)
            
            if not respuesta:
                print("El servidor cerro la conexion.")
                break
            
            print(f"Servidor: {respuesta.decode("utf-8")}")
            
            if mensaje.strip().lower() in ("salir" , "shutdown"):
                break
            
    print("Conexion cerrada.")

if __name__ == "__main__":
    main()
