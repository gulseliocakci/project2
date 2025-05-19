#include "headers/drone.h"
#include "headers/globals.h"
#include "headers/survivor.h"
#include "headers/view.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void* drone_behavior(void* arg) {
    Drone* drone = (Drone*)arg;
    
    while (!should_quit) {
        pthread_mutex_lock(&drone->lock);
        while (drone->status == IDLE && !should_quit) {
            pthread_cond_wait(&drone->mission_cond, &drone->lock);
        }
        pthread_mutex_unlock(&drone->lock);

        if (should_quit) break;

        // Hedefe doğru hareket et
        while (drone->status == ON_MISSION && !should_quit) {
            pthread_mutex_lock(&drone->lock);
            
            // Pil seviyesini güncelle
            drone->battery_level -= 1;
            if (drone->battery_level <= 20) {
                drone->status = IDLE;
                pthread_mutex_unlock(&drone->lock);
                continue;
            }

            // Hedefe doğru hareket
            float dx = drone->target.x - drone->coord.x;
            float dy = drone->target.y - drone->coord.y;
            float distance = sqrt(dx*dx + dy*dy);

            if (distance < 0.1) { // Hedefe ulaşıldı
                // Survivor'ı güncelle
                Node* survivor_node = survivors->head;
                while (survivor_node && survivor_node->occupied) {
                    Survivor* s = (Survivor*)survivor_node->data;
                    if (s && s->coord.x == drone->target.x && 
                        s->coord.y == drone->target.y && 
                        (s->status == 0 || s->status == 2)) {  // Bekleyen veya atanmış survivor
                        s->status = 1; // Kurtarıldı
                        time_t current_time;
                        time(&current_time);
                        s->helped_time = *localtime(&current_time);
                        break;
                    }
                    survivor_node = survivor_node->next;
                }
                drone->status = IDLE;
            } else {
                // Drone hızına göre hareket
                float step = drone->speed * 0.1; // 100ms için
                if (step > distance) step = distance;
                float ratio = step / distance;
                
                drone->coord.x += dx * ratio;
                drone->coord.y += dy * ratio;
            }
            
            pthread_mutex_unlock(&drone->lock);
            usleep(100000); // 100ms bekle
        }
    }
    return NULL;
}

void initialize_drones(void) {
    for (int i = 0; i < 10; i++) {
        Drone* d = (Drone*)malloc(sizeof(Drone));
        d->id = i;
        d->status = IDLE;
        d->coord.x = rand() % 20;
        d->coord.y = rand() % 20;
        d->battery_level = 100;
        d->speed = 1.0 + (rand() % 100) / 100.0; // 1.0 ile 2.0 arası hız
        
        pthread_mutex_init(&d->lock, NULL);
        pthread_cond_init(&d->mission_cond, NULL);
        
        drones->add(drones, d);
        
        pthread_create(&d->thread_id, NULL, drone_behavior, d);
    }
}

void cleanup_drones(void) {
    Node* current = drones->head;
    while (current && current->occupied) {
        Drone* d = (Drone*)current->data;
        pthread_cancel(d->thread_id);
        pthread_join(d->thread_id, NULL);
        pthread_mutex_destroy(&d->lock);
        pthread_cond_destroy(&d->mission_cond);
        current = current->next;
    }
}

void assign_mission(Drone *d, Coord target) {
    if (!d) return;

    pthread_mutex_lock(&d->lock);
    
    // Drone'un durumunu ve hedefini güncelle
    if (d->status == IDLE && d->battery_level > 20) {
        d->target = target;
        d->status = ON_MISSION;
        
        // Görev zamanını güncelle
        time_t now;
        time(&now);
        d->last_update = *localtime(&now);
        
        // Drone thread'ini uyandır
        pthread_cond_signal(&d->mission_cond);
    }
    
    pthread_mutex_unlock(&d->lock);
}

int get_battery_level(Drone *d) {
    if (!d) return 0;
    
    pthread_mutex_lock(&d->lock);
    int level = d->battery_level;
    pthread_mutex_unlock(&d->lock);
    
    return level;
}
void set_drone_speed(Drone *d, float speed) {
    if (!d) return;
    
    pthread_mutex_lock(&d->lock);
    d->speed = speed;
    pthread_mutex_unlock(&d->lock);
}