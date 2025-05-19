#include "headers/view.h"
#include "headers/globals.h"
#include "headers/drone.h"
#include "headers/survivor.h"
#include "headers/coord.h"
#include "headers/list.h"
#include <SDL2/SDL.h>
#include <stdio.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int init_sdl_window(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Drone Coordination System",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            800, 600,
                            SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, 
                                SDL_RENDERER_ACCELERATED | 
                                SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    return 0;
}

void draw_map(void) {
    if (!renderer) return;

    // Ekranı temizle
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Izgara çiz
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    for (int i = 0; i <= 20; i++) {
        SDL_RenderDrawLine(renderer, 
                          0, i * (600/20),
                          800, i * (600/20));
        SDL_RenderDrawLine(renderer,
                          i * (800/20), 0,
                          i * (800/20), 600);
    }

    // Drone'ları çiz
    if (drones) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        Node *current = drones->head;
        while (current != NULL && current->occupied) {
            Drone *d = (Drone *)current->data;
            if (d) {
                // Drone durumuna göre renk seç
                switch (d->status) {
                    case IDLE:
                        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Yeşil
                        break;
                    case ON_MISSION:
                        SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); // Turuncu
                        break;
                    case DISCONNECTED:
                        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Kırmızı
                        break;
                }

                SDL_Rect droneRect = {
                    d->coord.x * (800/20),
                    d->coord.y * (600/20),
                    10, 10
                };
                SDL_RenderFillRect(renderer, &droneRect);

                // Hedef konumu göster (eğer görevdeyse)
                if (d->status == ON_MISSION) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Sarı
                    SDL_RenderDrawLine(renderer,
                        d->coord.x * (800/20) + 5,
                        d->coord.y * (600/20) + 5,
                        d->target.x * (800/20) + 5,
                        d->target.y * (600/20) + 5
                    );
                }

                // Pil seviyesini göster
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_Rect batteryRect = {
                    d->coord.x * (800/20),
                    d->coord.y * (600/20) - 5,
                    (d->battery_level / 100.0f) * 10,
                    3
                };
                SDL_RenderFillRect(renderer, &batteryRect);
            }
            current = current->next;
        }
    }

    // Survivor'ları çiz
    if (survivors) {
        Node *current = survivors->head;
        while (current != NULL && current->occupied) {
            Survivor *s = (Survivor *)current->data;
            if (s) {
                // Survivor durumuna göre renk seç
                switch (s->status) {
                    case 0: // Kurtarılmamış
                        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                        break;
                    case 1: // Kurtarılmış
                        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                        break;
                    default:
                        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
                }

                SDL_Rect survivorRect = {
                    s->coord.x * (800/20),
                    s->coord.y * (600/20),
                    8, 8
                };
                SDL_RenderFillRect(renderer, &survivorRect);
            }
            current = current->next;
        }
    }

    SDL_RenderPresent(renderer);
}

void cleanup_sdl(void) {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
}

int check_events(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            should_quit = 1;
            return 1;
        }
    }
    return 0;
}

void quit_program(void) {
    should_quit = 1;
    cleanup_sdl();
    exit(0);
}