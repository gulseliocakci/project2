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
#include "headers/map.h"
#include "headers/list.h"

#define TIMEOUT_THRESHOLD 10 // 10 saniyelik timeout
#define MAX_MISSED_HEARTBEATS 3 // Maksimum izin verilen kaçırılan heartbeat sayısı

Mission mission_list[MAX_MISSIONS];
int mission_list_size = 0;

Map map = {100, 100, NULL}; // Varsayılan olarak 100x100 boyutunda bir harita
List *drones = NULL; // Drone listesi
pthread_mutex_t drones_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int missed_heartbeats;
    time_t last_heartbeat_time;
} DroneHeartbeatStatus;

DroneHeartbeatStatus heartbeat_status[MAX_DRONES];

void handle_heartbeat(int drone_id) {
    pthread_mutex_lock(&drones_lock);
    heartbeat_status[drone_id].missed_heartbeats = 0;
    heartbeat_status[drone_id].last_heartbeat_time = time(NULL);
    printf("Heartbeat received from Drone %d\n", drone_id);
    pthread_mutex_unlock(&drones_lock);
}

void check_drone_timeouts() {
    time_t current_time = time(NULL);
    pthread_mutex_lock(&drones_lock);
    for (int i = 0; i < MAX_DRONES; i++) {
        if (heartbeat_status[i].missed_heartbeats >= MAX_MISSED_HEARTBEATS) {
            printf("Drone %d marked as disconnected due to missed heartbeats.\n", i);
            handle_disconnected_drone(i);
        } else if (current_time - heartbeat_status[i].last_heartbeat_time > TIMEOUT_THRESHOLD) {
            heartbeat_status[i].missed_heartbeats++;
        }
    }
    pthread_mutex_unlock(&drones_lock);
}

void* monitor_heartbeats(void* arg) {
    while (1) {
        check_drone_timeouts();
        sleep(1); // Her saniye kontrol et
    }
}

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

    pthread_t heartbeat_thread;
    pthread_create(&heartbeat_thread, NULL, monitor_heartbeats, NULL);

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

    pthread_join(heartbeat_thread, NULL);
    return 0;
}