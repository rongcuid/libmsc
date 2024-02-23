#include "renderer.h"

#include <SDL3/SDL_vulkan.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>

#include "vk/device.h"
#include "vk/instance.h"

////// Create Instance

////// Renderer

struct Renderer_T {
  VkInstance instance;
  VkDebugUtilsMessengerEXT messenger;
  VkSurfaceKHR surface;
  VkPhysicalDevice phy;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
};

bool createRenderer(Renderer *pRenderer, bool validate, SDL_Window *window) {
  bool ok = false;
  Renderer renderer = SDL_malloc(sizeof(struct Renderer_T));
  if (!renderer) goto finally;
  if (!instanceCreate(&renderer->instance, &renderer->messenger, validate))
    goto finally;
  if (!SDL_Vulkan_CreateSurface(window, renderer->instance, NULL,
                                &renderer->surface)) {
    goto clean_instance;
  }
  if (!deviceCreate(&renderer->phy, &renderer->device, &renderer->graphicsQueue,
                    &renderer->presentQueue, renderer->instance,
                    renderer->surface)) {
    goto clean_surface;
  }
ok:
  *pRenderer = renderer;
  ok = true;
clean_device:
  if (!ok) vkDestroyDevice(renderer->device, NULL);
clean_surface:
  if (!ok) vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
clean_instance:
  if (!ok) {
    if (renderer->messenger != VK_NULL_HANDLE)
      vkDestroyDebugUtilsMessengerEXT(renderer->instance, renderer->messenger,
                                      NULL);
    vkDestroyInstance(renderer->instance, NULL);
  }
clean_malloc:
  if (!ok) SDL_free(renderer);
finally:
  return ok;
}

void destroyRenderer(Renderer renderer) {
  vkDestroyDevice(renderer->device, NULL);
  vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
  if (renderer->messenger != VK_NULL_HANDLE)
    vkDestroyDebugUtilsMessengerEXT(renderer->instance, renderer->messenger,
                                    NULL);
  vkDestroyInstance(renderer->instance, NULL);
  SDL_free(renderer);
}