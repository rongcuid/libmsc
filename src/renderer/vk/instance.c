#include "instance.h"

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

////// Instance creation

static VkBool32 debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  (void)type;
  (void)pUserData;
  if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", pCallbackData->pMessage);
  } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", pCallbackData->pMessage);
  } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", pCallbackData->pMessage);
  } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s", pCallbackData->pMessage);
  }
  return VK_FALSE;
}

typedef struct {
  VkExtensionProperties *items;
  uint32_t len;
  bool ok;
} InstanceExtensionProperties;
static InstanceExtensionProperties enumerateInstanceExtensionProperties() {
  InstanceExtensionProperties result = {0};
  if (vkEnumerateInstanceExtensionProperties(NULL, &result.len, NULL)) {
    SDL_Log("Failed to enumerate count of instance extension properties");
    goto finally;
  }
  result.items = SDL_calloc(result.len, sizeof(VkExtensionProperties));
  if (!result.items) goto finally;
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

bool instanceSupportsPortability(const InstanceExtensionProperties *props) {
  bool result = false;

  for (uint32_t i = 0; i < props->len; ++i) {
    VkExtensionProperties prop = props->items[i];
    if (SDL_strcmp(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
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
  uint32_t len;
  bool ok;
} InstanceRequiredExtensions;
static InstanceRequiredExtensions getInstanceRequiredExtensions(
    bool validate, bool portability) {
  InstanceRequiredExtensions result = {0};
  // Validation
  if (validate) result.len += 1;
  // SDL extensions
  uint32_t sdlExtsCount;
  const char *const *sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtsCount);
  result.len += sdlExtsCount;
  if (portability) result.len += 1;
  // Populate array
  result.items = SDL_calloc(result.len, sizeof(const char *));
  if (!result.items) goto finally;
  const char **it = result.items;
  if (validate) *(it++) = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  for (size_t i = 0; i < sdlExtsCount; ++i) *(it++) = sdlExts[i];
  if (portability) *(it++) = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
  result.ok = true;
  goto cleanup;
cleanup:

finally:
  return result;
}

typedef struct {
  VkLayerProperties *items;
  uint32_t len;
  bool ok;
} InstanceLayerProperties;
static InstanceLayerProperties enumerateInstanceLayerProperties() {
  InstanceLayerProperties result = {0};
  if (vkEnumerateInstanceLayerProperties(&result.len, NULL)) {
    SDL_Log("Failed to enumerate count of instance layer properties");
    goto finally;
  }
  result.items = SDL_calloc(result.len, sizeof(VkLayerProperties));
  if (!result.items) goto finally;
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
                                       uint32_t *pCount,
                                       const char **ppLayers) {
  *pCount = 0;
  for (uint32_t i = 0; i < props->len; ++i) {
    VkLayerProperties prop = props->items[i];
    if (SDL_strcmp("VK_LAYER_KHRONOS_synchronization2", prop.layerName) == 0) {
      if (ppLayers) ppLayers[*pCount] = "VK_LAYER_KHRONOS_synchronization2";
      ++(*pCount);
    } else if (SDL_strcmp("VK_LAYER_KHRONOS_timeline_semaphore",
                          prop.layerName) == 0) {
      if (ppLayers) ppLayers[*pCount] = "VK_LAYER_KHRONOS_timeline_semaphore";
      ++(*pCount);
    }
  }
}

typedef struct {
  const char **items;
  uint32_t len;
  bool ok;
} InstanceRequiredLayers;
InstanceRequiredLayers getInstanceRequiredLayers(bool validate) {
  InstanceRequiredLayers result = {0};
  // Count extensions
  if (validate) ++result.len;
  InstanceLayerProperties props = enumerateInstanceLayerProperties();
  uint32_t instLayersCount;
  findInstanceRequiredLayers(&props, &instLayersCount, NULL);
  result.len += instLayersCount;
  // Populate extensions
  result.items = SDL_calloc(result.len, sizeof(const char *));
  if (!result.items) goto clean_layer_props;
  const char **it = result.items;
  if (validate) *(it++) = "VK_LAYER_KHRONOS_validation";
  findInstanceRequiredLayers(&props, &instLayersCount, it);
  it += instLayersCount;
  // Done
  result.ok = true;
clean_layer_props:
  SDL_free(props.items);
finally:
  return result;
}

bool initInstance(struct Instance *pInstance, bool validate) {
  SDL_Log("Initializing Vulkan instance");
  bool ok = false;
  VkInstance instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
  // Instance extensions
  InstanceExtensionProperties props = enumerateInstanceExtensionProperties();
  bool portability = instanceSupportsPortability(&props);
  InstanceRequiredExtensions exts =
      getInstanceRequiredExtensions(validate, portability);
  if (!exts.ok) goto clean_ext_props;
  InstanceRequiredLayers layers = getInstanceRequiredLayers(validate);
  if (!layers.ok) goto clean_exts;
  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "MSC",
      .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
      .pEngineName = "MSCE",
      .engineVersion = VK_MAKE_VERSION(0, 1, 0),
      .apiVersion = VK_API_VERSION_1_2,
  };
  VkInstanceCreateInfo ci = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .flags =
          portability ? VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR : 0,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = layers.len,
      .ppEnabledLayerNames = layers.items,
      .enabledExtensionCount = exts.len,
      .ppEnabledExtensionNames = exts.items,
  };
  VkDebugUtilsMessengerCreateInfoEXT debugCI = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
      .pfnUserCallback = &debugCallback,
      .pUserData = NULL,
  };
  if (validate) {
    ci.pNext = &debugCI;
  }
  if (vkCreateInstance(&ci, NULL, &instance)) {
    goto err;
  }
  volkLoadInstance(instance);
  if (validate) {
    if (vkCreateDebugUtilsMessengerEXT(instance, &debugCI, NULL, &messenger)) {
      goto err_after_instance;
    }
  }
  ok = true;
  pInstance->instance = instance;
  pInstance->messenger = messenger;
  goto cleanup;
err:
err_after_instance:
  vkDestroyInstance(instance, NULL);
cleanup:
clean_layers:
  SDL_free(layers.items);
clean_exts:
  SDL_free(exts.items);
clean_ext_props:
  SDL_free(props.items);
finally:
  return ok;
}

void deinitInstance(struct Instance *pInstance) {
  SDL_Log("Deinitializing Vulkan instance");
  if (pInstance->messenger != VK_NULL_HANDLE)
    vkDestroyDebugUtilsMessengerEXT(pInstance->instance, pInstance->messenger,
                                    NULL);
  vkDestroyInstance(pInstance->instance, NULL);
  *pInstance = (struct Instance){0};
}