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
    int battery_level;          // Pil seviyesi (0-100)
    float speed;         // drone hizi 
} Drone;

// Drone yönetim fonksiyonları
void* drone_behavior(void *arg);
void cleanup_drones(void);
void initialize_drones(void);
void assign_mission(Drone *d, Coord target);
int get_battery_level(Drone *d);
void set_drone_speed(Drone *d, float speed);
void* send_heartbeat(void *arg);

#endif