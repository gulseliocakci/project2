#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

// Durum güncellemesi gönderen fonksiyon
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
    json_object_put(json_obj);
}

// Görev alıp hedefe ilerleyen fonksiyon
void handle_mission(int socket_fd) {
    char buffer[1024];
    while (1) {
        int bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            struct json_object *json_obj = json_tokener_parse(buffer);
            const char *type = json_object_get_string(json_object_object_get(json_obj, "type"));
            if (strcmp(type, "mission") == 0) {
                struct json_object *target = json_object_object_get(json_obj, "target");
                int x = json_object_get_int(json_object_array_get_idx(target, 0));
                int y = json_object_get_int(json_object_array_get_idx(target, 1));
                printf("Received mission: Move to (%d, %d)\n", x, y);

                int current_x = 0, current_y = 0; // Başlangıç noktası
                while (current_x != x || current_y != y) {
                    if (current_x < x) current_x++;
                    else if (current_x > x) current_x--;
                    if (current_y < y) current_y++;
                    else if (current_y > y) current_y--;

                    // Her adımda durum bildir
                    send_status_update(socket_fd, "D1", "on_mission", current_x, current_y);
                    sleep(1);
                }
                printf("Mission completed at (%d, %d)\n", x, y);
                send_status_update(socket_fd, "D1", "idle", x, y);
            }
            json_object_put(json_obj);
        }
    }
}

// README uyumlu başlatıcı fonksiyon
void start_drone_client() {
    printf("start_drone_client başlatıldı!\n");

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        return;
    }

    printf("Server'a bağlandı, görev bekleniyor...\n");

    // İlk başta idle durum bildirimi gönder
    send_status_update(sock, "D1", "idle", 0, 0);

    // Görevleri dinlemeye başla
    handle_mission(sock);

    close(sock);
}