#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <SDL2/SDL.h>
#include "headers/ai.h"
#include "headers/globals.h"
#include "headers/server.h"
#include "headers/mission.h"
#include "headers/view.h"
#include "headers/map.h"

#define _XOPEN_SOURCE 
#define _GNU_SOURCE

pthread_t visualization_thread, ai_thread, server_thread, survivor_generator_thread;

// Cleanup fonksiyonu
void cleanup(void) {
    printf("Starting cleanup...\n");
    should_quit = 1;

    pthread_cancel(visualization_thread);
    pthread_cancel(ai_thread);
    pthread_cancel(server_thread);
    pthread_cancel(survivor_generator_thread);

    pthread_join(visualization_thread, NULL);
    pthread_join(ai_thread, NULL);
    pthread_join(server_thread, NULL);
    pthread_join(survivor_generator_thread, NULL);
    
    cleanup_sdl();
    cleanup_drones();

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

// Sinyal işleme
void handle_signal(int sig) {
    printf("\nReceived signal %d, initiating shutdown...\n", sig);
    quit_program();
}

// Görselleştirme thread'i
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
    // Sinyal işleyicileri ayarla
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

    // Tarih ve zamanı ayrıştır
    int year, month, day, hour, min, sec;
    if (sscanf(date_time, "%d-%d-%d %d:%d:%d", 
               &year, &month, &day, &hour, &min, &sec) != 6) {
        fprintf(stderr, "Invalid date format. Use: YYYY-MM-DD HH:MM:SS\n");
        return 1;
    }

    // Unix timestamp'e çevir
    int days_since_epoch = (year - 1970) * 365 + day;
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (int i = 0; i < month - 1; i++) {
        days_since_epoch += days_in_month[i];
    }
    days_since_epoch += (year - 1969) / 4;

    current_time = (time_t)(
        days_since_epoch * 24 * 60 * 60 +  // günleri saniyeye çevir
        hour * 60 * 60 +                   // saatleri saniyeye çevir
        min * 60 +                         // dakikaları saniyeye çevir
        sec                                // saniyeleri ekle
    );

    // Kullanıcı adını kaydet
    strncpy(current_user, username, sizeof(current_user) - 1);
    current_user[sizeof(current_user) - 1] = '\0';

    // Başlangıç mesajlarını yazdır
    printf("Current Date and Time (UTC - YYYY-MM-DD HH:MM:SS formatted): %s\n", date_time);
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
    init_map(20, 20);
    
    // Drone'ları başlat
    initialize_drones();

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

    if (pthread_create(&server_thread, NULL, (void*)start_drone_server, NULL) != 0) {
        fprintf(stderr, "Failed to create server thread\n");
        cleanup();
        return 1;
    }

    if (pthread_create(&survivor_generator_thread, NULL, survivor_generator, NULL) != 0) {
        fprintf(stderr, "Failed to create survivor generator thread\n");
        cleanup();
        return 1;
    }

    printf("Server started. Waiting for drone connections...\n");

    // Ana döngü
    while (!should_quit) {
        sleep(1);
    }

    cleanup();
    return 0;
}