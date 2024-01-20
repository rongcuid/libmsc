#include <SDL3/SDL_main.h>
#include <volk/volk.h>

#include "renderer/renderer.h"

void printSdlVersion() {
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "SDL compiled with: %d.%d.%d",
               SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
  SDL_version v;
  SDL_GetVersion(&v);
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "SDL linked with: %d.%d.%d",
               v.major, v.minor, v.patch);
}

int main(int argc, char **argv) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER |
               SDL_INIT_GAMEPAD)) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "SDL Init error: %s\n",
                    SDL_GetError());
    return 1;
  }
  VkResult res;
  if ((res = volkInitialize()) != VK_SUCCESS) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Volk Init error: %d\n", res);
    return 1;
  }
  SDL_Window *window =
      SDL_CreateWindow("MSC", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);
  RendererCreated renderer = rendererCreate(true, window);
  if (!renderer.ok) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create renderer");
    return 1;
  }
  rendererDestroy(renderer.value);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}