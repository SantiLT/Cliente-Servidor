/*
 * Cliente básico de sockets TCP en C.
 * Se conecta al servidor, manda un mensaje y espera la respuesta.
 * Equivalente al cliente.py básico.
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
    char buffer[BUFFER_SIZE] = {0};

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


    //---Mando un mensaje---

    const char *mensaje = "Hola servidor, soy el cliente!";
    send(sock_fd, mensaje, strlen(mensaje),0);

    //---Esperar respuesta---
    ssize_t bytes_leidos = read(sock_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_leidos < 0) {
        perror("Error al leer la respuesta");
    } else {
        buffer[bytes_leidos] = '\0';
        printf("Respuesta del servidor: %s\n", buffer);
    }


    close(sock_fd);

    return 0;
}
