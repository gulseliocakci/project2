#include "headers/survivor.h"
#include "headers/globals.h"
#include "headers/map.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Survivor yaratma fonksiyonu
Survivor *create_survivor(Coord *coord, char *info, struct tm *discovery_time) {
    Survivor *s = malloc(sizeof(Survivor));
    if (!s) return NULL;

    memset(s, 0, sizeof(Survivor));
    s->coord = *coord;
    memcpy(&s->discovery_time, discovery_time, sizeof(struct tm));
    strncpy(s->info, info, sizeof(s->info) - 1);
    s->info[sizeof(s->info) - 1] = '\0';  // Null-terminate the string
    s->status = 0;  // Status: 0 = waiting
    return s;
}

// Survivor üretme fonksiyonu (rastgele üretim)
void *survivor_generator(void *args) {
    (void)args;  // Unused parameter
    time_t t;
    struct tm discovery_time;

    while (1) {
        // Rastgele survivor konumu üretme
        Coord coord = {.x = rand() % map.width, .y = rand() % map.height};

        char info[25];
        snprintf(info, sizeof(info), "SURV-%04d", rand() % 10000);

        time(&t);
        localtime_r(&t, &discovery_time);

        // Survivor oluşturma
        Survivor *s = create_survivor(&coord, info, &discovery_time);
        if (!s) continue;

        // Global survivor listesine ekle
        pthread_mutex_lock(&survivors->lock);
        survivors->add(survivors, s);  // Survivor'ı listeye ekle
        pthread_mutex_unlock(&survivors->lock);

        // Harita hücresine survivor ekle
        pthread_mutex_lock(&map.cells[coord.x][coord.y].survivors->lock);
        map.cells[coord.x][coord.y].survivors->add(map.cells[coord.x][coord.y].survivors, s);
        pthread_mutex_unlock(&map.cells[coord.x][coord.y].survivors->lock);

        printf("New survivor at (%d,%d): %s\n", coord.x, coord.y, info);

        // Survivor'ı ekledikten sonra 2-5 saniye arası rastgele bekle
        sleep(rand() % 3 + 2);
    }
    return NULL;
}

// Survivor temizleme fonksiyonu
void survivor_cleanup(Survivor *s) {
    // Harita hücresinden çıkar
    pthread_mutex_lock(&map.cells[s->coord.x][s->coord.y].survivors->lock);
    map.cells[s->coord.x][s->coord.y].survivors->removedata(map.cells[s->coord.x][s->coord.y].survivors, s);
    pthread_mutex_unlock(&map.cells[s->coord.x][s->coord.y].survivors->lock);

    // Global survivor listesine çıkar
    pthread_mutex_lock(&survivors->lock);
    survivors->removenode(survivors, (Node *)s);  // Survivor'ı global listeden çıkar
    pthread_mutex_unlock(&survivors->lock);

    free(s);  // Hafıza temizleme
}
