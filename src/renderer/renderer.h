#pragma once

typedef struct Renderer Renderer;

typedef struct {
  Renderer *value;
  b32 ok;
} RendererCreated;
RendererCreated rendererCreate(b32 validate);
void rendererDestroy(Renderer *renderer);