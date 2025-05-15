#include <stdio.h>
#include "headers/drone.h"
#include "headers/list.h"
#include "headers/view.h"
#include <unistd.h>

int main() {
    // SDL Penceresini başlat
    if (init_sdl_window() != 0) {
        fprintf(stderr, "SDL başlatılamadı.\n");
        return 1;
    }

    // Drone'ları başlat
    initialize_drones();

    // Ana döngü
    while (!check_events()) {
        draw_map();
        sleep(1); // Simülasyon hızı
    }

    // Temizlik işlemleri
    quit_all();
    return 0;
}