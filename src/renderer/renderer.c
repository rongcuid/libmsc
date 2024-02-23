#include "renderer.h"

#include <SDL3/SDL_vulkan.h>
#include <msc_arena.h>
#include <volk.h>

#include "vk/device.h"
#include "vk/instance.h"

////// Create Instance

////// Renderer

struct Renderer_T {
  void *scratch_memory;
  struct msc_arena scratch;
  struct Instance instance;
  VkSurfaceKHR surface;
  struct Device device;
};

static const size_t SCRATCH_SIZE = 16 * 1024 * 1024;

bool createRenderer(Renderer *pRenderer, bool validate, SDL_Window *window) {
  SDL_Log("Creating renderer");
  bool ok = false;
  //
  Renderer renderer = SDL_malloc(sizeof(struct Renderer_T));
  if (!renderer) goto finally;
  renderer->scratch_memory = SDL_malloc(SCRATCH_SIZE);
  if (!renderer->scratch_memory) goto clean_renderer;
  msc_arena_init(&renderer->scratch, renderer->scratch_memory, SCRATCH_SIZE);
  if (!initInstance(&renderer->instance, validate)) goto clean_scratch;
  if (!SDL_Vulkan_CreateSurface(window, renderer->instance.instance, NULL,
                                &renderer->surface)) {
    goto clean_instance;
  }
  if (!initDevice(&renderer->device, renderer->instance.instance,
                  renderer->surface)) {
    goto clean_surface;
  }
ok:
  *pRenderer = renderer;
  ok = true;
clean_device:
  if (!ok) vkDestroyDevice(renderer->device.device, NULL);
clean_surface:
  if (!ok)
    vkDestroySurfaceKHR(renderer->instance.instance, renderer->surface, NULL);
clean_instance:
  if (!ok) {
    if (renderer->instance.messenger != VK_NULL_HANDLE)
      vkDestroyDebugUtilsMessengerEXT(renderer->instance.instance,
                                      renderer->instance.messenger, NULL);
    vkDestroyInstance(renderer->instance.instance, NULL);
  }
clean_scratch:
  if (!ok) SDL_free(renderer->scratch_memory);
clean_renderer:
  if (!ok) SDL_free(renderer);
finally:
  return ok;
}

void destroyRenderer(Renderer renderer) {
  SDL_Log("Destroying renderer");
  deinitDevice(&renderer->device);
  vkDestroySurfaceKHR(renderer->instance.instance, renderer->surface, NULL);
  deinitInstance(&renderer->instance);
  SDL_free(renderer->scratch_memory);
  SDL_free(renderer);
}