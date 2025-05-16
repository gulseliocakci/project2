#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "headers/server.h" // Server-specific header file
#include "headers/mission.h"
#define TIMEOUT_THRESHOLD 10 // 10 seconds timeout
#include "headers/map.h" // Düzgün tanımlamalar için map.h dahil 

Mission mission_list[MAX_MISSIONS]; // Görev listesini tanımla
int mission_list_size = 0;          // Görev listesi başlangıç boyutu
/*
Drone drones[MAX_DRONES]; // Drone dizisini tanımlıyoruz
int drone_count = 0;      // Aktif drone sayısını sıfırla
*/
List *drones = NULL;
int drone_count = 0; 

// Global drone list and mutex for thread safety
pthread_mutex_t drones_lock = PTHREAD_MUTEX_INITIALIZER;
Drone drones[MAX_DRONES];
int drone_count = 0;

// Function to handle communication with a single drone
void* handle_drone(void* arg) {
    int drone_fd = *(int*)arg;
    char buffer[1024];
    time_t last_message_time = time(NULL);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(drone_fd, buffer, sizeof(buffer));

        if (bytes_read <= 0) {
            printf("Drone disconnected. Closing connection.\n");
            close(drone_fd);

            // Mark drone as disconnected
            pthread_mutex_lock(&drones_lock);
            for (int i = 0; i < drone_count; i++) {
                if (drones[i].drone_fd == drone_fd) {
                    drones[i].status = DISCONNECTED;
                    handle_disconnected_drone(drones[i].id); // Reassign missions
                    break;
                }
            }
            pthread_mutex_unlock(&drones_lock);
            return NULL;
        }

        last_message_time = time(NULL); // Update last message time
        printf("Received: %s\n", buffer);

        // Simulate message processing delay
        usleep(100000);
    }
}

// Function to handle disconnected drones
void handle_disconnected_drone(int disconnected_drone_id) {
    printf("Handling disconnected drone: %d\n", disconnected_drone_id);

    pthread_mutex_lock(&drones_lock);
    for (int i = 0; i < mission_list_size; i++) {
        if (mission_list[i].assigned_drone_id == disconnected_drone_id) {
            // Reassign mission
            for (int j = 0; j < drone_count; j++) {
                if (drones[j].status == IDLE) {
                    mission_list[i].assigned_drone_id = drones[j].id;
                    drones[j].status = ON_MISSION;
                    printf("Mission reassigned to Drone %d\n", drones[j].id);
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&drones_lock);
}

// Main server function
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

    return 0;
}