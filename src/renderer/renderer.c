#include "renderer.h"

#include <SDL3/SDL_vulkan.h>
#include <msc_arena.h>
#include <volk.h>

#include "nk_layer/nk_layer.h"
#include "vk/device.h"
#include "vk/instance.h"

////// Create Instance

////// Renderer

struct Renderer_T {
  void *scratch_memory;
  struct msca scratch;
  struct Instance instance;
  VkSurfaceKHR surface;
  struct Device device;
  VkPipelineCache cache;
  //
  NkLayer nkLayer;
};

static const size_t SCRATCH_SIZE = 16 * 1024 * 1024;

static bool pickSurfaceFormat(VkPhysicalDevice pdev, VkSurfaceKHR surface,
                              struct msca scratch,
                              VkSurfaceFormatKHR *pFormat) {
  VkSurfaceFormatKHR result;
  bool ok = false;
  struct {
    uint32_t len;
    VkSurfaceFormatKHR *items;
  } formats;
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(pdev, surface, &formats.len, NULL))
    goto finally;
  formats.items = msca_alloc(&scratch, alignof(VkSurfaceFormatKHR), formats.len,
                             sizeof(VkSurfaceFormatKHR));
  if (!formats.items) goto finally;
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(pdev, surface, &formats.len,
                                           formats.items))
    goto finally;
  if (formats.len == 0) goto finally;
  for (uint32_t i = 0; i < formats.len; ++i) {
    VkSurfaceFormatKHR *f = &formats.items[i];
    if (f->format == VK_FORMAT_B8G8R8A8_SRGB &&
        f->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      result = *f;
      goto ok;
    }
  }
  result = formats.items[0];
ok:
  *pFormat = result;
  ok = true;
finally:
  return ok;
}

bool createRenderer(Renderer *pRenderer, bool validate, SDL_Window *window) {
  SDL_Log("Creating renderer");
  bool ok = false;
  //
  Renderer renderer = SDL_malloc(sizeof(struct Renderer_T));
  if (!renderer) goto finally;
  renderer->scratch_memory = SDL_malloc(SCRATCH_SIZE);
  if (!renderer->scratch_memory) goto clean_renderer;
  msca_init(&renderer->scratch, renderer->scratch_memory, SCRATCH_SIZE);
  if (!initInstance(&renderer->instance, validate, renderer->scratch))
    goto clean_scratch;
  if (!SDL_Vulkan_CreateSurface(window, renderer->instance.instance, NULL,
                                &renderer->surface)) {
    goto clean_instance;
  }
  if (!initDevice(&renderer->device, renderer->instance.instance,
                  renderer->surface, renderer->scratch)) {
    goto clean_surface;
  }
  VkPipelineCacheCreateInfo pipelineCacheCI = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
  };
  if (vkCreatePipelineCache(renderer->device.device, &pipelineCacheCI, NULL,
                            &renderer->cache)) {
    goto clean_device;
  }
  VkSurfaceFormatKHR format;
  if (!pickSurfaceFormat(renderer->device.physical, renderer->surface,
                         renderer->scratch, &format))
    goto clean_device;
  if (!createNkLayer(&renderer->nkLayer, renderer->device.device,
                     renderer->cache, format.format, renderer->scratch))
    goto clean_device;
ok:
  *pRenderer = renderer;
  ok = true;
clean_cache:
  if (!ok)
    vkDestroyPipelineCache(renderer->device.device, renderer->cache, NULL);
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
  destroyNkLayer(renderer->nkLayer);
  deinitDevice(&renderer->device);
  vkDestroySurfaceKHR(renderer->instance.instance, renderer->surface, NULL);
  deinitInstance(&renderer->instance);
  SDL_free(renderer->scratch_memory);
  SDL_free(renderer);
}