#ifndef SERVER_H
#define SERVER_H

#include "map.h" // Shared structures like Coord

#define MAX_DRONES 100

// Drone structure definition
typedef struct {
    int drone_fd;
    char drone_id[16];
    char status[16];
    Coord location;
} Drone;

// Function declarations
void* handle_drone(void* arg);
void start_drone_server(int port);

#endif