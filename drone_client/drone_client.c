/* example drone_client.c 
* This is a simple drone client that connects to a server and sends its status
* while receiving navigation commands.
* You should separate the client implementation from server.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include "map.h" // Shared structures like Map, Coord

void start_drone_client(const char* server_ip, int port, const char* drone_id) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    while (1) {
        // Send status update
        struct json_object* json = json_object_new_object();
        json_object_object_add(json, "drone_id", json_object_new_string(drone_id));
        json_object_object_add(json, "status", json_object_new_string("idle"));
        struct json_object* location = json_object_new_array();
        json_object_array_add(location, json_object_new_int(rand() % 100));
        json_object_array_add(location, json_object_new_int(rand() % 100));
        json_object_object_add(json, "location", location);
        snprintf(buffer, sizeof(buffer), "%s", json_object_to_json_string(json));
        send(sock, buffer, strlen(buffer), 0);

        // Receive mission assignment
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(sock, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            printf("Mission received: %s\n", buffer);
        }

        json_object_put(json);
        sleep(5); // Send updates every 5 seconds
    }

    close(sock);
}
