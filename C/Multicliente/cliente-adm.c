/*
 * Cliente admin en C: se autentica con contraseña y después puede
 * chatear normalmente O mandar "shutdown" en cualquier momento para
 * apagar el servidor remotamente. Equivalente a cliente-admin.py.
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
    char password[BUFFER_SIZE];

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

    printf("Contraseña de admin: ");
    if (fgets(password, BUFFER_SIZE, stdin) == NULL)
    {
        exit(EXIT_FAILURE);
    }
    
    password[strcspn(password, "\n")] = '\0';

    snprintf(mensaje, BUFFER_SIZE, "ADMIN:%s", password);
    send(sock_fd,mensaje,strlen(mensaje), 0);

    ssize_t bytes_leidos = read(sock_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_leidos <= 0)
        {
            printf("El servidor cerro la conexion.\n");
            close(sock_fd);
            return 1;
        }

    buffer[bytes_leidos] = '\0';
    printf("Servidor: %s\n", buffer);

    if (strncmp(buffer,"Contraseña incorrecta", 21) == 0)
    {
        close(sock_fd);
        return 1;
    }
    

    printf("Escribi mensajes normales, o 'shutdown' para apagar el servidor.\n");


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
        bytes_leidos = read(sock_fd, buffer,BUFFER_SIZE - 1);

        if (bytes_leidos <= 0)
        {
            printf("El servidor cerro la conexion.\n");
            break;
        }

        buffer[bytes_leidos] = '\0';
        printf("Servidor: %s\n", buffer);

        int es_salir = (strncasecmp(mensaje,"salir",5) == 0 && strlen(mensaje) == 5);
        int es_shutdown = (strncasecmp(mensaje,"shutdown",8) == 0 && strlen(mensaje) == 8);

        if (es_salir || es_shutdown)
        {
            break;
        }
        
        

    }


    close(sock_fd);
    printf("Conexion cerrada.\n");

    return 0;
}
