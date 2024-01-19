#include "renderer.h"

#include <volk.h>

#include "vk/instance.h"

////// Create Instance

////// Renderer

struct Renderer {
  VkInstance instance;
  VkDebugUtilsMessengerEXT debug;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
};

RendererCreated rendererCreate(b32 validate) {
  RendererCreated result = {0};
  InstanceCreated inst = instanceCreate(validate);
  if (!inst.ok)
    goto finally;

finally:
  return result;
}