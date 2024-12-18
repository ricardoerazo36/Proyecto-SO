/**
 * servidor.c
 * Servidor para una aplicación cliente-servidor.
 * Recibe comandos del cliente, los ejecuta y envía la salida de vuelta.
 * Solo se permiten comandos no interactivos.
 * Autores: Ricardo Erazo, Juan Manuel Perea, Andrés Ramirez, Daniel Cárdenas.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Validar si el comando es permitido
int is_command_allowed(const char *command) {
    const char *allowed_commands[] = {
        "ls", "pwd", "cat", "cp", "rm", "mkdir", "rmdir", "touch",
        "df", "du", "who", "id", "uptime", "hostname", "uname", "date",
        "echo", "head", "tail", "wc", "grep", "stat", "file", NULL
    };

    for (int i = 0; allowed_commands[i] != NULL; i++) {
        if (strncmp(command, allowed_commands[i], strlen(allowed_commands[i])) == 0) {
            return 1; // Comando permitido
        }
    }
    return 0; // Comando no permitido
}

// Ejecutar el comando y capturar la salida
void execute_command(const char *command, char *response) {
    if (!is_command_allowed(command)) {
        strcpy(response, "Error: Comando no permitido.\n");
        return;
    }

    FILE *fp = popen(command, "r"); // Ejecutar comando
    if (fp == NULL) {
        strcpy(response, "Error: No se pudo ejecutar el comando.\n");
        return;
    }

    size_t bytes_read = fread(response, 1, BUFFER_SIZE - 1, fp); // Leer salida
    response[bytes_read] = '\0'; // Asegurar fin de cadena
    pclose(fp);

    if (bytes_read == 0) {
        if (strncmp(command, "rm ", 3) == 0) {
            snprintf(response, BUFFER_SIZE, "Comando ejecutado con éxito. Se eliminó el archivo %s.\n", command + 3);
        } else if (strncmp(command, "cp ", 3) == 0) {
            snprintf(response, BUFFER_SIZE, "Comando ejecutado con éxito. Se copió el archivo %s.\n", command + 3);
        } else if (strncmp(command, "mkdir ", 6) == 0) {
            snprintf(response, BUFFER_SIZE, "Comando ejecutado con éxito. Se creó el directorio %s.\n", command + 6);
        } else if (strncmp(command, "rmdir ", 6) == 0) {
            snprintf(response, BUFFER_SIZE, "Comando ejecutado con éxito. Se eliminó el directorio %s.\n", command + 6);
        } else if (strncmp(command, "touch ", 6) == 0) {
            snprintf(response, BUFFER_SIZE, "Comando ejecutado con éxito. Se creó el archivo %s.\n", command + 6);
        } else {
            strcpy(response, "Comando ejecutado con éxito, sin salida.\n");
        }
    }
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
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Configurar dirección del servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Asociar socket al puerto
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error al asociar socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, 3) < 0) {
        perror("Error al escuchar conexiones");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto %d...\n", PORT);

    while (1) {
        // Aceptar nueva conexión
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("Error al aceptar conexión");
            continue;
        }

        printf("Cliente conectado.\n");

        // Manejar comandos del cliente
        while (1) {
            memset(buffer, 0, BUFFER_SIZE); // Limpiar buffer

            // Leer comando enviado por el cliente
            ssize_t valread = read(new_socket, buffer, BUFFER_SIZE - 1);
            if (valread <= 0) {
                printf("Cliente desconectado.\n");
                break;
            }

            buffer[valread] = '\0'; // Asegurar fin de cadena
            printf("Comando recibido: %s\n", buffer);

            // Detectar si el cliente desea salir
            if (strcmp(buffer, "salida") == 0) {
                printf("Cliente solicitó salir. Cerrando conexión...\n");
                break;
            }

            // Ejecutar comando y enviar respuesta
            memset(response, 0, BUFFER_SIZE);
            execute_command(buffer, response);
            if (send(new_socket, response, strlen(response), 0) < 0) {
                perror("Error al enviar respuesta");
                break;
            }
        }

        close(new_socket); // Cerrar conexión con el cliente
    }

    close(server_fd); // Cerrar socket del servidor
    return 0;
}
