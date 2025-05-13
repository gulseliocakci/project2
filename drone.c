#include "headers/drone.h"
#include "headers/globals.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Global drone fleet
Drone *drone_fleet = NULL;
int num_drones = 10; // Default fleet size

void initialize_drones() {
    drone_fleet = malloc(sizeof(Drone) * num_drones);
    srand(time(NULL));

    for (int i = 0; i < num_drones; i++) {
        drone_fleet[i].id = i;
        drone_fleet[i].status = IDLE;
        drone_fleet[i].coord = (Coord){rand() % map.width, rand() % map.height};
        drone_fleet[i].target = drone_fleet[i].coord; // Initial target=current position
        pthread_mutex_init(&drone_fleet[i].lock, NULL);
        pthread_cond_init(&drone_fleet[i].mission_cond, NULL); // <- Her drone için condition variable
        
        pthread_mutex_lock(&drones->lock);
        drones->add(drones, &drone_fleet[i]); 
        pthread_mutex_unlock(&drones->lock);

        pthread_create(&drone_fleet[i].thread_id, NULL, drone_behavior, &drone_fleet[i]);
    }
}

void* drone_behavior(void *arg) {
    Drone *d = (Drone*)arg;

    while (1) {
        pthread_mutex_lock(&d->lock);

        while (d->status == IDLE) {
            // IDLE durumda ise condition variable ile bekle
            pthread_cond_wait(&d->mission_cond, &d->lock);
        }

        // Görev varsa hedefe doğru hareket et
        if (d->status == ON_MISSION) {
            if (d->coord.x < d->target.x) d->coord.x++;
            else if (d->coord.x > d->target.x) d->coord.x--;

            if (d->coord.y < d->target.y) d->coord.y++;
            else if (d->coord.y > d->target.y) d->coord.y--;

            // Hedefe ulaştıysa görevi tamamla
            if (d->coord.x == d->target.x && d->coord.y == d->target.y) {
                d->status = IDLE;
                printf("Drone %d: Mission completed!\n", d->id);
            }
        }

        pthread_mutex_unlock(&d->lock);
        sleep(1);
    }
    return NULL;
}

void assign_mission(Drone *d, Coord target) {
    pthread_mutex_lock(&d->lock);
    d->target = target;
    d->status = ON_MISSION;
    printf("Drone %d: New mission assigned to (%d,%d)\n", d->id, target.x, target.y);
    
    // Bu drone'u uyandır
    pthread_cond_signal(&d->mission_cond);
    pthread_mutex_unlock(&d->lock);
}

void cleanup_drones() {
    for (int i = 0; i < num_drones; i++) {
        pthread_cancel(drone_fleet[i].thread_id);
        pthread_mutex_destroy(&drone_fleet[i].lock);
        pthread_cond_destroy(&drone_fleet[i].mission_cond); // <- Condition variable temizliği
    }
    free(drone_fleet);
}
