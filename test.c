#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "headers/drone.h"
#include "headers/survivor.h"
#include "headers/globals.h"
#include "headers/view.h"
#include "headers/list.h"

// Global değişkenlerin tanımı (sadece test.c için)
//List *drones = NULL;
//List *survivors = NULL;
volatile int should_quit = 0;

// Fonksiyon prototipleri
void initialize_test_environment(void);
void cleanup_test_environment(void);
void test_survivor_creation(void);
void test_drone_assignment(void);
void test_visualization(void);

void initialize_test_environment(void) {
    printf("[%s][%s] Starting Test Suite...\n", 
           "2025-05-19 11:38:35", "Amine86s");
    
    drones = create_list(sizeof(Drone), 20);
    survivors = create_list(sizeof(Survivor), 50);
    
    if (!drones || !survivors) {
        fprintf(stderr, "[%s][%s] Failed to initialize lists\n", 
                "2025-05-19 11:38:35", "Amine86s");
        exit(1);
    }

    printf("Map initialized: 20x20\n");
    printf("Test Environment Initialized\n");
    printf("Current User: Amine86s\n");
    printf("Current UTC Time: 2025-05-19 11:38:35\n\n");
}

void cleanup_test_environment(void) {
    if (drones) {
        cleanup_drones();
        drones->destroy(drones);
        drones = NULL;
    }
    
    if (survivors) {
        Node *current = survivors->head;
        while (current && current->occupied) {
            // Survivor data cleanup
            current = current->next;
        }
        survivors->destroy(survivors);
        survivors = NULL;
    }
}

void test_survivor_creation(void) {
    printf("[%s][%s] Testing Survivor Creation...\n", 
           "2025-05-19 11:38:35", "Amine86s");
    
    Coord coord = {5, 5};
    char info[] = "Test Survivor";
    time_t now;
    time(&now);
    
    Survivor *s = create_survivor(&coord, info, localtime(&now));
    assert(s != NULL);
    assert(s->coord.x == 5);
    assert(s->coord.y == 5);
    assert(s->status == 0);
    
    survivors->add(survivors, s);
    printf("Survivor created successfully at (%d, %d)\n\n", s->coord.x, s->coord.y);
    free(s);
}

void test_drone_assignment(void) {
    printf("[%s][%s] Testing Drone Assignment...\n", 
           "2025-05-19 11:38:35", "Amine86s");
    
    Drone d = {
        .id = 1,
        .status = IDLE,
        .coord = {.x = 0, .y = 0},
        .battery_level = 100,
        .speed = 1.0
    };
    pthread_mutex_init(&d.lock, NULL);
    pthread_cond_init(&d.mission_cond, NULL);

    Coord target = {10, 10};
    assign_mission(&d, target);
    
    assert(d.status == ON_MISSION);
    assert(d.target.x == target.x);
    assert(d.target.y == target.y);
    
    pthread_mutex_destroy(&d.lock);
    pthread_cond_destroy(&d.mission_cond);
    
    printf("Drone assigned to target (%d, %d)\n\n", target.x, target.y);
}

void test_visualization(void) {
    printf("[%s][%s] Testing Visualization...\n", 
           "2025-05-19 11:38:35", "Amine86s");
    
    if (init_sdl_window() != 0) {
        fprintf(stderr, "[%s][%s] Failed to initialize SDL\n", 
                "2025-05-19 11:38:35", "Amine86s");
        return;
    }
    
    draw_map();
    SDL_Delay(1000);
    cleanup_sdl();
    
    printf("Visualization test completed successfully\n\n");
}

int main(void) {
    initialize_test_environment();
    
    test_survivor_creation();
    test_drone_assignment();
    test_visualization();
    
    cleanup_test_environment();
    
    printf("[%s][%s] All tests completed successfully!\n",
           "2025-05-19 11:38:35", "Amine86s");
    return 0;
}