#pragma once

#include <SDL3/SDL.h>

typedef struct Renderer Renderer;

bool rendererCreate(Renderer **renderer, bool validate, SDL_Window *window);
void rendererDestroy(Renderer *renderer);