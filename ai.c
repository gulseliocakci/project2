#include "headers/ai.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void assign_mission(Drone *drone, Coord target) {
    pthread_mutex_lock(&drone->lock);
    drone->target = target;
    drone->status = ON_MISSION;
    pthread_mutex_unlock(&drone->lock);
}

Drone *find_closest_idle_drone(Coord target) {
    Drone *closest = NULL;
    int min_distance = INT_MAX;
    pthread_mutex_lock(&drones->lock);  // Lock for drones list

    Node *node = drones->head;
    while (node != NULL) {
        Drone *d = (Drone *)node->data;
        if (d->status == IDLE) {
            int dist = abs(d->coord.x - target.x) +
                       abs(d->coord.y - target.y);
            if (dist < min_distance) {
                min_distance = dist;
                closest = d;
            }
        }
        node = node->next;
    }

    pthread_mutex_unlock(&drones->lock);  // Unlock for drones list
    return closest;
}

void *ai_controller(void *arg) {
    while (1) {
        Survivor *s = NULL;

        // Lock the survivors list first, then proceed with fetching a survivor
        pthread_mutex_lock(&survivors->lock);  // Lock for survivors list

        s = survivors->peek(survivors);
        if (s) {
            // We only need the survivor list locked when accessing the list itself
            pthread_mutex_unlock(&survivors->lock);  // Unlock survivors list immediately after peek

            // Now, find the closest idle drone (drones list locked inside this function)
            Drone *closest = find_closest_idle_drone(s->coord);
            if (closest) {
                // Lock the drone list to assign the mission and update status
                assign_mission(closest, s->coord);  // Drone lock happens inside this function

                printf("Drone %d assigned to survivor at (%d, %d)\n", closest->id, s->coord.x, s->coord.y);

                // Remove from the global survivor list
                pthread_mutex_lock(&survivors->lock);  // Lock again when modifying survivor list
                survivors->removedata(survivors, s);
                pthread_mutex_unlock(&survivors->lock);  // Unlock after modification

                // Mark as helped
                s->status = 1;  // Mark survivor as helped
                s->helped_time = s->discovery_time;

                printf("Survivor %s being helped by Drone %d\n", s->info, closest->id);
            }
        } else {
            pthread_mutex_unlock(&survivors->lock);  // Unlock if no survivor is available
        }

        sleep(1);  // Pause before checking for the next survivor
    }
    return NULL;
}
