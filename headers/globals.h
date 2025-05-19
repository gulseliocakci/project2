#ifndef GLOBALS_H
#define GLOBALS_H

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

#endif

