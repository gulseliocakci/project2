#ifndef DRONE_H
#define DRONE_H

#include "coord.h"
#include <time.h>
#include <pthread.h>
#include "list.h"

typedef enum {
    IDLE,
    ON_MISSION,
    DISCONNECTED
} DroneStatus;

typedef struct drone {
    int id;                     // Drone ID
    int drone_fd;               // Drone'un socket bağlantısı
    DroneStatus status;         // Drone'un durumu (IDLE, ON_MISSION, DISCONNECTED)
    Coord coord;                // Mevcut konum
    Coord target;               // Hedef konum
    pthread_t thread_id; 
    struct tm last_update;      // Son güncelleme zamanı
    pthread_mutex_t lock;       // Drone için mutex
    pthread_cond_t mission_cond;// Görev beklemek için condition variable
} Drone;

// Yeni eklenen fonksiyonlar için tanımlamalar
void* send_heartbeat(void* arg); // Heartbeat gönderimi
void assign_mission(Drone *d, Coord target);
void cleanup_drones();

// Functions
void initialize_drones();
void* drone_behavior(void *arg);

#endif