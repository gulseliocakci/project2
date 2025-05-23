#ifndef AI_H
#define AI_H

#include "drone.h"
#include "survivor.h"
#include "coord.h"

int manhattan_distance(Coord a, Coord b);
Drone* find_closest_idle_drone(Coord target);
float calculate_distance(Coord a, Coord b);
int is_targeted(Survivor* survivor);

// AI kontrolcü fonksiyonları
void* ai_controller(void *args);
float calculate_distance(Coord a, Coord b);

// AI parametreleri
typedef struct {
    int min_battery_level;     // Minimum pil seviyesi için eşik değeri
    float max_mission_distance; // Maksimum görev mesafesi
    int rescue_priority;       // Kurtarma önceliği (1-5)
} AIParameters;

extern AIParameters ai_params;

#endif