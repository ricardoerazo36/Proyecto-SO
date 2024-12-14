/**
 * servidor.c
 * Servidor para una aplicación cliente-servidor compatible con Windows.
 * Recibe comandos del cliente, los ejecuta y envía la salida de vuelta.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Validar si el comando es permitido
int is_command_allowed(const char *command) {
    const char *allowed_commands[] = {
    "dir", "cd", "type", "copy", "del", "mkdir", "rmdir", "echo", 
    "date /t", "time /t", "hostname", "ver", "whoami", "tasklist", 
    "systeminfo", "fsutil", "findstr", NULL
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

    FILE *fp = _popen(command, "r"); // Ejecutar comando
    if (fp == NULL) {
        strcpy(response, "Error ejecutando comando\n");
        return;
    }

    size_t bytes_read = fread(response, 1, BUFFER_SIZE - 1, fp);
    response[bytes_read] = '\0'; // Asegurar fin de cadena
    _pclose(fp);
}

int main() {
    WSADATA wsa;
    SOCKET server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addrlen = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};

    // Inicializar Winsock
    printf("Inicializando Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error al inicializar Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    // Crear socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Error al crear socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Configurar dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Asociar socket al puerto
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Error al asociar socket: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        printf("Error al escuchar conexiones: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Servidor escuchando en el puerto %d...\n", PORT);

    // Aceptar conexiones
    while ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen)) != INVALID_SOCKET) {
        printf("Cliente conectado.\n");

        // Manejar comandos del cliente
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);

            int valread = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
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
            send(client_socket, response, strlen(response), 0);
        }

        closesocket(client_socket);
        printf("Esperando próxima conexión...\n");
    }

    closesocket(server_fd);
    WSACleanup();

    return 0;
}
