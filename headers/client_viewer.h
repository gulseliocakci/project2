#ifndef CLIENT_VIEWER_H
#define CLIENT_VIEWER_H

#include <SDL2/SDL.h>
#include "coord.h"

void draw_grid(SDL_Renderer* renderer);
void draw_drones(SDL_Renderer* renderer, int drone_count, Coord drones[]);
void start_client_viewer(Coord drones[], int drone_count);

#endif