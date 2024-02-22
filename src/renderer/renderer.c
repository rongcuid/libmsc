#include "renderer.h"

#include <SDL3/SDL_vulkan.h>
#include <volk.h>

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

bool rendererCreate(Renderer **ppRenderer, bool validate, SDL_Window *window) {
  bool ok = false;
  Renderer *pRenderer = SDL_malloc(sizeof(Renderer));
  if (!pRenderer) goto finally;
  InstanceCreated inst = instanceCreate(validate);
  if (!inst.ok) goto finally;
  pRenderer->instance = inst.instance;
  pRenderer->messenger = inst.messenger;
  if (!SDL_Vulkan_CreateSurface(window, inst.instance, NULL,
                                &pRenderer->surface)) {
    goto clean_instance;
  }
  DeviceCreated device = deviceCreate(inst.instance, pRenderer->surface);
  if (!device.ok) {
    goto clean_surface;
  }
  pRenderer->phy = device.phy;
  pRenderer->device = device.device;
  pRenderer->graphicsQueue = device.graphicsQueue;
  pRenderer->presentQueue = device.presentQueue;
ok:
  ok = true;
clean_device:
  if (!ok) vkDestroyDevice(device.device, NULL);
clean_surface:
  if (!ok) vkDestroySurfaceKHR(pRenderer->instance, pRenderer->surface, NULL);
clean_instance:
  if (!ok) {
    if (inst.messenger != VK_NULL_HANDLE)
      vkDestroyDebugUtilsMessengerEXT(inst.instance, inst.messenger, NULL);
    vkDestroyInstance(inst.instance, NULL);
  }
clean_malloc:
  if (!ok) SDL_free(pRenderer);
finally:
  *ppRenderer = pRenderer;
  return ok;
}

void rendererDestroy(Renderer *renderer) {
  vkDestroyDevice(renderer->device, NULL);
  vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
  if (renderer->messenger != VK_NULL_HANDLE)
    vkDestroyDebugUtilsMessengerEXT(renderer->instance, renderer->messenger,
                                    NULL);
  vkDestroyInstance(renderer->instance, NULL);
  SDL_free(renderer);
}