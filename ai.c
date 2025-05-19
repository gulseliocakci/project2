#include "headers/ai.h"
#include "headers/drone.h"
#include "headers/globals.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "headers/view.h"

// Yardımcı fonksiyon: İki koordinat arasındaki mesafeyi hesaplar
float calculate_distance(Coord a, Coord b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

void* ai_controller(void* arg) {
    (void)arg;
    
    while (!should_quit) {
        if (!survivors || !drones) {
            usleep(100000);
            continue;
        }

        // Önce tüm kurtarılmamış survivor'ları bul
        Node* waiting_survivors[1000];  // Max survivor sayısı kadar
        int survivor_count = 0;
        
        Node* survivor_node = survivors->head;
        while (survivor_node && survivor_node->occupied) {
            Survivor* s = (Survivor*)survivor_node->data;
            if (s && s->status == 0) { // Sadece bekleyen survivor'lar
                waiting_survivors[survivor_count++] = survivor_node;
            }
            survivor_node = survivor_node->next;
        }

        // Her boşta olan drone için
        Node* drone_node = drones->head;
        while (drone_node && drone_node->occupied) {
            Drone* d = (Drone*)drone_node->data;
            
            if (d && d->status == IDLE && d->battery_level > 20) {
                float min_distance = INFINITY;
                Survivor* closest_survivor = NULL;
                Node* closest_node = NULL;
                
                // Bu drone için gerçekten en yakın survivor'ı bul
                for (int i = 0; i < survivor_count; i++) {
                    Survivor* s = (Survivor*)waiting_survivors[i]->data;
                    float dist = calculate_distance(d->coord, s->coord);
                    
                    if (dist < min_distance) {
                        min_distance = dist;
                        closest_survivor = s;
                        closest_node = waiting_survivors[i];
                    }
                }
                
                // En yakın survivor'a görevi ata ve listeden çıkar
                if (closest_survivor) {
                    closest_survivor->status = 2;  // Drone atandı
                    pthread_mutex_lock(&d->lock);
                    d->target = closest_survivor->coord;
                    d->status = ON_MISSION;
                    pthread_cond_signal(&d->mission_cond);
                    pthread_mutex_unlock(&d->lock);
                    
                    // Bu survivor'ı listeden çıkar
                    for (int i = 0; i < survivor_count; i++) {
                        if (waiting_survivors[i] == closest_node) {
                            for (int j = i; j < survivor_count - 1; j++) {
                                waiting_survivors[j] = waiting_survivors[j + 1];
                            }
                            survivor_count--;
                            break;
                        }
                    }
                }
            }
            drone_node = drone_node->next;
        }
        
        usleep(100000); // 100ms bekle
    }
    return NULL;
}