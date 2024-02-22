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

RendererCreated rendererCreate(bool validate, SDL_Window *window) {
  RendererCreated result = {0};
  result.value = SDL_malloc(sizeof(Renderer));
  if (!result.value) goto finally;
  InstanceCreated inst = instanceCreate(validate);
  if (!inst.ok) goto finally;
  result.value->instance = inst.instance;
  result.value->messenger = inst.messenger;
  if (!SDL_Vulkan_CreateSurface(window, inst.instance, NULL,
                                &result.value->surface)) {
    goto clean_instance;
  }
  DeviceCreated device = deviceCreate(inst.instance, result.value->surface);
  if (!device.ok) {
    goto clean_surface;
  }
  result.value->phy = device.phy;
  result.value->device = device.device;
  result.value->graphicsQueue = device.graphicsQueue;
  result.value->presentQueue = device.presentQueue;
ok:
  result.ok = true;
clean_device:
  if (!result.ok) vkDestroyDevice(device.device, NULL);
clean_surface:
  if (!result.ok)
    vkDestroySurfaceKHR(result.value->instance, result.value->surface, NULL);
clean_instance:
  if (!result.ok) {
    if (inst.messenger != VK_NULL_HANDLE)
      vkDestroyDebugUtilsMessengerEXT(inst.instance, inst.messenger, NULL);
    vkDestroyInstance(inst.instance, NULL);
  }
clean_malloc:
  if (!result.ok) SDL_free(result.value);
finally:
  return result;
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