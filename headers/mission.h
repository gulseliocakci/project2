#ifndef MISSION_H
#define MISSION_H

#include "coord.h"

typedef struct {
    int mission_id;
    int assigned_drone_id;
    Coord target;
} Mission;

#define MAX_MISSIONS 100

extern Mission mission_list[MAX_MISSIONS]; // Görev listesi
extern int mission_list_size;             // Görev listesi boyutu

#endif