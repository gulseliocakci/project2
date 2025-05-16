#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "headers/server.h"
#include "headers/mission.h"
#include "headers/drone.h"

#define TIMEOUT_THRESHOLD 10 // 10 seconds timeout

Mission mission_list[MAX_MISSIONS];
int mission_list_size = 0;

List *drones = NULL; // Drone listesi
pthread_mutex_t drones_lock = PTHREAD_MUTEX_INITIALIZER;

void* handle_drone(void* arg) {
    int drone_fd = *(int*)arg;
    free(arg);
    char buffer[1024];
    time_t last_message_time = time(NULL);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(drone_fd, buffer, sizeof(buffer));

        if (bytes_read <= 0) {
            printf("Drone disconnected. Closing connection.\n");
            close(drone_fd);

            pthread_mutex_lock(&drones_lock);
            Node *node = drones->head;
            while (node != NULL) {
                Drone *drone = (Drone *)node->data;
                if (drone->drone_fd == drone_fd) {
                    drone->status = DISCONNECTED;
                    handle_disconnected_drone(drone->id);
                    break;
                }
                node = node->next;
            }
            pthread_mutex_unlock(&drones_lock);
            return NULL;
        }

        last_message_time = time(NULL);
        printf("Received: %s\n", buffer);

        usleep(100000);
    }
}

void handle_disconnected_drone(int disconnected_drone_id) {
    printf("Handling disconnected drone: %d\n", disconnected_drone_id);

    pthread_mutex_lock(&drones_lock);
    for (int i = 0; i < mission_list_size; i++) {
        if (mission_list[i].assigned_drone_id == disconnected_drone_id) {
            Node *node = drones->head;
            while (node != NULL) {
                Drone *drone = (Drone *)node->data;
                if (drone->status == IDLE) {
                    mission_list[i].assigned_drone_id = drone->id;
                    drone->status = ON_MISSION;
                    printf("Mission reassigned to Drone %d\n", drone->id);
                    break;
                }
                node = node->next;
            }
        }
    }
    pthread_mutex_unlock(&drones_lock);
}

int main() {
    drones = create_list(sizeof(Drone), MAX_DRONES);

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