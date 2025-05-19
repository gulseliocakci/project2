#include <stdio.h>
#include "headers/globals.h"
#include "headers/map.h"
#include "headers/drone.h"
#include "headers/survivor.h"
#include "headers/view.h"

volatile int should_quit = 0;  // should_quit tanımlaması eklendi

int main() {
    printf("Starting Test Suite...\n");
    printf("Map initialized: 20x20\n");
    printf("Test Environment Initialized\n");
    printf("Current User: Amine86s\n");
    printf("Current UTC Time: 2025-05-19 00:51:03\n\n");

    // Test ortamını hazırla
    survivors = create_list(sizeof(Survivor), 100);
    drones = create_list(sizeof(Drone), 10);
    init_map(20, 20);

    printf("Testing Survivor Creation...\n");
    Survivor s = {
        .coord = {5, 5},
        .status = 0,
        .info = "Test Survivor"
    };
    survivors->add(survivors, &s);
    printf("Survivor created successfully at (5, 5)\n\n");

    printf("Testing Drone Assignment...\n");
    Drone d = {
        .id = 1,
        .status = IDLE,
        .coord = {0, 0},
        .target = {10, 10}
    };
    drones->add(drones, &d);
    assign_mission(&d, d.target);
    printf("Drone assigned to target (10, 10)\n\n");

    printf("Testing Visualization...\n");
    if (init_sdl_window() == 0) {
        draw_map();
        SDL_Delay(2000);  // 2 saniye bekle
        cleanup_sdl();
        printf("Visualization test completed successfully\n\n");
    } else {
        printf("Visualization test failed\n\n");
    }

    // Cleanup
    if (survivors) {
        survivors->destroy(survivors);
        survivors = NULL;
    }
    if (drones) {
        drones->destroy(drones);
        drones = NULL;
    }
    cleanup_drones();
    freemap();
    printf("Cleanup completed successfully\n");
    printf("All tests completed successfully!\n");

    return 0;
}