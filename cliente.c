/**
 * cliente.c
 * Cliente para una aplicación cliente-servidor.
 * Envía comandos al servidor y muestra las respuestas en pantalla.
 * Autores: Ricardo Erazo, Juan Manuel Perea, Andrés Ramirez, Daniel Cárdenas.
 */

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
        return EXIT_FAILURE;
    }

    // Configurar dirección del servidor
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Dirección IP inválida o no soportada");
        close(sock);
        return EXIT_FAILURE;
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error al conectar al servidor");
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Conectado al servidor.\n");

    // Bucle principal
    while (1) {
        printf("Ingrese comando (o 'salida' para terminar): ");
        if (fgets(command, BUFFER_SIZE, stdin) == NULL) {
            perror("Error al leer el comando");
            continue;
        }

        // Eliminar el salto de línea al final del comando
        size_t len = strlen(command);
        if (command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        // Comando de salida
        if (strcmp(command, "salida") == 0) {
            if (send(sock, command, strlen(command), 0) < 0) {
                perror("Error al enviar comando de salida");
            }
            printf("Cerrando conexión...\n");
            break;
        }

        // Enviar comando al servidor
        if (send(sock, command, strlen(command), 0) < 0) {
            perror("Error al enviar el comando");
            continue;
        }

        // Leer respuesta del servidor
        ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE - 1);
        if (bytes_read < 0) {
            perror("Error al leer la respuesta del servidor");
            continue;
        }

        buffer[bytes_read] = '\0'; // Asegurar fin de cadena
        printf("Respuesta del servidor:\n%s\n", buffer);

        // Limpiar buffers
        memset(buffer, 0, BUFFER_SIZE);
        memset(command, 0, BUFFER_SIZE);
    }

    close(sock);
    return EXIT_SUCCESS;
}
