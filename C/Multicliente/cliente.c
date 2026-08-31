/*
 * Cliente de sockets TCP con loop de mensajes, en C.
 * Se conecta al servidor y permite mandar varios mensajes seguidos
 * desde la terminal, hasta escribir "salir". Equivalente a cliente-loop.py.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define PORT 65432
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

int main(){
    int sock_fd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char mensaje[BUFFER_SIZE];

    //---creo el socket---

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }

    // ---Configuro direccion del servidor---

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);


    if ( inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0){
        perror("Direccion IP invalida");
        exit(EXIT_FAILURE);
    }


    //---Conecto al server---

    if(connect(sock_fd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Error al conectar");
        exit(EXIT_FAILURE);
    }

    printf("Conectado al servidor. Escribi mensajes (o salir para cortar).\n");


    //---Mando mensajes---

    while (1)
    {
        printf(">");

        if(fgets(mensaje, BUFFER_SIZE, stdin) == NULL){
            break;
        }

        mensaje[strcspn(mensaje, "\n")] = '\0';

        send(sock_fd, mensaje, strlen(mensaje), 0);

        memset(buffer, 0, BUFFER_SIZE);

        ssize_t bytes_leidos = read(sock_fd, buffer, BUFFER_SIZE - 1);

        if (bytes_leidos <= 0)
        {
            printf("El servidor cerro la conexion.\n");
            break;
        }

        buffer[bytes_leidos] = '\0';
        printf("Servidor: %s\n", buffer);

        if (strncasecmp(mensaje, "salir", 5) == 0 && strlen(mensaje) == 5)
        {
            break;
        }
        
        

    }


    close(sock_fd);

    return 0;
}
