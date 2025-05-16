#ifndef MAP_H
#define MAP_H

#include "survivor.h"
#include "list.h"
#include "coord.h"
/*
#define MAX_DRONES 100

// Drone dizisi olarak tanımlama
extern Drone drones[MAX_DRONES]; // Extern tanımı
extern int drone_count;          // Aktif drone sayısı
*/
extern int drone_count;
typedef struct mapcell {
    Coord coord;
    List *survivors;    // Survivors in this cell
} MapCell;

typedef struct map {
    int height, width;
    MapCell **cells;
} Map;


// Functions
void init_map(int height, int width);
void freemap();

#endif