#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_PORT 8080
#define MAX_DRONES 50

void* handle_drone(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);

    char buffer[1024];
    while (1) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("Drone disconnected.\n");
            close(client_socket);
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Received from drone: %s\n", buffer);

        // Example: Sending mission assignment
        const char* mission = "{\"type\": \"mission\", \"target\": [15, 25]}";
        send(client_socket, mission, strlen(mission), 0);
    }

    return NULL;
}

void* server_thread(void* arg) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return NULL;
    }

    if (listen(server_socket, MAX_DRONES) < 0) {
        perror("Listen failed");
        close(server_socket);
        return NULL;
    }

    printf("Server is listening on port %d\n", SERVER_PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int* client_socket = malloc(sizeof(int));
        *client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

        if (*client_socket < 0) {
            perror("Accept failed");
            free(client_socket);
            continue;
        }

        printf("Drone connected.\n");

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_drone, client_socket);
        pthread_detach(thread_id);
    }

    close(server_socket);
    return NULL;
}

int main() {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, server_thread, NULL);
    pthread_join(thread_id, NULL);

    return 0;
}