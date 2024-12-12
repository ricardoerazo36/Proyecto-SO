/**
 * cliente.c
 * Este programa actúa como cliente en una aplicación cliente-servidor.
 * Envia comandos al servidor y muestra las respuestas en pantalla.
 * Autores: Ricardo Erazo, Juan Manuel Perea, Andrés Ramirez, Daniel Cárdenas. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080            // Puerto del servidor
#define BUFFER_SIZE 1024     // Tamaño del buffer para comandos y respuestas

int main() {
    int sock = 0;                     // Descriptor del socket
    struct sockaddr_in serv_addr;     // Estructura para la dirección del servidor
    char buffer[BUFFER_SIZE] = {0};   // Buffer para recibir respuestas
    char command[BUFFER_SIZE] = {0};  // Buffer para enviar comandos

    // Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear socket");
        return -1;
    }

    // Configurar la dirección del servidor
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertir dirección IP a formato binario
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

    // Bucle para leer comandos y enviar al servidor
    while (1) {
        printf("Ingrese comando (o 'salida' para terminar): ");
        fgets(command, BUFFER_SIZE, stdin);  // Leer comando del usuario

        // Detectar si el usuario desea salir
        if (strcmp(command, "salida\n") == 0) {
            send(sock, command, strlen(command), 0);
            printf("Cerrando conexión...\n");
            break;
        }

        // Crear proceso hijo para enviar comando
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error al crear proceso");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            send(sock, command, strlen(command), 0);  // Enviar comando al servidor
            exit(0);
        }

        // Leer respuesta del servidor
        read(sock, buffer, BUFFER_SIZE);
        printf("Respuesta del servidor:\n%s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);  // Limpiar buffer
    }

    close(sock);  // Cerrar conexión
    return 0;
}
