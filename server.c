#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include "headers/server.h"
#include "headers/mission.h"
#include "headers/drone.h"
#include "headers/map.h"
#include "headers/list.h"
#include "headers/globals.h"

typedef struct {
    int missed_heartbeats;
    time_t last_heartbeat_time;
} DroneHeartbeatStatus;

static DroneHeartbeatStatus heartbeat_status[MAX_DRONES];
static pthread_mutex_t drones_lock = PTHREAD_MUTEX_INITIALIZER;

// extern değişkenler mission.h'dan geliyor
Mission mission_list[MAX_MISSIONS];
int mission_list_size = 0;

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
    while (!should_quit) {
        check_drone_timeouts();
        sleep(1);
    }
    return NULL;
}

void* handle_drone(void* arg) {
    int drone_fd = *(int*)arg;
    free(arg);
    char buffer[1024];

    while (!should_quit) {
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

        printf("Received: %s\n", buffer);

        // JSON mesajını parse et
        // ... (JSON işleme kodları buraya gelecek)

        usleep(100000);
    }
    return NULL;
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

void start_drone_server(void) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // Soket oluştur
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        return;
    }

    // Socket seçeneklerini ayarla
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return;
    }

    // Listen
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        return;
    }

    printf("Server listening on port %d\n", SERVER_PORT);

    // Heartbeat monitor thread'ini başlat
    pthread_t heartbeat_thread;
    pthread_create(&heartbeat_thread, NULL, monitor_heartbeats, NULL);

    while (!should_quit) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int *client_sock = malloc(sizeof(int));
        *client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (*client_sock < 0) {
            perror("Accept failed");
            free(client_sock);
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_drone, client_sock);
        pthread_detach(thread_id);
    }

    pthread_cancel(heartbeat_thread);
    pthread_join(heartbeat_thread, NULL);
    close(server_fd);
}