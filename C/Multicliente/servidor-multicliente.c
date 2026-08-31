/*
 * Servidor de sockets TCP multicliente en C, con panel de administración
 * remota. Equivalente a servidor-multicliente.py (versión con admin).
 *
 * Un cliente puede autenticarse como admin mandando "ADMIN:<contraseña>"
 * como primer mensaje. Una vez autenticado, puede chatear normalmente
 * o mandar "shutdown" en cualquier momento para apagar el servidor.
 */




 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <arpa/inet.h>
 #include <pthread.h>
 #include <sys/select.h>

 #define PORT 65432
 #define BUFFER_SIZE 1024
 #define MAX_CLIENTES 100


 volatile int shutdown_flag = 0;

 int clientes_fds[MAX_CLIENTES];
 int num_clientes = 0;
 pthread_mutex_t clientes_locks = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    int client_fd;
    struct sockaddr_in address;
} datos_cliente;



void *atender_cliente(void *arg){
    datos_cliente *datos = (datos_cliente *)arg;
    int client_fd = datos -> client_fd;

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &datos -> address.sin_addr,ip_str, INET_ADDRSTRLEN);
    int port = ntohs(datos -> address.sin_port);

    printf("[NUEVA CONEXION] %s:%d conectado.\n", ip_str, port);

    char buffer [BUFFER_SIZE];
    
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_leidos = read(client_fd, buffer, BUFFER_SIZE - 1);

        if(bytes_leidos < 0){
            printf("Eroor al leer del cliente");
            break;
        }

        if (bytes_leidos == 0)
        {
            printf("[DESCONECTADO] %s:%d cerro la conexion.\n", ip_str, port);
            break;
        }

        buffer[bytes_leidos] = '\0';
        printf("[%s:%d] dice: %s\n", ip_str, port, buffer);

        if (strncasecmp(buffer, "salir", 5) == 0 && bytes_leidos == 5)
        {
            const char *despedida = "Chau!";
            send(client_fd, despedida,strlen(despedida), 0);
            printf("[DESCONECTADO] %s:%d pidio salir.\n", ip_str, port);
            break;
        }
    
        char respuesta[BUFFER_SIZE];
        snprintf(respuesta, BUFFER_SIZE, "Servidor recibio: '%s'", buffer);
        send(client_fd, respuesta, strlen(respuesta), 0);
        
    }
    
    close(client_fd);
    free(datos);
    return NULL;
}

 int main(){

    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);

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

    if(listen(server_fd, 5) < 0){
        perror("Error en listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto %d... \n", PORT);

    while (1)
    {
        //---accept---
        client_fd = accept(server_fd, (struct sockaddr*)&address, &addr_len);
        if (client_fd < 0){
        perror("Error en accept");
        continue;
        }

        datos_cliente *datos = malloc(sizeof(datos_cliente));
        datos -> client_fd = client_fd;
        datos -> address = address;

        pthread_t hilo;

        if (pthread_create(&hilo, NULL, atender_cliente,datos) != 0)
        {
            perror("Error al crear el thread");
            free(datos);
            close(client_fd);
            continue;
        }

        pthread_detach(hilo);
        
    }


    close(server_fd);
    return 0;
 }