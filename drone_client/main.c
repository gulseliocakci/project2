#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <json-c/json.h>
#include <sys/socket.h>
#include "headers/drone.h"

// Fonksiyon prototipleri
void start_drone_client(int *sock_ptr, int drone_id);
void* send_heartbeat(void* arg);

typedef struct {
    int sock;
    int drone_id;
} HeartbeatArgs;

int main(int argc, char *argv[]) {
    // Rastgele sayı üreteci için seed
    srand(time(NULL));
    
    pthread_t heartbeat_thread;
    int sock;
    int drone_id = rand() % 1000; // Rastgele drone ID

    // Drone Client (İstemci) başlat
    start_drone_client(&sock, drone_id);

    // Heartbeat thread'ini başlat
    HeartbeatArgs *args = malloc(sizeof(HeartbeatArgs));
    args->sock = sock;
    args->drone_id = drone_id;
    pthread_create(&heartbeat_thread, NULL, send_heartbeat, args);

    // Thread'in tamamlanmasını bekle
    pthread_join(heartbeat_thread, NULL);
    
    return 0;
}

void* send_heartbeat(void* arg) {
    HeartbeatArgs *args = (HeartbeatArgs*)arg;
    while(1) {
        // JSON ile heartbeat mesajı oluştur
        struct json_object *jobj = json_object_new_object();
        json_object_object_add(jobj, "type", json_object_new_string("HEARTBEAT"));
        json_object_object_add(jobj, "drone_id", json_object_new_int(args->drone_id));
        
        // Sunucuya gönder
        const char *json_str = json_object_to_json_string(jobj);
        send(args->sock, json_str, strlen(json_str), 0);
        
        // JSON objesini temizle
        json_object_put(jobj);
        
        sleep(1); // Her saniye heartbeat gönder
    }
    free(arg);
    return NULL;
}