#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include <pthread.h>

#define NUM_DRONES 50
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

void* simulate_drone(void* arg) {
    int drone_id = *(int*)arg;
    free(arg);

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Drone %d: Socket creation error\n", drone_id);
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("Drone %d: Invalid address / Address not supported\n", drone_id);
        close(sock);
        return NULL;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Drone %d: Connection Failed\n", drone_id);
        close(sock);
        return NULL;
    }

    printf("Drone %d: Connected to server\n", drone_id);

    while (1) {
        struct json_object* json = json_object_new_object();
        json_object_object_add(json, "drone_id", json_object_new_int(drone_id));
        json_object_object_add(json, "status", json_object_new_string("active"));

        struct json_object* location = json_object_new_array();
        json_object_array_add(location, json_object_new_int(rand() % 100));
        json_object_array_add(location, json_object_new_int(rand() % 100));
        json_object_object_add(json, "location", location);

        const char* json_str = json_object_to_json_string(json);
        send(sock, json_str, strlen(json_str), 0);
        printf("Drone %d: Sent data: %s\n", drone_id, json_str);

        json_object_put(json);

        sleep(1);  // Send data every second
    }

    close(sock);
    return NULL;
}

int main() {
    pthread_t threads[NUM_DRONES];

    for (int i = 0; i < NUM_DRONES; i++) {
        int* drone_id = malloc(sizeof(int));
        *drone_id = i + 1;
        pthread_create(&threads[i], NULL, simulate_drone, drone_id);
    }

    for (int i = 0; i < NUM_DRONES; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}