#include "headers/globals.h"


time_t current_time;  // Global zaman değişkeni
char current_user[256];  // Mevcut kullanıcı


List *survivors = NULL;
List *helpedsurvivors = NULL;
List *drones = NULL;
Map map;
Drone *drone_fleet = NULL;
int num_drones = 0;
volatile int should_quit = 0;  
