#include "headers/ai.h"
#include "headers/drone.h"
#include "headers/globals.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "headers/view.h"

// Mesafe hesaplama fonksiyonu - mutlak değer kullanarak daha doğru hesaplama
float calculate_distance(Coord a, Coord b) {
    return fabs(a.x - b.x) + fabs(a.y - b.y);  // Manhattan mesafesi
}

// Survivor'ın başka bir drone tarafından hedeflenip hedeflenmediğini kontrol et
int is_targeted(Survivor* survivor) {
    if (!drones) return 0;
    
    Node* drone_node = drones->head;
    while (drone_node && drone_node->occupied) {
        Drone* d = (Drone*)drone_node->data;
        if (d && d->status == ON_MISSION) {
            if (d->target.x == survivor->coord.x && d->target.y == survivor->coord.y) {
                return 1;
            }
        }
        drone_node = drone_node->next;
    }
    return 0;
}

void* ai_controller(void* arg) {
    (void)arg;
    
    while (!should_quit) {
        if (!survivors || !drones) {
            usleep(100000);
            continue;
        }

        // Boşta olan drone'ları kontrol et
        Node* drone_node = drones->head;
        while (drone_node && drone_node->occupied) {
            Drone* d = (Drone*)drone_node->data;
            
            // Sadece boşta olan ve yeterli pili olan drone'ları işle
            if (d && d->status == IDLE && d->battery_level > 20) {
                Survivor* best_survivor = NULL;
                float min_distance = INFINITY;
                
                // Tüm survivor'ları kontrol et
                Node* survivor_node = survivors->head;
                while (survivor_node && survivor_node->occupied) {
                    Survivor* s = (Survivor*)survivor_node->data;
                    
                    // Sadece kurtarılmamış ve hedeflenmemiş survivor'ları değerlendir
                    if (s && s->status == 0 && !is_targeted(s)) {
                        float dist = calculate_distance(d->coord, s->coord);
                        
                        // En yakın survivor'ı bul
                        if (dist < min_distance) {
                            min_distance = dist;
                            best_survivor = s;
                        }
                    }
                    survivor_node = survivor_node->next;
                }
                
                // En yakın uygun survivor bulunduysa görevi ata
                if (best_survivor) {
                    pthread_mutex_lock(&d->lock);
                    d->target = best_survivor->coord;
                    d->status = ON_MISSION;
                    best_survivor->status = 2;  // Drone atandı
                    pthread_cond_signal(&d->mission_cond);
                    pthread_mutex_unlock(&d->lock);
                }
            }
            drone_node = drone_node->next;
        }
        
        usleep(100000); // 100ms bekle
    }
    return NULL;
}