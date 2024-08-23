#include "gui.h"
#include <SDL2/SDL_ttf.h>
#include <err.h>
#include <stdlib.h>
#include <pthread.h>
#include "kvm.h"

SDL_Window *win;
SDL_Renderer *renderer;
TTF_Font *font;
pthread_t stopping_thread;

void gui_poll_exit()
{
    SDL_Event event;
    bool running = true;
    while(running) 
    {
        while(SDL_PollEvent(&event)) 
        {
            if(event.type == SDL_QUIT)
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

    win = SDL_CreateWindow("VGA Text Display", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
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

    if(TTF_Init() != 0)
    {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        exit(1);
    }

    font = TTF_OpenFont("Px437_IBM_VGA_8x16.ttf", 16);
    if(font == NULL)
    {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        exit(1);
    }

    SDL_Color color = {255, 255, 255, 255};

    SDL_Surface *surface = TTF_RenderText_Solid(font, "Hello, VGA!", color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dstrect = {50, 50, surface->w, surface->h};

    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_RenderPresent(renderer);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    
    pthread_create(&stopping_thread, NULL, gui_poll_exit, NULL);
}

void gui_deinit()
{
    SDL_Event quit_event = {
        .type = SDL_QUIT
    };
    SDL_PushEvent(&quit_event);
    pthread_join(stopping_thread, NULL);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
}