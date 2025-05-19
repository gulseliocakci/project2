#include "headers/drone.h"
#include "headers/globals.h"
#include "headers/map.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void* drone_behavior(void *arg) {
    Drone *drone = (Drone *)arg;
    struct timespec sleep_time = {0, 500000000}; // 500ms = 500,000,000 nanoseconds
    while (1) {
        pthread_mutex_lock(&drone->lock);
        
        while (drone->status == IDLE) {
            pthread_cond_wait(&drone->mission_cond, &drone->lock);
        }
        
        if (drone->status == ON_MISSION) {
            // Hedefe doğru hareket et
            int dx = drone->target.x - drone->coord.x;
            int dy = drone->target.y - drone->coord.y;
            
            if (dx != 0 || dy != 0) {
                // Basit hareket algoritması
                if (dx != 0) {
                    drone->coord.x += (dx > 0) ? 1 : -1;
                }
                if (dy != 0) {
                    drone->coord.y += (dy > 0) ? 1 : -1;
                }
                
                // Pil seviyesini azalt
                drone->battery_level = fmax(0, drone->battery_level - 1);
            } else {
                // Hedefe ulaşıldı
                drone->status = IDLE;
            }
        }
        
        pthread_mutex_unlock(&drone->lock);
        nanosleep(&sleep_time, NULL);
    }
    
    return NULL;
}

void cleanup_drones() {
    if (drone_fleet) {
        for (int i = 0; i < num_drones; i++) {
            pthread_mutex_destroy(&drone_fleet[i].lock);
            pthread_cond_destroy(&drone_fleet[i].mission_cond);
        }
        free(drone_fleet);
        drone_fleet = NULL;
    }
}

void initialize_drones() {
    // drone_fleet için bellek ayır
    drone_fleet = (Drone *)malloc(sizeof(Drone) * num_drones);
    if (!drone_fleet) {
        fprintf(stderr, "Failed to allocate memory for drone fleet\n");
        return;
    }

    // Her drone'u başlat
    for (int i = 0; i < num_drones; i++) {
        drone_fleet[i].id = i + 1;
        drone_fleet[i].status = IDLE;
        drone_fleet[i].battery_level = 100;
        drone_fleet[i].speed = 1.0;
        
        // Random başlangıç konumu ata
        drone_fleet[i].coord.x = rand() % map.width;
        drone_fleet[i].coord.y = rand() % map.height;
        drone_fleet[i].target = drone_fleet[i].coord;  // Başlangıçta hedef = mevcut konum
        
        // Thread sync nesnelerini başlat
        pthread_mutex_init(&drone_fleet[i].lock, NULL);
        pthread_cond_init(&drone_fleet[i].mission_cond, NULL);
        
        // Drone'u listeye ekle
        drones->add(drones, &drone_fleet[i]);
        
        // Drone thread'ini başlat
        pthread_create(&drone_fleet[i].thread_id, NULL, drone_behavior, &drone_fleet[i]);
    }
}

void assign_mission(Drone *d, Coord target) {
    pthread_mutex_lock(&d->lock);
    
    d->target = target;
    d->status = ON_MISSION;
    
    pthread_cond_signal(&d->mission_cond);
    pthread_mutex_unlock(&d->lock);
}

int get_battery_level(Drone *d) {
    int level;
    pthread_mutex_lock(&d->lock);
    level = d->battery_level;
    pthread_mutex_unlock(&d->lock);
    return level;
}

void set_drone_speed(Drone *d, float speed) {
    pthread_mutex_lock(&d->lock);
    d->speed = speed;
    pthread_mutex_unlock(&d->lock);
}

void* send_heartbeat(void* arg) {
    Drone *d = (Drone*)arg;
    
    while (1) {
        pthread_mutex_lock(&d->lock);
        time_t now;
        time(&now);
        localtime_r(&now, &d->last_update);
        pthread_mutex_unlock(&d->lock);
        
        sleep(1); // Her saniye heartbeat gönder
    }
    
    return NULL;
}