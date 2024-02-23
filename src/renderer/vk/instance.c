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

static bool enumerateInstanceExtensionProperties(
    uint32_t *pLen, VkExtensionProperties **ppProps, struct msca *up) {
  bool ok = false;
  uint32_t len;
  msca_cp_t cp = msca_checkpoint(up);
  VkExtensionProperties *pProps;
  if (vkEnumerateInstanceExtensionProperties(NULL, &len, NULL)) {
    SDL_Log("Failed to enumerate count of instance extension properties");
    goto finally;
  }
  pProps = msca_alloc(up, alignof(*pProps), len, sizeof(*pProps));
  if (!pProps) goto finally;
  if (vkEnumerateInstanceExtensionProperties(NULL, &len, pProps)) {
    SDL_Log("Failed to enumerate instance extension properties");
    goto finally;
  }
ok:
  ok = true;
  *pLen = len;
  *ppProps = pProps;
finally:
  if (!ok) msca_rewind(up, cp);
  return ok;
}

bool instanceSupportsPortability(uint32_t propsCount,
                                 const VkExtensionProperties *pProps) {
  bool result = false;

  for (uint32_t i = 0; i < propsCount; ++i) {
    VkExtensionProperties prop = pProps[i];
    if (SDL_strcmp(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
                   prop.extensionName) == 0) {
      result = true;
      goto finally;
    }
  }
finally:
  return result;
}

static bool getInstanceRequiredExtensions(bool validate, bool portability,
                                          struct msca *up,
                                          uint32_t *pExtensionCount,
                                          const char ***pppExtensions) {
  struct {
    uint32_t len;
    const char **items;
  } result = {0};
  bool ok = false;
  msca_cp_t cp = msca_checkpoint(up);
  // Validation
  if (validate) result.len += 1;
  // SDL extensions
  uint32_t sdlExtsCount;
  const char *const *sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtsCount);
  result.len += sdlExtsCount;
  if (portability) result.len += 1;
  // Populate array
  result.items =
      msca_alloc(up, alignof(const char *), result.len, sizeof(const char *));
  if (!result.items) goto finally;
  const char **it = result.items;
  if (validate) *(it++) = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  for (size_t i = 0; i < sdlExtsCount; ++i) *(it++) = sdlExts[i];
  if (portability) *(it++) = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
ok:
  ok = true;
  *pExtensionCount = result.len;
  *pppExtensions = result.items;
finally:
  if (!ok) msca_rewind(up, cp);
  return ok;
}

static bool enumerateInstanceLayerProperties(struct msca *up,
                                             uint32_t *pItemCount,
                                             VkLayerProperties **ppItems) {
  struct {
    uint32_t len;
    VkLayerProperties *items;
  } result = {0};
  bool ok = false;
  msca_cp_t cp = msca_checkpoint(up);
  if (vkEnumerateInstanceLayerProperties(&result.len, NULL)) {
    SDL_Log("Failed to enumerate count of instance layer properties");
    goto finally;
  }
  result.items = msca_alloc(up, alignof(VkLayerProperties), result.len,
                            sizeof(VkLayerProperties));
  if (!result.items) goto finally;
  if (vkEnumerateInstanceLayerProperties(&result.len, result.items)) {
    SDL_Log("Failed to enumerate instance layer properties");
    goto finally;
  }
ok:
  ok = true;
  *pItemCount = result.len;
  *ppItems = result.items;
finally:
  if (!ok) msca_rewind(up, cp);
  return ok;
}

static void findInstanceRequiredLayers(uint32_t propCount,
                                       VkLayerProperties *pProps,
                                       uint32_t *pCount,
                                       const char **ppLayers) {
  *pCount = 0;
  for (uint32_t i = 0; i < propCount; ++i) {
    VkLayerProperties prop = pProps[i];
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

bool getInstanceRequiredLayers(bool validate, struct msca *up,
                               uint32_t *pLayerCount, const char ***pppLayers) {
  struct {
    uint32_t len;
    const char **items;
  } result = {0};
  bool ok = false;
  msca_cp_t cp = msca_checkpoint(up);
  // Count extensions
  if (validate) ++result.len;
  struct {
    uint32_t len;
    VkLayerProperties *items;
  } props;
  if (!enumerateInstanceLayerProperties(up, &props.len, &props.items))
    goto finally;
  uint32_t instLayersCount;
  findInstanceRequiredLayers(props.len, props.items, &instLayersCount, NULL);
  result.len += instLayersCount;
  // Populate extensions
  result.items =
      msca_alloc(up, alignof(const char *), result.len, sizeof(const char *));
  if (!result.items) goto finally;
  const char **it = result.items;
  if (validate) *(it++) = "VK_LAYER_KHRONOS_validation";
  findInstanceRequiredLayers(props.len, props.items, &instLayersCount, it);
  it += instLayersCount;
ok:
  ok = true;
  *pLayerCount = result.len;
  *pppLayers = result.items;
finally:
  if (!ok) msca_rewind(up, cp);
  return ok;
}

bool initInstance(struct Instance *pInstance, bool validate,
                  struct msca scratch) {
  SDL_Log("Initializing Vulkan instance");
  bool ok = false;
  VkInstance instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
  // Instance extensions
  struct {
    uint32_t len;
    VkExtensionProperties *items;
  } props;
  if (!enumerateInstanceExtensionProperties(&props.len, &props.items, &scratch))
    goto finally;

  bool portability = instanceSupportsPortability(props.len, props.items);
  struct {
    uint32_t len;
    const char **items;
  } exts;
  if (!getInstanceRequiredExtensions(validate, portability, &scratch, &exts.len,
                                     &exts.items))
    goto finally;
  struct {
    uint32_t len;
    const char **items;
  } layers;
  if (!getInstanceRequiredLayers(validate, &scratch, &layers.len,
                                 &layers.items))
    goto finally;
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
    goto finally;
  }
  volkLoadInstance(instance);
  if (validate) {
    if (vkCreateDebugUtilsMessengerEXT(instance, &debugCI, NULL, &messenger)) {
      goto clean_instance;
    }
  }
  ok = true;
  pInstance->instance = instance;
  pInstance->messenger = messenger;
clean_instance:
  if (!ok) vkDestroyInstance(instance, NULL);
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