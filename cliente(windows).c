/**
 * cliente.c
 * Cliente para una aplicación cliente-servidor compatible con Windows.
 * Envía comandos al servidor y muestra las respuestas en pantalla.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE] = {0};
    char command[BUFFER_SIZE] = {0};

    // Inicializar Winsock
    printf("Inicializando Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error al inicializar Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    // Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Error al crear socket: %d\n", WSAGetLastError());
        return 1;
    }

    // Configurar dirección del servidor
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Cambiar IP si el servidor está en otra máquina

    // Conectar al servidor
    printf("Conectando al servidor...\n");
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Error al conectar al servidor: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Conectado al servidor.\n");

    // Bucle principal
    while (1) {
        printf("Ingrese comando (o 'salida' para terminar): ");
        fgets(command, BUFFER_SIZE, stdin);

        // Eliminar salto de línea al final del comando
        command[strcspn(command, "\n")] = '\0';

        // Detectar si el usuario desea salir
        if (strcmp(command, "salida") == 0) {
            send(sock, command, strlen(command), 0);
            printf("Cerrando conexión...\n");
            break;
        }

        // Enviar comando al servidor
        if (send(sock, command, strlen(command), 0) < 0) {
            printf("Error al enviar el comando.\n");
            continue;
        }

        // Leer respuesta del servidor
        int valread = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (valread < 0) {
            printf("Error al recibir respuesta del servidor.\n");
            break;
        }

        buffer[valread] = '\0'; // Asegurar fin de cadena
        printf("Respuesta del servidor:\n%s\n", buffer);

        // Limpiar buffers
        memset(buffer, 0, BUFFER_SIZE);
        memset(command, 0, BUFFER_SIZE);
    }

    // Cerrar socket y limpiar Winsock
    closesocket(sock);
    WSACleanup();

    return 0;
}
