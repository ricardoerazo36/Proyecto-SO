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

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear socket");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Dirección inválida");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error al conectar al servidor");
        return -1;
    }

    printf("Conectado al servidor.\n");
    while (1) {
        printf("Ingrese comando (o 'salida' para terminar): ");
        fgets(command, BUFFER_SIZE, stdin);

        send(sock, command, strlen(command), 0);
        if (strcmp(command, "salida\n") == 0) {
            printf("Cerrando conexión...\n");
            break;
        }

        read(sock, buffer, BUFFER_SIZE);
        printf("Respuesta del servidor:\n%s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);
    }

    close(sock);
    return 0;
}
