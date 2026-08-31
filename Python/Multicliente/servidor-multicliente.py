

"""
Servidor de sockets TCP que atiende múltiples clientes a la vez
usando un thread por cada conexión, y acepta un comando especial
de administrador para apagarse remotamente.
 
Al apagarse, avisa a todos los clientes conectados antes de cerrar
el proceso (los threads son daemon, así que mueren apenas main()
termina, pero primero les mandamos un mensaje de aviso).
"""

import threading
import socket
import os

HOST = "127.0.0.1"  
PORT = 65432

ADMIN_PASSWORD = os.environ.get("ADMIN_PASSWORD", "admin123")

shutdown_event = threading.Event()

clientes_activos = []
clientes_lock = threading.Lock()



def atender_cliente(conn, addr, server_socket):
    
    """
    Corre en un thread por cada cliente. El PRIMER mensaje siempre se
    interpreta como intento de autenticación: "ADMIN:<contraseña>" para
    loguearse como admin, o cualquier otra cosa para ser un cliente normal.
 
    Un admin autenticado puede después mandar mensajes de chat como
    cualquier cliente, O el comando "shutdown" en cualquier momento
    para apagar el servidor.
    """

    print(f"[NUEVA CONEXION] {addr} conectado")
    es_admin = False
    
    with clientes_lock:
        clientes_activos.append((conn,addr))
        
    try:
    
        with conn:
            #--- Primer mensaje : intento de autenticacion ---
            
            data = conn.recv(1024)
            if not data:
                print (f"[DESCONECTADO] {addr} cerro la conexion antes de identificarse.")
                return
            
            primer_mensaje = data.decode("utf-8").strip()
            partes = primer_mensaje.split(":")
            
            if len(partes) == 2 and partes[0] == "ADMIN":
                #print(f"DEBUG: recibido={repr(partes[1])} | esperado={repr(ADMIN_PASSWORD)}")
                if partes[1] == ADMIN_PASSWORD:
                    es_admin = True
                    print(f"[ADMIN] {addr} se autentico correctamente.")
                    conn.sendall("Ok, autenticado como admin. podes chatear o mandar shutdown".encode("utf-8"))
                
                else:
                    print(f"[ADMIN] {addr} intento autenticarse con contraseña incorrecta.")
                    conn.sendall("Contraseña incorrecta.".encode("utf-8"))
                    return
            
            else:
                print(f"[{addr}] dice: {primer_mensaje}")
                if primer_mensaje.lower()== "salir":
                    conn.sendall("Chau!".encode("utf-8"))
                    print(f"[DESCONECTADO] {addr} pidio salir.")
                    return
                
                respuesta = f"Servidor recibio: '{primer_mensaje}'"
                conn.sendall(respuesta.encode("UTF-8"))
        
            #--- Loop normal -----
            
            while True:
                    data = conn.recv(1024)
                    
                    if  not data:
                        print(f"[DESCONECTADO] {addr} cerró la conexion")
                        break
                    
                    mensaje = data.decode('utf-8').strip()
                    
                    if es_admin and mensaje.lower() == "shutdown":
                        print(f"[ADMIN] {addr} pidio apagar el servidor")
                        conn.sendall("OK, apagando servidor...".encode("UTF-8"))
                        shutdown_event.set()
                        break
                    
                    print(f"[{addr}] dice: {mensaje}")
                    
                    if mensaje.lower() == "salir":
                        conn.sendall("Chau!".encode("utf-8"))
                        print(f"[DESCONECTADO] {addr} pidio salir.")
                        break
                    
                    respuesta = f"Servidor recibio: '{mensaje}'"
                    conn.sendall(respuesta.encode("utf-8"))
    finally:
        
        with clientes_lock:
            if(conn,addr) in clientes_activos:
                clientes_activos.remove((conn,addr))
                

def avisar_y_cerrar():
    
    with clientes_lock:
        copia = list(clientes_activos)
        
    for conn,add in copia:
        try:
            conn.sendall("El servidor se esta apagando. Chau!".encode('utf-8'))
        except OSError:
            pass
    

def main():     

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((HOST, PORT))
    server_socket.listen()
    
    server_socket.settimeout(1.0)
    print(f"Servidor escuchando en {HOST}:{PORT}...")

    try:
         
        while not shutdown_event.is_set():
            try:
                conn, addr = server_socket.accept()
                
            except socket.timeout:
                continue
            
            except OSError:
                break
            
            hilo = threading.Thread(target= atender_cliente, args=(conn,addr,server_socket),daemon=True)
            hilo.start()
            
            print(f"[CLIENTES ACTIVOS] {threading.active_count() - 1}")
    
    except KeyboardInterrupt:
        print("\n[APAGANDO] Ctrl+c detectado, cerrando servidor...")
        shutdown_event.set()
        
    print("[APAGANDO] Avisando a clientes conectados...")
    avisar_y_cerrar()
    
    
        
    server_socket.close()
    print("[APAGADO] Servidor apagado.")
    print("Servidor finalizado.")
    
                
if __name__ == "__main__":
    main() 