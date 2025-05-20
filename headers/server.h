#ifndef SERVER_H
#define SERVER_H

#include "map.h" // Shared structures like Coord
#include "drone.h" // Drone yapısı burada tanımlanmıştır
#include "mission.h"
#include "list.h"
#include <time.h>
#include <pthread.h>

#define SERVER_PORT 8080
#define TIMEOUT_THRESHOLD 10
#define MAX_MISSED_HEARTBEATS 3
#define MAX_DRONES 100

// Yeni eklenen fonksiyonlar için tanımlamalar
void handle_heartbeat(int drone_id);
void check_drone_timeouts();
void* monitor_heartbeats(void* arg);
void broadcast_to_drones(const char* message);

// Function declarations
void handle_disconnected_drone(int disconnected_drone_id);
void* handle_drone(void* arg);
void start_drone_server(void);

#endif