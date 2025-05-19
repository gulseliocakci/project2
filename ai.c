#include "headers/ai.h"
#include "headers/globals.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "headers/view.h"

void* ai_controller(void* arg) {
    (void)arg;
    
    while (!should_quit) {
        if (!survivors || !drones) continue;

        // Survivor listesini kontrol et
        Node* survivor_node = survivors->head;
        while (survivor_node && survivor_node->occupied) {
            Survivor* s = (Survivor*)survivor_node->data;
            if (s && s->status == 0) { // Kurtarılmamış survivor
                // En yakın boşta olan drone'u bul
                Node* drone_node = drones->head;
                Drone* closest_drone = NULL;
                float min_distance = INFINITY;

                while (drone_node && drone_node->occupied) {
                    Drone* d = (Drone*)drone_node->data;
                    if (d && d->status == IDLE && d->battery_level > 20) {
                        float dist = sqrt(pow(d->coord.x - s->coord.x, 2) + 
                                       pow(d->coord.y - s->coord.y, 2));
                        if (dist < min_distance) {
                            min_distance = dist;
                            closest_drone = d;
                        }
                    }
                    drone_node = drone_node->next;
                }

                // Uygun drone bulunduysa görevi ata
                if (closest_drone) {
                    pthread_mutex_lock(&closest_drone->lock);
                    closest_drone->target = s->coord;
                    closest_drone->status = ON_MISSION;
                    pthread_cond_signal(&closest_drone->mission_cond);
                    pthread_mutex_unlock(&closest_drone->lock);
                }
            }
            survivor_node = survivor_node->next;
        }
        usleep(100000); // 100ms bekle
    }
    return NULL;
}