#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char command[BUFFER_SIZE] = {0};

    // Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear socket");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Dirección IP del servidor
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Dirección inválida");
        return -1;
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error al conectar al servidor");
        return -1;
    }

    printf("Conectado al servidor.\n");
    while (1) {
        printf("Ingrese comando (o 'salida' para terminar): ");
        fgets(command, BUFFER_SIZE, stdin);

        // Detectar si el usuario desea salir
        if (strcmp(command, "salida\n") == 0) {
            send(sock, command, strlen(command), 0);
            printf("Cerrando conexión...\n");
            break;
        }

        // Crear un nuevo proceso para enviar el comando
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error al crear proceso");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Proceso hijo: enviar comando
            send(sock, command, strlen(command), 0);
            exit(0); // Termina el proceso hijo
        }

        // Recibir y mostrar la respuesta
        read(sock, buffer, BUFFER_SIZE);
        printf("Respuesta del servidor:\n%s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE); // Limpiar buffer
    }

    close(sock); // Cerrar socket
    return 0;
}
