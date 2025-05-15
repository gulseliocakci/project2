#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "headers/ai.h"
#include "headers/view.h"
#include "headers/map.h"

// Görselleştirme işlevi (view.c'den)
void *start_visualization(void *arg) {
    if (init_sdl_window() != 0) {
        fprintf(stderr, "Görselleştirme başlatılamadı.\n");
        return NULL;
    }

    while (!quit_all()) {
        draw_map();
        draw_drones();
        draw_survivors();
        SDL_Delay(100); // 100ms gecikme
    }

    return NULL;
}

int main() {
    // Sunucu portu
    int port = 8080;

    // Haritayı başlat
    init_map(20, 20); // Örnek boyutlar: 20x20

    // Görselleştirme iş parçacığı oluştur
    pthread_t visualization_thread;
    if (pthread_create(&visualization_thread, NULL, start_visualization, NULL) != 0) {
        fprintf(stderr, "Görselleştirme iş parçacığı başlatılamadı.\n");
        return 1;
    }

    // Yapay zeka kontrol iş parçacığı oluştur
    pthread_t ai_thread;
    if (pthread_create(&ai_thread, NULL, ai_controller, NULL) != 0) {
        fprintf(stderr, "AI iş parçacığı başlatılamadı.\n");
        return 1;
    }

    // İş parçacıklarının tamamlanmasını bekle
    pthread_join(visualization_thread, NULL);
    pthread_join(ai_thread, NULL);

    // Haritayı serbest bırak
    freemap();

    return 0;
}