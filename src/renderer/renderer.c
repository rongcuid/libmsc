#include "renderer.h"

#include <SDL3/SDL_vulkan.h>
#include <volk.h>

#include <string.h>

////// Create Instance

typedef struct {
  VkExtensionProperties *items;
  u32 len;
  b32 ok;
} InstanceExtensionProperties;
static InstanceExtensionProperties enumerateInstanceExtensionProperties() {
  InstanceExtensionProperties result = {0};
  if (vkEnumerateInstanceExtensionProperties(NULL, &result.len, NULL)) {
    SDL_Log("Failed to enumerate count of instance extension properties");
    goto finally;
  }
  result.items = SDL_calloc(result.len, sizeof(VkExtensionProperties));
  if (!result.items)
    goto finally;
  if (vkEnumerateInstanceExtensionProperties(NULL, &result.len, result.items)) {
    SDL_Log("Failed to enumerate instance extension properties");
    goto err_after_calloc_props;
  }
  result.ok = true;
  goto finally;
err_after_calloc_props:
  SDL_free(result.items);
  goto finally;
finally:
  return result;
}

static b32
instanceSupportsPortability(const InstanceExtensionProperties *props) {
  b32 result = false;

  for (u32 i = 0; i < props->len; ++i) {
    VkExtensionProperties prop = props->items[i];
    if (strcmp(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
               prop.extensionName) == 0) {
      result = true;
      goto finally;
    }
  }
finally:
  return result;
}

typedef struct {
  const char **items;
  u32 count;
  b32 ok;
} InstanceRequiredExtensions;
static InstanceRequiredExtensions getInstanceRequiredExtensions(b32 validate) {
  InstanceRequiredExtensions result = {0};
  // Validation
  if (validate)
    result.count += 1;
  // SDL extensions
  u32 sdlExtsCount;
  const char *const *sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtsCount);
  result.count += sdlExtsCount;
  // Instance extensions
  InstanceExtensionProperties props = enumerateInstanceExtensionProperties();
  b32 portability = instanceSupportsPortability(&props);
  if (portability)
    result.count += 1;
  // Populate array
  result.items = SDL_calloc(result.count, sizeof(const char *));
  if (!result.items)
    goto finally;
  const char **it = result.items;
  if (validate)
    *(it++) = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  for (usize i = 0; i < sdlExtsCount; ++i)
    *(it++) = sdlExts[i];
  if (portability)
    *(it++) = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
  result.ok = true;
  goto cleanup;
cleanup:
clean_get_extension_props:
  SDL_free(props.items);
finally:
  return result;
}

typedef struct {
  VkLayerProperties *items;
  u32 len;
  b32 ok;
} InstanceLayerProperties;
InstanceLayerProperties enumerateInstanceLayerProperties() {
  InstanceLayerProperties result = {0};
  if (vkEnumerateInstanceLayerProperties(&result.len, NULL)) {
    SDL_Log("Failed to enumerate count of instance layer properties");
    goto finally;
  }
  result.items = SDL_calloc(result.len, sizeof(VkLayerProperties));
  if (!result.items)
    goto finally;
  if (vkEnumerateInstanceLayerProperties(&result.len, result.items)) {
    SDL_Log("Failed to enumerate instance layer properties");
    goto err_after_calloc_props;
  }
  // Done
  result.ok = true;
  goto finally;
err_after_calloc_props:
  SDL_free(result.items);
  goto finally;
finally:
  return result;
}

static void findInstanceRequiredLayers(const InstanceLayerProperties *props,
                                       u32 *pCount, const char **ppLayers) {
  *pCount = 0;
  for (u32 i = 0; i < props->len; ++i) {
    VkLayerProperties prop = props->items[i];
    if (strcmp("VK_LAYER_KHRONOS_synchronization2", prop.layerName) == 0) {
      if (ppLayers)
        ppLayers[*pCount] = "VK_LAYER_KHRONOS_synchronization2";
      ++(*pCount);
    } else if (strcmp("VK_LAYER_KHRONOS_timeline_semaphore", prop.layerName) ==
               0) {
      if (ppLayers)
        ppLayers[*pCount] = "VK_LAYER_KHRONOS_timeline_semaphore";
      ++(*pCount);
    }
  }
}

typedef struct {
  const char **items;
  u32 len;
  b32 ok;
} InstanceRequiredLayers;
InstanceRequiredLayers getInstanceRequiredLayers(b32 validate) {
  InstanceRequiredLayers result = {0};
  // Count extensions
  if (validate)
    ++result.len;
  InstanceLayerProperties props = enumerateInstanceLayerProperties();
  u32 instLayersCount;
  findInstanceRequiredLayers(&props, &instLayersCount, NULL);
  result.len += instLayersCount;
  // Populate extensions
  result.items = SDL_calloc(result.len, sizeof(const char *));
  if (!result.items)
    goto clean_layer_props;
  const char **it = result.items;
  if (validate)
    *(it++) = "VK_LAYER_KHRONOS_validation";
  findInstanceRequiredLayers(&props, &instLayersCount, it);
  it += instLayersCount;
  // Done
  result.ok = true;
clean_layer_props:
  SDL_free(props.items);
finally:
  return result;
}

typedef struct {
  VkInstance value;
  b32 ok;
} InstanceCreated;
InstanceCreated instanceCreate(b32 validate) {
  InstanceCreated result = {0};
  InstanceRequiredExtensions exts = getInstanceRequiredExtensions(validate);
  if (!exts.ok)
    goto finally;
  InstanceRequiredLayers layers = getInstanceRequiredLayers(validate);
  if (!layers.ok)
    goto clean_exts;
clean:
clean_layers:
  SDL_free(layers.items);
clean_exts:
  SDL_free(exts.items);
finally:
  return result;
}

////// Renderer

struct Renderer {
  VkInstance instance;
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