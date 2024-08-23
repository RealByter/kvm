#include <SDL2/SDL.h>
#include <stdbool.h>

struct gui_cell {
    char character;
    SDL_Color fg_color;
    SDL_Color bg_color;
};

void gui_init();
void gui_update(int x, int y, char character, SDL_Color fg_color, SDL_Color bg_color);
void gui_render();
void gui_deinit();
