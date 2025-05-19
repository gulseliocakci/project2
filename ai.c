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

        // Her boşta olan drone için en yakın survivor'ı bul
        Node* drone_node = drones->head;
        while (drone_node && drone_node->occupied) {
            Drone* d = (Drone*)drone_node->data;
            if (d && d->status == IDLE && d->battery_level > 20) {
                // En yakın kurtarılmamış survivor'ı bul
                Node* survivor_node = survivors->head;
                Survivor* closest_survivor = NULL;
                float min_distance = INFINITY;

                while (survivor_node && survivor_node->occupied) {
                    Survivor* s = (Survivor*)survivor_node->data;
                    if (s && s->status == 0) { // Kurtarılmamış survivor
                        float dist = sqrt(pow(d->coord.x - s->coord.x, 2) + 
                                       pow(d->coord.y - s->coord.y, 2));
                        if (dist < min_distance) {
                            min_distance = dist;
                            closest_survivor = s;
                        }
                    }
                    survivor_node = survivor_node->next;
                }

                // Uygun survivor bulunduysa görevi ata
                if (closest_survivor) {
                    pthread_mutex_lock(&d->lock);
                    d->target = closest_survivor->coord;
                    d->status = ON_MISSION;
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