#include "renderer.h"

#include <SDL3/SDL_vulkan.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>

#include "vk/device.h"
#include "vk/instance.h"

////// Create Instance

////// Renderer

struct Renderer {
  VkInstance instance;
  VkDebugUtilsMessengerEXT messenger;
  VkSurfaceKHR surface;
  VkPhysicalDevice phy;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
};

bool rendererCreate(struct Renderer **ppRenderer, bool validate,
                    SDL_Window *window) {
  bool ok = false;
  struct Renderer *pRenderer = SDL_malloc(sizeof(struct Renderer));
  if (!pRenderer) goto finally;
  if (!instanceCreate(&pRenderer->instance, &pRenderer->messenger, validate))
    goto finally;
  if (!SDL_Vulkan_CreateSurface(window, pRenderer->instance, NULL,
                                &pRenderer->surface)) {
    goto clean_instance;
  }
  DeviceCreated device = deviceCreate(pRenderer->instance, pRenderer->surface);
  if (!device.ok) {
    goto clean_surface;
  }
  pRenderer->phy = device.phy;
  pRenderer->device = device.device;
  pRenderer->graphicsQueue = device.graphicsQueue;
  pRenderer->presentQueue = device.presentQueue;
ok:
  *ppRenderer = pRenderer;
  ok = true;
clean_device:
  if (!ok) vkDestroyDevice(device.device, NULL);
clean_surface:
  if (!ok) vkDestroySurfaceKHR(pRenderer->instance, pRenderer->surface, NULL);
clean_instance:
  if (!ok) {
    if (pRenderer->messenger != VK_NULL_HANDLE)
      vkDestroyDebugUtilsMessengerEXT(pRenderer->instance, pRenderer->messenger,
                                      NULL);
    vkDestroyInstance(pRenderer->instance, NULL);
  }
clean_malloc:
  if (!ok) SDL_free(pRenderer);
finally:
  return ok;
}

void rendererDestroy(struct Renderer *renderer) {
  vkDestroyDevice(renderer->device, NULL);
  vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
  if (renderer->messenger != VK_NULL_HANDLE)
    vkDestroyDebugUtilsMessengerEXT(renderer->instance, renderer->messenger,
                                    NULL);
  vkDestroyInstance(renderer->instance, NULL);
  SDL_free(renderer);
}