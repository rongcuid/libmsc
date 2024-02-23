#pragma once

#include <SDL3/SDL.h>

typedef struct Renderer_T *Renderer;

bool createRenderer(Renderer *pRenderer, bool validate, SDL_Window *window);
void destroyRenderer(Renderer renderer);