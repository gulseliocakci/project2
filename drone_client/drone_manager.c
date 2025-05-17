#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#define MAX_DRONES 50
#define MAX_SURVIVORS 100

void start_drone_manager(); // start_drone_manager fonksiyonu prototipi

// Drone yapısı
typedef struct {
    int id;
    int x;
    int y;
    int active; // 0: Bağlantı yok, 1: Aktif
    pthread_mutex_t lock;
} Drone;

// Survivor yapısı
typedef struct {
    int id;
    int x;
    int y;
    int helped; // 0: Yardım edilmedi, 1: Yardım edildi
} Survivor;

// Global değişkenler
Drone drones[MAX_DRONES];
Survivor survivors[MAX_SURVIVORS];
int survivor_count = 0;

pthread_mutex_t drones_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t survivors_lock = PTHREAD_MUTEX_INITIALIZER;

// Drone'ları başlatma
void initialize_drones() {
    for (int i = 0; i < MAX_DRONES; i++) {
        drones[i].id = i;
        drones[i].x = 0;
        drones[i].y = 0;
        drones[i].active = 0; // Başlangıçta tüm drone'lar pasif
        pthread_mutex_init(&drones[i].lock, NULL);
    }
}

// En yakın drone'u bulma
int find_closest_drone(int x, int y) {
    int closest_drone = -1;
    double min_distance = __DBL_MAX__;

    pthread_mutex_lock(&drones_lock);
    for (int i = 0; i < MAX_DRONES; i++) {
        if (drones[i].active) {
            pthread_mutex_lock(&drones[i].lock);
            double distance = sqrt(pow(drones[i].x - x, 2) + pow(drones[i].y - y, 2));
            if (distance < min_distance) {
                min_distance = distance;
                closest_drone = i;
            }
            pthread_mutex_unlock(&drones[i].lock);
        }
    }
    pthread_mutex_unlock(&drones_lock);

    return closest_drone;
}

// Görevleri drone'lara atama
void assign_missions() {
    pthread_mutex_lock(&survivors_lock);
    for (int i = 0; i < survivor_count; i++) {
        if (!survivors[i].helped) { // Yardım edilmemiş survivor
            int closest_drone = find_closest_drone(survivors[i].x, survivors[i].y);
            if (closest_drone != -1) {
                pthread_mutex_lock(&drones[closest_drone].lock);
                drones[closest_drone].x = survivors[i].x;
                drones[closest_drone].y = survivors[i].y;
                survivors[i].helped = 1;
                pthread_mutex_unlock(&drones[closest_drone].lock);

                printf("Assigned Drone %d to Survivor %d at (%d, %d)\n",
                       closest_drone, survivors[i].id, survivors[i].x, survivors[i].y);
            }
        }
    }
    pthread_mutex_unlock(&survivors_lock);
}

// Drone Manager başlatma fonksiyonu
void start_drone_manager() {
    printf("Drone Manager started successfully!\n");

    // Drone'ları başlat
    initialize_drones();

    // Görev atama simülasyonu (sürekli çalışacak bir döngü)
    while (1) {
        assign_missions();
        sleep(5); // 5 saniye bekle
    }
}