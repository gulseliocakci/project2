#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

// JSON library (e.g., json-c) for JSON parsing
#include <json-c/json.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

void send_status_update(int socket_fd, const char* drone_id, const char* status, int x, int y) {
    struct json_object *json_obj = json_object_new_object();
    json_object_object_add(json_obj, "drone_id", json_object_new_string(drone_id));
    json_object_object_add(json_obj, "status", json_object_new_string(status));
    
    struct json_object *location = json_object_new_array();
    json_object_array_add(location, json_object_new_int(x));
    json_object_array_add(location, json_object_new_int(y));
    json_object_object_add(json_obj, "location", location);

    const char *json_str = json_object_to_json_string(json_obj);
    send(socket_fd, json_str, strlen(json_str), 0);

    json_object_put(json_obj); // Free JSON object
}

void* drone_client_thread(void* arg) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(socket_fd);
        return NULL;
    }

    printf("Connected to server as drone client.\n");

    // Example: Sending status updates
    send_status_update(socket_fd, "D1", "idle", 10, 20);

    // Close connection
    close(socket_fd);
    return NULL;
}

int main() {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, drone_client_thread, NULL);
    pthread_join(thread_id, NULL);

    return 0;
}