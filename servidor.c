/**
 * servidor.c
 * Este programa actúa como servidor en una aplicación cliente-servidor.
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

// Validar si el comando es permitido (no interactivo)
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
void execute_command(char *command, char *response) {
    if (!is_command_allowed(command)) {
        strcpy(response, "Error: Comando no permitido.\n");
        return;
    }

    FILE *fp = popen(command, "r");  // Ejecutar comando
    if (fp == NULL) {
        strcpy(response, "Error ejecutando comando\n");
        return;
    }
    fread(response, 1, BUFFER_SIZE, fp);  // Leer salida
    pclose(fp);
}

int main() {
    int server_fd, new_socket;       // Descriptor del socket
    struct sockaddr_in address;     // Dirección del servidor
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0}; // Buffer para comandos
    char response[BUFFER_SIZE] = {0}; // Buffer para respuestas

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

    // Configurar dirección del servidor
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
        // Aceptar nueva conexión
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

            // Crear proceso hijo para ejecutar comando
            pid_t pid = fork();
            if (pid < 0) {
                perror("Error al crear proceso");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                execute_command(buffer, response);  // Ejecutar comando
                send(new_socket, response, strlen(response), 0);  // Enviar salida
                exit(0);  // Termina proceso hijo
            }
        }

        close(new_socket); // Cerrar conexión con el cliente
    }

    close(server_fd); // Cerrar socket del servidor
    return 0;
}
