#include "renderer.h"

#include <volk.h>

#include "vk/instance.h"

////// Create Instance

////// Renderer

struct Renderer {
  VkInstance instance;
  VkDebugUtilsMessengerEXT messenger;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
};

RendererCreated rendererCreate(b32 validate) {
  RendererCreated result = {0};
  result.value = SDL_malloc(sizeof(Renderer));
  if (!result.value)
    goto finally;
  InstanceCreated inst = instanceCreate(validate);
  if (!inst.ok)
    goto finally;
  result.value->instance = inst.instance;
  result.value->messenger = inst.messenger;
ok:
  result.ok = true;
  goto cleanup;
err_after_instance:
  if (inst.messenger != VK_NULL_HANDLE)
    vkDestroyDebugUtilsMessengerEXT(inst.instance, inst.messenger, NULL);
  vkDestroyInstance(inst.instance, NULL);
err_after_malloc:
  SDL_free(result.value);
cleanup:
finally:
  return result;
}

void rendererDestroy(Renderer *renderer) {
  if (renderer->messenger != VK_NULL_HANDLE)
    vkDestroyDebugUtilsMessengerEXT(renderer->instance, renderer->messenger,
                                    NULL);
  vkDestroyInstance(renderer->instance, NULL);
  SDL_free(renderer);
}