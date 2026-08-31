/*
 * Servidor de sockets TCP con loop de mensajes, en C.
 * Se queda escuchando, acepta un cliente, y mantiene una conversación
 * (recibe y responde varias veces) hasta que el cliente se desconecta
 * o manda "salir". Equivalente a servidor-loop.py.
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

    printf("Cliente conectado.\n");

    //---recv---

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_leidos = read(client_fd, buffer,BUFFER_SIZE - 1);

        if (bytes_leidos <= 0)
        {
            perror("El cliente cerro la conexion.\n");
            break;
        }

        buffer[bytes_leidos] = '\0';
        printf("Mensaje recibido: %s\n", buffer);

        if (strncasecmp(buffer,"salir", 5) == 0 && bytes_leidos == 5)
        {
            printf("El cliente pidio salir.\n");
            const char *despedida = "Chau!";
            send(client_fd, despedida, strlen(despedida), 0);
            break;
        }
        
        //---responder al cliente ---

    char respuesta[BUFFER_SIZE];
    snprintf(respuesta, BUFFER_SIZE, "Servidor recibio: '%s'", buffer);
    send(client_fd, respuesta, strlen(respuesta), 0);
        
    }

    close(client_fd);
    close(server_fd);

    printf("Conexion cerrada.\n");

    return 0;
 }