#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "../headers/drone.h"

// Metrics for Load Testing
typedef struct {
    int total_connections;
    int active_connections;
    int total_messages_received;
    double total_connection_time;
} ServerMetrics;

ServerMetrics metrics = {0, 0, 0, 0.0};
pthread_mutex_t metrics_lock = PTHREAD_MUTEX_INITIALIZER;

void* handle_drone(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);

    char buffer[1024] = {0};
    clock_t start_time, end_time;

    start_time = clock();
    pthread_mutex_lock(&metrics_lock);
    metrics.total_connections++;
    metrics.active_connections++;
    pthread_mutex_unlock(&metrics_lock);

    printf("New drone connected. Active drones: %d\n", metrics.active_connections);

    while (1) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("Drone disconnected. Closing connection.\n");
            close(client_socket);
            pthread_mutex_lock(&metrics_lock);
            metrics.active_connections--;
            pthread_mutex_unlock(&metrics_lock);
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);

        pthread_mutex_lock(&metrics_lock);
        metrics.total_messages_received++;
        pthread_mutex_unlock(&metrics_lock);

        // Simulate some processing delay
        usleep(100000);
    }

    end_time = clock();
    pthread_mutex_lock(&metrics_lock);
    metrics.total_connection_time += ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    pthread_mutex_unlock(&metrics_lock);

    return NULL;
}

int main() {
    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 50) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 8080...\n");

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

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_drone, (void*)client_socket);
        pthread_detach(thread_id);
    }

    // Server metrics report (this won't run unless server is terminated manually)
    printf("\n=== Server Metrics ===\n");
    printf("Total Connections: %d\n", metrics.total_connections);
    printf("Active Connections: %d\n", metrics.active_connections);
    printf("Total Messages Received: %d\n", metrics.total_messages_received);
    printf("Average Connection Time: %.2f seconds\n",
           metrics.total_connections > 0 ? metrics.total_connection_time / metrics.total_connections : 0.0);

    return 0;
}