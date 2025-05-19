#include "headers/survivor.h"
#include "headers/globals.h"
#include "headers/view.h"     // should_quit için
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>          // sleep fonksiyonu için

void* survivor_generator(void* arg) {
    (void)arg;
    
    while (!should_quit) {
        if (survivors->number_of_elements < survivors->capacity) {
            Coord coord = {
                .x = rand() % 20,
                .y = rand() % 20
            };
            
            char info[25];
            snprintf(info, sizeof(info), "Survivor_%d", 
                    survivors->number_of_elements + 1);
            
            time_t now;
            time(&now);
            
            Survivor* s = create_survivor(&coord, info, localtime(&now));
            if (s) {
                survivors->add(survivors, s);
            }
        }
        sleep(rand() % 5 + 5); // 5-10 saniye arası bekle
    }
    return NULL;
}

Survivor* create_survivor(Coord* coord, char* info, struct tm* discovery_time) {
    Survivor* s = (Survivor*)malloc(sizeof(Survivor));
    if (!s) return NULL;
    
    s->coord = *coord;
    s->status = 0; // Kurtarılmamış
    strncpy(s->info, info, sizeof(s->info) - 1);
    s->info[sizeof(s->info) - 1] = '\0';
    
    if (discovery_time) {
        s->discovery_time = *discovery_time;
    }
    
    return s;
}

void update_survivor_status(Survivor* s, int new_status) {
    s->status = new_status;
    if (new_status == 1) { // Kurtarıldı
        time_t now;
        time(&now);
        s->helped_time = *localtime(&now);
    }
}

int calculate_help_time(Survivor* s) {
    if (s->status != 1) return -1;
    
    time_t discovery = mktime(&s->discovery_time);
    time_t helped = mktime(&s->helped_time);
    
    return (int)difftime(helped, discovery);
}