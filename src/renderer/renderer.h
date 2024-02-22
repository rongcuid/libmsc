#pragma once

typedef struct Renderer Renderer;

typedef struct {
  Renderer *value;
  bool ok;
} RendererCreated;
RendererCreated rendererCreate(bool validate, SDL_Window *window);
void rendererDestroy(Renderer *renderer);