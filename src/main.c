#include <stdio.h>

int main(int argc, char **argv) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER |
               SDL_INIT_GAMEPAD)) {
    fprintf(stderr, "SDL Init error: %s\n", SDL_GetError());
    return 1;
  }
  SDL_Quit();
  return 0;
}