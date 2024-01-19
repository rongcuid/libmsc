#include "renderer.h"

#include <SDL3/SDL_vulkan.h>
#include <volk.h>

#include <string.h>

struct Renderer {
  VkInstance instance;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
};

////// Create Instance

typedef struct {
  VkExtensionProperties *items;
  uint32_t len;
  bool ok;
} EnumeratedInstanceExtensionProperties;
static EnumeratedInstanceExtensionProperties
enumerateInstanceExtensionProperties() {
  EnumeratedInstanceExtensionProperties result = {0};
  if (vkEnumerateInstanceExtensionProperties(NULL, &result.len, NULL)) {
    SDL_Log("Failed to enumerate count of instance extension properties");
    goto finally;
  }
  result.items = SDL_calloc(result.len, sizeof(VkExtensionProperties));
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

static bool instancePortability() {
  bool result = false;
  EnumeratedInstanceExtensionProperties props =
      enumerateInstanceExtensionProperties();
  for (uint32_t i = 0; i < props.len; ++i) {
    VkExtensionProperties prop = props.items[i];
    if (strcmp(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
               prop.extensionName) == 0) {
      result = true;
      goto finally;
    }
  }
finally:
  SDL_free(props.items);
  return result;
}

typedef struct {
  const char **items;
  uint32_t count;
  bool ok;
} InstanceRequiredExtensions;
static InstanceRequiredExtensions instanceGetRequiredExtensions(bool validate) {
  InstanceRequiredExtensions result = {0};
  // Validation
  if (validate)
    result.count += 1;
  // SDL extensions
  uint32_t sdlExtsCount;
  const char *const *sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtsCount);
  result.count += sdlExtsCount;
  // Instance extensions
  bool portability = instancePortability();
  if (portability)
    result.count += 1;
  // Populate
  result.items = SDL_calloc(result.count, sizeof(const char *));
  const char **it = result.items;
  if (validate)
    *(it++) = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  for (size_t i = 0; i < sdlExtsCount; ++i)
    *(it++) = sdlExts[i];
  if (portability)
    *(it++) = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
  result.ok = true;
  goto finally;
err_after_alloc_items:
  SDL_free(result.items);
finally:
  return result;
}

typedef struct {
  VkInstance value;
  bool ok;
} InstanceCreated;
InstanceCreated instanceCreate(bool validate) {
  InstanceRequiredExtensions exts = instanceGetRequiredExtensions(validate);
}

////// Renderer

RendererCreated rendererCreate(bool validate) {}