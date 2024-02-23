#include "renderer.h"

#include <SDL3/SDL_vulkan.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>

#include "vk/device.h"
#include "vk/instance.h"

////// Create Instance

////// Renderer

struct Renderer_T {
  struct Instance instance;
  VkSurfaceKHR surface;
  struct Device device;
};

bool createRenderer(Renderer *pRenderer, bool validate, SDL_Window *window) {
  SDL_Log("Creating renderer");
  bool ok = false;
  Renderer renderer = SDL_malloc(sizeof(struct Renderer_T));
  if (!renderer) goto finally;
  if (!initInstance(&renderer->instance, validate)) goto finally;
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
clean_malloc:
  if (!ok) SDL_free(renderer);
finally:
  return ok;
}

void destroyRenderer(Renderer renderer) {
  SDL_Log("Destroying renderer");
  deinitDevice(&renderer->device);
  vkDestroySurfaceKHR(renderer->instance.instance, renderer->surface, NULL);
  deinitInstance(&renderer->instance);
  SDL_free(renderer);
}