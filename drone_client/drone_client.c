#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <json-c/json.h>
#include <errno.h>  // Hata detayları için eklendi
#include "headers/drone.h"
#include "headers/coord.h"

#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"

// Global değişkenler
Drone current_drone;
pthread_mutex_t drone_lock = PTHREAD_MUTEX_INITIALIZER;

void start_drone_client(int *sock_ptr, int drone_id) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    // Soket oluştur
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error: %s\n", strerror(errno));
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    memset(&serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));  // Eklendi
    
    // IP adresini dönüştür
    if(inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported: %s\n", strerror(errno));
        return;
    }

    printf("Trying to connect to %s:%d...\n", SERVER_IP, SERVER_PORT);  // Debug mesajı

    // Sunucuya bağlan
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed: %s\n", strerror(errno));
        printf("Make sure the server (drone_simulator) is running.\n");
        return;
    }

    printf("Connected to server successfully\n");

    // Drone verilerini hazırla
    pthread_mutex_lock(&drone_lock);
    current_drone.id = rand() % 10;
    current_drone.status = IDLE;
    current_drone.coord.x = rand() % 20;
    current_drone.coord.y = rand() % 20;
    current_drone.battery_level = 100;
    current_drone.speed = 1.0;
    pthread_mutex_unlock(&drone_lock);

    // JSON mesajını oluştur
    struct json_object *jobj = json_object_new_object();
    json_object_object_add(jobj, "type", json_object_new_string("HANDSHAKE"));
    json_object_object_add(jobj, "drone_id", json_object_new_int( current_drone.id));
    json_object_object_add(jobj, "x", json_object_new_int(current_drone.coord.x));
    json_object_object_add(jobj, "y", json_object_new_int(current_drone.coord.y));
    
    // Sunucuya gönder
    const char *json_str = json_object_to_json_string(jobj);
    ssize_t sent_bytes = send(sock, json_str, strlen(json_str), 0);
    if (sent_bytes < 0) {
        printf("Failed to send handshake: %s\n", strerror(errno));
    } else {
        printf("Sent handshake message: %s\n", json_str);
    }
    json_object_put(jobj);

    *sock_ptr = sock;

    // Ana mesaj döngüsü
    while(1) {
        char buffer[1024] = {0};
        int valread = read(sock, buffer, 1024);
        if (valread < 0) {
            printf("Read error: %s\n", strerror(errno));
            break;
        } else if (valread == 0) {
            printf("Server disconnected\n");
            break;
        } else if (valread > 0) {
            printf("Received from server: %s\n", buffer);
            
            // JSON mesajını parse et
            struct json_object *received = json_tokener_parse(buffer);
            if (received) {
                struct json_object *type_obj;
                if (json_object_object_get_ex(received, "type", &type_obj)) {
                    const char *msg_type = json_object_get_string(type_obj);
                    
                    if (strcmp(msg_type, "MISSION") == 0) {
                        // Görev mesajını işle
                        struct json_object *x_obj, *y_obj;
                        if (json_object_object_get_ex(received, "target_x", &x_obj) &&
                            json_object_object_get_ex(received, "target_y", &y_obj)) {
                            
                            pthread_mutex_lock(&drone_lock);
                            current_drone.target.x = json_object_get_int(x_obj);
                            current_drone.target.y = json_object_get_int(y_obj);
                            current_drone.status = ON_MISSION;
                            pthread_mutex_unlock(&drone_lock);
                            
                            printf("Received mission: Move to (%d,%d)\n", 
                                   current_drone.target.x, current_drone.target.y);
                        }
                    }
                }
                json_object_put(received);
            }
        }
        usleep(100000); // 100ms bekle
    }

    close(sock);
}