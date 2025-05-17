#include <stdio.h>
#include <pthread.h>

// Fonksiyon prototipleri
void start_drone_client();
void start_drone_manager();

int main() {
    pthread_t manager_thread, client_thread;

    // Drone Manager (Sunucu) başlat
    pthread_create(&manager_thread, NULL, (void*)start_drone_manager, NULL);

    // Drone Client (İstemci) başlat
    pthread_create(&client_thread, NULL, (void*)start_drone_client, NULL);

    // Thread'lerin tamamlanmasını bekle
    pthread_join(manager_thread, NULL);
    pthread_join(client_thread, NULL);

    return 0;
}