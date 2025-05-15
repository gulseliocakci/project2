#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include "map.h" // Shared structures like Map, Coord

#define MAX_DRONES 100
#define PORT 8080

typedef struct {
    int drone_fd;
    char drone_id[16];
    char status[16];
    Coord location;
} Drone;

pthread_mutex_t drones_lock = PTHREAD_MUTEX_INITIALIZER;
Drone drones[MAX_DRONES];
int drone_count = 0;

void* handle_drone(void* arg) {
    int drone_fd = *(int*)arg;
    char buffer[1024];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(drone_fd, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            printf("Drone disconnected.\n");
            close(drone_fd);
            return NULL;
        }
        printf("Received: %s\n", buffer);

        // Parse JSON data
        struct json_object* parsed_json = json_tokener_parse(buffer);
        struct json_object* drone_id;
        struct json_object* status;
        struct json_object* location;

        json_object_object_get_ex(parsed_json, "drone_id", &drone_id);
        json_object_object_get_ex(parsed_json, "status", &status);
        json_object_object_get_ex(parsed_json, "location", &location);

        pthread_mutex_lock(&drones_lock);
        for (int i = 0; i < drone_count; i++) {
            if (strcmp(drones[i].drone_id, json_object_get_string(drone_id)) == 0) {
                strcpy(drones[i].status, json_object_get_string(status));
                drones[i].location.x = json_object_get_int(json_object_array_get_idx(location, 0));
                drones[i].location.y = json_object_get_int(json_object_array_get_idx(location, 1));
                break;
            }
        }
        pthread_mutex_unlock(&drones_lock);
        json_object_put(parsed_json);
    }
    return NULL;
}

void start_drone_server(int port) {
    int server_fd, drone_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", port);

    // Accept connections
    while (1) {
        if ((drone_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Drone connected.\n");

        pthread_mutex_lock(&drones_lock);
        Drone new_drone = { .drone_fd = drone_fd };
        drones[drone_count++] = new_drone;
        pthread_mutex_unlock(&drones_lock);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_drone, (void*)&drone_fd);
        pthread_detach(thread_id);
    }
}