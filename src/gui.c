#include "gui.h"
#include <SDL2/SDL_ttf.h>
#include <err.h>
#include <stdlib.h>
#include <pthread.h>
#include "kvm.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

SDL_Window *win;
SDL_Renderer *renderer;
TTF_Font *font;
pthread_t stopping_thread;
struct gui_cell vga_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

void gui_poll_exit()
{
    SDL_Event event;
    bool running = true;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }
    }

    exit(0);
}

void gui_init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        exit(1);
    }

    win = SDL_CreateWindow("VGA Text Display", 100, 100, 640, 400, SDL_WINDOW_SHOWN);
    if (win == NULL)
    {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        SDL_DestroyWindow(win);
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    if (TTF_Init() != 0)
    {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        exit(1);
    }

    font = TTF_OpenFont("Px437_IBM_VGA_8x16.ttf", 16);
    if (font == NULL)
    {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        exit(1);
    }
    int char_width, char_height;
    TTF_SizeText(font, "W", &char_width, &char_height);
    printf("Char width: %d, char height: %d\n", char_width, char_height);

    pthread_create(&stopping_thread, NULL, gui_poll_exit, NULL);
}

void gui_update(int x, int y, char character, SDL_Color fg_color, SDL_Color bg_color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
        return;
    
    vga_buffer[y][x].character = character;
    vga_buffer[y][x].fg_color = fg_color;
    vga_buffer[y][x].bg_color = bg_color;
}

void gui_render()
{
    SDL_Rect rect;
    SDL_Surface *surface;
    SDL_Texture *texture;

    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            struct gui_cell cell = vga_buffer[y][x];

            rect.x = x * 8;
            rect.y = y * 16;
            rect.w = 8;
            rect.h = 16;
            SDL_SetRenderDrawColor(renderer, cell.bg_color.r, cell.bg_color.g, cell.bg_color.b, cell.bg_color.a);
            SDL_RenderFillRect(renderer, &rect);

            surface = TTF_RenderGlyph_Solid(font, cell.character, cell.fg_color);
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);

            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
        }
    }

    SDL_RenderPresent(renderer);
}

void gui_deinit()
{
    SDL_Event quit_event = {
        .type = SDL_QUIT};
    SDL_PushEvent(&quit_event);
    pthread_join(stopping_thread, NULL);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
}