#pragma once

#include <SDL3/SDL.h>

struct Renderer;

bool rendererCreate(struct Renderer **renderer, bool validate,
                    SDL_Window *window);
void rendererDestroy(struct Renderer *renderer);