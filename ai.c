#include "headers/ai.h"
#include "headers/globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

// Yardımcı fonksiyonlar
Drone* find_closest_idle_drone(Coord target) {
    Drone *closest = NULL;
    double min_distance = -1;

    pthread_mutex_lock(&drones->lock);
    Node *curr = drones->head;
    
    while (curr != NULL) {
        Drone *d = (Drone *)curr->data;
        pthread_mutex_lock(&d->lock);
        
        if (d->status == IDLE) {
            double dist = sqrt(pow(d->coord.x - target.x, 2) + 
                             pow(d->coord.y - target.y, 2));
            
            if (min_distance < 0 || dist < min_distance) {
                min_distance = dist;
                closest = d;
            }
        }
        
        pthread_mutex_unlock(&d->lock);
        curr = curr->next;
    }
    
    pthread_mutex_unlock(&drones->lock);
    return closest;
}

void *ai_controller(void *arg) {
    (void)arg;  // Kullanılmayan parametre uyarısını engelle
    
    while (1) {
        Survivor *s = NULL;

        pthread_mutex_lock(&survivors->lock);
        s = survivors->peek(survivors);
        
        if (s) {
            pthread_mutex_unlock(&survivors->lock);

            // En yakın boştaki drone'u bul
            Drone *closest = find_closest_idle_drone(s->coord);
            if (closest) {
                // Görevi drone'a ata
                assign_mission(closest, s->coord);

                printf("Drone %d assigned to survivor at (%d, %d)\n", 
                       closest->id, s->coord.x, s->coord.y);

                pthread_mutex_lock(&survivors->lock);
                survivors->removedata(survivors, s);
                update_survivor_status(s, 1);
                pthread_mutex_unlock(&survivors->lock);

                printf("Survivor %s being helped by Drone %d\n", s->info, closest->id);
            }
        } else {
            pthread_mutex_unlock(&survivors->lock);
        }

        sleep(1);
    }
    return NULL;
}