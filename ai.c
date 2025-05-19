#include "headers/ai.h"
#include "headers/drone.h"
#include "headers/globals.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "headers/view.h"

typedef struct {
    Drone* drone;
    Survivor* survivor;
    float distance;
} Assignment;

float calculate_distance(Coord a, Coord b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

int compare_assignments(const void* a, const void* b) {
    const Assignment* ass_a = (const Assignment*)a;
    const Assignment* ass_b = (const Assignment*)b;
    if (ass_a->distance < ass_b->distance) return -1;
    if (ass_a->distance > ass_b->distance) return 1;
    return 0;
}

void* ai_controller(void* arg) {
    (void)arg;
    
    while (!should_quit) {
        if (!survivors || !drones) {
            usleep(100000);
            continue;
        }

        Assignment assignments[100];
        int assignment_count = 0;

        // Tüm olası drone-survivor kombinasyonlarını ve mesafelerini hesapla
        Node* drone_node = drones->head;
        while (drone_node && drone_node->occupied) {
            Drone* d = (Drone*)drone_node->data;
            if (d && d->status == IDLE && d->battery_level > 20) {
                Node* survivor_node = survivors->head;
                while (survivor_node && survivor_node->occupied) {
                    Survivor* s = (Survivor*)survivor_node->data;
                    if (s && s->status == 0) { // Kurtarılmamış survivor
                        assignments[assignment_count].drone = d;
                        assignments[assignment_count].survivor = s;
                        assignments[assignment_count].distance = 
                            calculate_distance(d->coord, s->coord);
                        assignment_count++;
                    }
                    survivor_node = survivor_node->next;
                }
            }
            drone_node = drone_node->next;
        }

        // Mesafeye göre sırala
        qsort(assignments, assignment_count, sizeof(Assignment), compare_assignments);

        // Atanmış survivor ve drone'ları takip etmek için
        int assigned_survivors[100] = {0};
        int assigned_drones[100] = {0};

        // En kısa mesafeli eşleşmeleri yap
        for (int i = 0; i < assignment_count; i++) {
            Drone* d = assignments[i].drone;
            Survivor* s = assignments[i].survivor;
            
            // Bu drone veya survivor zaten atanmış mı kontrol et
            int drone_id = d->id;
            int survivor_id = 0;
            Node* temp = survivors->head;
            while (temp && temp->occupied) {
                // Type casting ile düzgün karşılaştırma
                if ((Survivor*)temp->data == s) break;
                survivor_id++;
                temp = temp->next;
            }

            if (!assigned_drones[drone_id] && !assigned_survivors[survivor_id]) {
                // Drone'u survivor'a ata
                pthread_mutex_lock(&d->lock);
                d->target = s->coord;
                d->status = ON_MISSION;
                s->status = 2; // Drone atandı
                printf("AI: Drone %d -> Survivor (%d, %d), Mesafe: %.2f\n", 
                d->id, s->coord.x, s->coord.y, assignments[i].distance);
                pthread_cond_signal(&d->mission_cond);
                pthread_mutex_unlock(&d->lock);

                 assigned_drones[drone_id] = 1;
                 assigned_survivors[survivor_id] = 1;
                }
        }

        usleep(100000); // 100ms bekle
    }
    return NULL;
}