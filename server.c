#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <signal.h>  // signal fonksiyonları için
#include <errno.h>   // errno için
#include <float.h>   // FLT_MAX için
#include "headers/server.h"
#include "headers/mission.h"
#include "headers/drone.h"
#include "headers/map.h"
#include "headers/list.h"
#include "headers/globals.h"
#include "headers/coord.h"
#include "headers/ai.h"

static pthread_t monitor_thread;
static int server_fd;

typedef struct {
    int missed_heartbeats;
    time_t last_heartbeat_time;
    int is_active;          // is_active ekledik
    int socket_fd;          // socket_fd ekledik
} DroneHeartbeatStatus;


static DroneHeartbeatStatus heartbeat_status[MAX_DRONES];
static pthread_mutex_t drones_lock = PTHREAD_MUTEX_INITIALIZER;

// extern değişkenler mission.h'dan geliyor
Mission mission_list[MAX_MISSIONS];
int mission_list_size = 0;

Coord create_coord(int x, int y) {
    Coord c = {x, y};
    return c;
}
// Mission için hedef koordinat alma fonksiyonu
 Coord get_mission_target(const Mission* mission) {
    Coord target;
    target.x = mission->target_x;
    target.y = mission->target_y;
    return target;
}


// Mission için hedef koordinat ayarlama fonksiyonu
 void set_mission_target(Mission* mission, Coord coord) {
    mission->target_x = coord.x;
    mission->target_y = coord.y;
}


void handle_heartbeat(int drone_id) {
    struct tm* tm_info = localtime(&current_time);
    char timestamp[26];
    strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    pthread_mutex_lock(&drones_lock);
    heartbeat_status[drone_id].missed_heartbeats = 0;
    heartbeat_status[drone_id].last_heartbeat_time = current_time;
    heartbeat_status[drone_id].is_active = 1;
    printf("[%s] User %s: Heartbeat received from Drone %d\n", 
           timestamp, current_user, drone_id);
    pthread_mutex_unlock(&drones_lock);
}

void check_drone_timeouts() {
    struct tm* tm_info = localtime(&current_time);
    char timestamp[26];
    strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    pthread_mutex_lock(&drones_lock);
    for (int i = 0; i < MAX_DRONES; i++) {
        if (heartbeat_status[i].is_active) {
            if (heartbeat_status[i].missed_heartbeats >= MAX_MISSED_HEARTBEATS) {
                printf("[%s] User %s: Drone %d marked as disconnected due to missed heartbeats.\n",
                       timestamp, current_user, i);
                heartbeat_status[i].is_active = 0;
                handle_disconnected_drone(i);
            } else if (current_time - heartbeat_status[i].last_heartbeat_time > TIMEOUT_THRESHOLD) {
                heartbeat_status[i].missed_heartbeats++;
                if (current_time - heartbeat_status[i].last_heartbeat_time > TIMEOUT_THRESHOLD * 3) {
                    printf("[%s] User %s: Drone %d connection timeout.\n",
                           timestamp, current_user, i);
                    heartbeat_status[i].is_active = 0;
                    handle_disconnected_drone(i);
                }
            }
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
    char timestamp[26];

    while (!should_quit) {
        memset(buffer, 0, sizeof(buffer));
        
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(drone_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        
        int bytes_read = read(drone_fd, buffer, sizeof(buffer));

        if (bytes_read <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            if (should_quit) {
                break;
            }
            
            struct tm *tm_info = localtime(&current_time);
            strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);
            printf("[%s] User %s: Drone disconnected. Closing connection.\n", 
                   timestamp, current_user);
            
            close(drone_fd);

            pthread_mutex_lock(&drones_lock);
            Node *node = drones->head;
            while (node != NULL) {
                Drone *drone = (Drone *)node->data;
                if (drone->drone_fd == drone_fd) {
                    drone->status = DISCONNECTED;
                    printf("[%s] User %s: Handling disconnection of Drone %d\n", 
                           timestamp, current_user, drone->id);
                    handle_disconnected_drone(drone->id);
                    break;
                }
                node = node->next;
            }
            pthread_mutex_unlock(&drones_lock);
            return NULL;
        }

        printf("[%s] User %s received: %s\n", timestamp, current_user, buffer);
    }
    
    close(drone_fd);
    return NULL;
}


void handle_disconnected_drone(int disconnected_drone_id) {
    struct tm* tm_info = localtime(&current_time);
    char timestamp[26];
    strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    pthread_mutex_lock(&drones_lock);
    for (int i = 0; i < mission_list_size; i++) {
        if (mission_list[i].assigned_drone_id == disconnected_drone_id) {
            Node *closest_drone = NULL;
            float min_distance = FLT_MAX;
            
            Coord mission_target = get_mission_target(&mission_list[i]);
            
            Node *node = drones->head;
            while (node != NULL) {
                Drone *drone = (Drone *)node->data;
                if (drone->status == IDLE) {
                    float distance = calculate_distance(drone->coord, mission_target);
                    if (distance < min_distance) {
                        min_distance = distance;
                        closest_drone = node;
                    }
                }
                node = node->next;
            }
            
            if (closest_drone != NULL) {
                Drone *new_drone = (Drone *)closest_drone->data;
                mission_list[i].assigned_drone_id = new_drone->id;
                new_drone->status = ON_MISSION;
                new_drone->target = mission_target;
                printf("[%s] User %s: Mission reassigned from Drone %d to Drone %d\n",
                       timestamp, current_user, disconnected_drone_id, new_drone->id);
            }
        }
    }
    pthread_mutex_unlock(&drones_lock);
}
void start_drone_server(void) {
    struct sockaddr_in address;
    int opt = 1;
    char timestamp[26];

    // Signal handler'ları kaydet
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Soket oluştur
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        struct tm *tm_info = localtime(&current_time);
        strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        printf("[%s] User %s: Socket creation failed\n", timestamp, current_user);
        return;
    }

    // Socket seçeneklerini ayarla
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        struct tm *tm_info = localtime(&current_time);
        strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        printf("[%s] User %s: setsockopt failed\n", timestamp, current_user);
        close(server_fd);
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        struct tm *tm_info = localtime(&current_time);
        strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        printf("[%s] User %s: Bind failed\n", timestamp, current_user);
        close(server_fd);
        return;
    }

    // Listen
    if (listen(server_fd, 10) < 0) {
        struct tm *tm_info = localtime(&current_time);
        strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        printf("[%s] User %s: Listen failed\n", timestamp, current_user);
        close(server_fd);
        return;
    }

    struct tm *tm_info = localtime(&current_time);
    strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] User %s: Server listening on port %d\n", 
           timestamp, current_user, SERVER_PORT);

    // Monitor thread'ini başlat
    pthread_create(&monitor_thread, NULL, monitor_heartbeats, NULL);

    while (!should_quit) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        
        int *client_sock = malloc(sizeof(int));
        *client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (*client_sock < 0) {
            free(client_sock);
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            if (should_quit) {
                break;
            }
            tm_info = localtime(&current_time);
            strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);
            printf("[%s] User %s: Accept failed\n", timestamp, current_user);
            continue;
        }

        tm_info = localtime(&current_time);
        strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        printf("[%s] User %s: New connection accepted\n", timestamp, current_user);

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_drone, client_sock) != 0) {
            tm_info = localtime(&current_time);
            strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);
            printf("[%s] User %s: Failed to create thread for drone connection\n", 
                   timestamp, current_user);
            close(*client_sock);
            free(client_sock);
            continue;
        }
        pthread_detach(thread_id);
    }

    // Temizlik işlemleri
    printf("\nServer shutting down...\n");
    
    // Monitor thread'ini bekle
    pthread_cancel(monitor_thread);
    pthread_join(monitor_thread, NULL);
    
    // Server socket'i kapat
    if (server_fd > 0) {
        close(server_fd);
    }
    
    printf("Server shutdown complete.\n");
}

void signal_handler(int signum) {
    char timestamp[26];
    struct tm *tm_info = localtime(&current_time);
    strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    
    printf("\n[%s] User %s initiated shutdown (Signal: %d)\n", 
           timestamp, current_user, signum);
    
    // Global flag'i set et
    should_quit = 1;
    
    // Tüm aktif drone'ları durdur
    pthread_mutex_lock(&drones_lock);
    Node *node = drones->head;
    while (node != NULL) {
        Drone *drone = (Drone *)node->data;
        if (drone->status != DISCONNECTED) {
            printf("[%s] Shutting down Drone %d operated by %s\n", 
                   timestamp, drone->id, current_user);
            drone->status = DISCONNECTED;
            if (drone->drone_fd > 0) {
                close(drone->drone_fd);
            }
        }
        node = node->next;
    }
    pthread_mutex_unlock(&drones_lock);
    
    // Monitor thread'ini bekle
    if (monitor_thread) {
        pthread_cancel(monitor_thread);
        pthread_join(monitor_thread, NULL);
        printf("[%s] Monitor thread stopped by %s\n", timestamp, current_user);
    }
    
    // Server socket'i kapat
    if (server_fd > 0) {
        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
        printf("[%s] Server socket closed by %s\n", timestamp, current_user);
    }
    
    // Listeleri temizle
    if (drones) {
        drones->destroy(drones);
        printf("[%s] Drones list destroyed by %s\n", timestamp, current_user);
    }
    if (survivors) {
        survivors->destroy(survivors);
        printf("[%s] Survivors list destroyed by %s\n", timestamp, current_user);
    }
    
    printf("[%s] %s completed shutdown process\n", timestamp, current_user);
    exit(0);
}