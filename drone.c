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

// Initialize condition variable and mutex
pthread_cond_t mission_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mission_lock = PTHREAD_MUTEX_INITIALIZER;

void initialize_drones() {
    drone_fleet = malloc(sizeof(Drone) * num_drones);
    srand(time(NULL));

    for (int i = 0; i < num_drones; i++) {
        drone_fleet[i].id = i;
        drone_fleet[i].status = IDLE;
        drone_fleet[i].coord = (Coord){rand() % map.width, rand() % map.height};
        drone_fleet[i].target = drone_fleet[i].coord; // Initial target=current position
        pthread_mutex_init(&drone_fleet[i].lock, NULL);
        
        // Add to global drone list
        pthread_mutex_lock(&drones->lock);
        drones->add(drones, &drone_fleet[i]); 
        pthread_mutex_unlock(&drones->lock);

        // Create thread
        pthread_create(&drone_fleet[i].thread_id, NULL, drone_behavior, &drone_fleet[i]);
    }
}

// Drone behavior (updated for waiting when idle)
void* drone_behavior(void *arg) {
    Drone *d = (Drone*)arg;

    while (1) {
        pthread_mutex_lock(&d->lock);

        // If the drone is on a mission, move toward the target
        if (d->status == ON_MISSION) {
            // Move toward target (1 cell per iteration)
            if (d->coord.x < d->target.x) d->coord.x++;
            else if (d->coord.x > d->target.x) d->coord.x--;

            if (d->coord.y < d->target.y) d->coord.y++;
            else if (d->coord.y > d->target.y) d->coord.y--;

            // Check mission completion
            if (d->coord.x == d->target.x && d->coord.y == d->target.y) {
                d->status = IDLE;
                printf("Drone %d: Mission completed!\n", d->id);
            }
        }

        // If the drone is idle, wait for a new mission
        if (d->status == IDLE) {
            // Wait until a mission is assigned (using condition variable)
            pthread_cond_wait(&mission_condition, &d->lock);
        }

        pthread_mutex_unlock(&d->lock);
        sleep(1); // Update every second
    }
    return NULL;
}

// Assign mission to a drone (this function is assumed to exist or can be written as part of Phase-2)
void assign_mission(Drone *d, Coord target) {
    pthread_mutex_lock(&d->lock);
    d->target = target;
    d->status = ON_MISSION;
    printf("Drone %d: New mission assigned, target at (%d, %d)\n", d->id, target.x, target.y);
    
    // Signal the drone to wake up and start the mission
    pthread_cond_signal(&mission_condition);
    pthread_mutex_unlock(&d->lock);
}

void cleanup_drones() {
    for (int i = 0; i < num_drones; i++) {
        pthread_cancel(drone_fleet[i].thread_id);
        pthread_mutex_destroy(&drone_fleet[i].lock);
    }
    free(drone_fleet);
}
