/*
 * Servidor básico de sockets TCP en C.
 * Se queda escuchando en un puerto, acepta una conexión,
 * recibe un mensaje y responde. Equivalente al servidor.py básico.
 */


 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <arpa/inet.h>

 #define PORT 65432
 #define BUFFER_SIZE 1024

 int main(){

    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    //---socket---

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }
    

    //---Reusar la direccion---

    int opt = 1;
    setsockopt(server_fd,SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //---Configurar direccion de listen---

    memset(&address, 0 , sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;

    address.sin_port = htons(PORT);


    //---Asociar socket a direccion/puerto---

    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address))< 0) {
        perror("Error en bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    //---listen---

    if(listen(server_fd, 1) < 0){
        perror("Error en listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto %d... \n", PORT);

    //---accept---

    client_fd = accept(server_fd, (struct sockaddr*)&address, &addr_len);
    if (client_fd < 0){
        perror("Error en accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    //---recv---

    ssize_t bytes_leidos = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_leidos < 0){
        perror("Error al leer del cliente");
    }else{
        buffer[bytes_leidos] = '\0';
        printf("Mensaje recibido: %s\n", buffer);
    }

    //---responder al cliente ---

    char respuesta[BUFFER_SIZE];
    snprintf(respuesta, BUFFER_SIZE, "Servidor recibio: '%s'", buffer);
    send(client_fd, respuesta, strlen(respuesta), 0);

    close(client_fd);
    close(server_fd);

    printf("Conexion cerrada.\n");

    return 0;
 }