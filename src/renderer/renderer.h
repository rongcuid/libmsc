#pragma once

typedef struct Renderer Renderer;

typedef struct {
  Renderer *value;
  bool ok;
} RendererCreated;
RendererCreated rendererCreate(bool validate);
void rendererDestroy(Renderer *renderer);