#ifndef SERVER_H
#define SERVER_H

#include "map.h" // Shared structures like Coord
#include "drone.h" // Drone yapısı burada tanımlanmıştır

#define MAX_DRONES 100

// Function declarations
void handle_disconnected_drone(int disconnected_drone_id);
void* handle_drone(void* arg);
void start_drone_server(int port);

#endif