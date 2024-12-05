#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Ejecutar el comando recibido y capturar la salida
void execute_command(char *command, char *response) {
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        strcpy(response, "Error ejecutando comando\n");
        return;
    }
    fread(response, 1, BUFFER_SIZE, fp);
    pclose(fp);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};

    // Crear socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }

    // Configurar socket para reutilizar dirección y puerto
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Error al configurar socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Asociar socket al puerto
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error al asociar socket");
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones
    if (listen(server_fd, 3) < 0) {
        perror("Error al escuchar");
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("Error al aceptar conexión");
            exit(EXIT_FAILURE);
        }

        printf("Cliente conectado\n");
        while (1) {
            memset(buffer, 0, BUFFER_SIZE); // Limpiar buffer

            // Leer comando enviado por el cliente
            int valread = read(new_socket, buffer, BUFFER_SIZE);
            if (valread <= 0) {
                printf("Cliente desconectado.\n");
                break;
            }

            // Detectar si el cliente desea salir
            if (strcmp(buffer, "salida\n") == 0) {
                printf("Cliente solicitó salir. Cerrando conexión...\n");
                break;
            }

            printf("Comando recibido: %s\n", buffer);

            // Crear un nuevo proceso para ejecutar el comando
            pid_t pid = fork();
            if (pid < 0) {
                perror("Error al crear proceso");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // Proceso hijo: ejecutar el comando y enviar la respuesta
                execute_command(buffer, response);
                send(new_socket, response, strlen(response), 0);
                exit(0); // Termina el proceso hijo
            }
        }

        close(new_socket); // Cerrar conexión con el cliente
    }

    close(server_fd); // Cerrar socket del servidor
    return 0;
}
