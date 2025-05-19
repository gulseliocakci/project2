
#ifndef VIEW_H
#define VIEW_H
#include <SDL2/SDL.h>
/*view.c*/

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define CELL_SIZE 30

// Renk yapıları
typedef struct {
    SDL_Color WAITING_COLOR;   // Kırmızı - Bekleyen survivor
    SDL_Color HELPED_COLOR;    // Yeşil - Yardım edilmiş survivor
    SDL_Color DRONE_IDLE;      // Mavi - Boştaki drone
    SDL_Color DRONE_MISSION;   // Cyan - Görevdeki drone
    SDL_Color GRID_COLOR;      // Gri - Grid çizgileri
} ViewColors;

extern ViewColors colors;

extern SDL_Window *window;
extern SDL_Renderer *renderer;
//extern volatile int should_quit;

extern int init_sdl_window();
extern void draw_cell(int x, int y, SDL_Color color);
extern void draw_drones();
extern void draw_survivors();
extern void draw_grid();
extern void draw_map();
extern int check_events();
extern int quit_all(void);
void cleanup_sdl(void);
void quit_program(void); 
void handle_signal(int sig);

#endif