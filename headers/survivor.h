#ifndef SURVIVOR_H
#define SURVIVOR_H

#include "coord.h"
#include <time.h>
#include "list.h"
typedef struct survivor {
    int status;
    Coord coord;
    struct tm discovery_time;
    struct tm helped_time;
    char info[25];
} Survivor;

// Functions
Survivor* create_survivor(Coord *coord, char *info, struct tm *discovery_time);
void *survivor_generator(void *args);
void update_survivor_status(Survivor *s, int new_status);
int calculate_help_time(Survivor *s);
void survivor_cleanup(Survivor *s);

#endif