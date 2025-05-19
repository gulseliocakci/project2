#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "headers/ai.h"
#include "headers/view.h"
#include "headers/map.h"
#include "headers/globals.h"

pthread_t visualization_thread, ai_thread;
volatile int should_quit = 0;

void cleanup(void) {
    printf("Starting cleanup...\n");
    should_quit = 1;

    pthread_cancel(visualization_thread);
    pthread_cancel(ai_thread);

    pthread_join(visualization_thread, NULL);
    pthread_join(ai_thread, NULL);
    
    cleanup_sdl();

    if (survivors) {
        survivors->destroy(survivors);
        survivors = NULL;
    }
    if (drones) {
        drones->destroy(drones);
        drones = NULL;
    }
    freemap();
}

void handle_signal(int sig) {
    printf("\nReceived signal %d, initiating shutdown...\n", sig);
    quit_program();
}

void *start_visualization(void *arg) {
    (void)arg;
    
    if (init_sdl_window() != 0) {
        fprintf(stderr, "Visualization initialization failed\n");
        should_quit = 1;
        return NULL;
    }

    printf("Visualization started successfully\n");
    
    while (!should_quit) {
        if (check_events() != 0) {
            break;
        }
        draw_map();
        SDL_Delay(16);
    }

    cleanup_sdl();
    return NULL;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Parametre kontrolü
    if (argc != 5) {
        fprintf(stderr, "Usage: %s --date <UTC_DATE_TIME> --user <USERNAME>\n", argv[0]);
        return 1;
    }

    char *date_time = NULL;
    char *username = NULL;

    // Parametreleri işle
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "--date") == 0) {
            date_time = argv[i + 1];
        } else if (strcmp(argv[i], "--user") == 0) {
            username = argv[i + 1];
        }
    }

    // Parametre kontrolü
    if (!date_time || !username) {
        fprintf(stderr, "Missing required parameters\n");
        return 1;
    }

    // Tarih formatı kontrolü (YYYY-MM-DD HH:MM:SS)
    struct tm tm;
    if (strptime(date_time, "%Y-%m-%d %H:%M:%S", &tm) == NULL) {
        fprintf(stderr, "Invalid date format. Use: YYYY-MM-DD HH:MM:SS\n");
        return 1;
    }

    // Başlangıç mesajlarını yazdır
    printf("Current Date and Time (UTC): %s\n", date_time);
    printf("Current User's Login: %s\n", username);

    // Listeleri oluştur
    survivors = create_list(sizeof(Survivor), 100);
    if (!survivors) {
        fprintf(stderr, "Failed to create survivors list\n");
        return 1;
    }

    drones = create_list(sizeof(Drone), 10);
    if (!drones) {
        fprintf(stderr, "Failed to create drones list\n");
        survivors->destroy(survivors);
        return 1;
    }

    // Haritayı başlat
    init_map(20, 20);  // void fonksiyon olduğu için kontrol etmiyoruz
    printf("Map initialized: 20x20\n");

    // Thread'leri oluştur
    if (pthread_create(&visualization_thread, NULL, start_visualization, NULL) != 0) {
        fprintf(stderr, "Failed to create visualization thread\n");
        cleanup();
        return 1;
    }

    if (pthread_create(&ai_thread, NULL, ai_controller, NULL) != 0) {
        fprintf(stderr, "Failed to create AI controller thread\n");
        cleanup();
        return 1;
    }

    // Ana döngü
    while (!should_quit) {
        sleep(1);
    }

    // Temiz bir şekilde kapat
    cleanup();
    return 0;
}