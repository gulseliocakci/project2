#ifndef GLOBALS_H
#define GLOBALS_H

#include <time.h>
#include <stdbool.h>

#include "map.h"
#include "drone.h"
#include "survivor.h"
#include "list.h"
#include "coord.h"

extern List *survivors, *helpedsurvivors, *drones;
extern Map map;
extern Drone *drone_fleet; // Global drone fleet
extern int num_drones; // Drone sayısı
extern volatile int should_quit;
extern time_t current_time;  // Global zaman değişkeni
extern char current_user[256];  // Mevcut kullanıcı


#endif

