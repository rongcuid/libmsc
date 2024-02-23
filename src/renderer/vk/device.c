#include "device.h"

bool enumeratePhysicalDevices(VkInstance instance, struct msca *up,
                              uint32_t *pPhysicalDeviceCount,
                              VkPhysicalDevice **ppPhysicalDevices) {
  struct {
    uint32_t len;
    VkPhysicalDevice *items;
  } result = {0};
  bool ok = false;
  msca_cp_t cp = msca_checkpoint(up);
  if (vkEnumeratePhysicalDevices(instance, &result.len, NULL)) goto finally;

  result.items = msca_try_alloc(up, alignof(VkPhysicalDevice), result.len,
                                sizeof(VkPhysicalDevice));
  if (!result.items) {
    goto finally;
  }
  if (vkEnumeratePhysicalDevices(instance, &result.len, result.items)) {
    goto finally;
  }
ok:
  ok = true;
  *pPhysicalDeviceCount = result.len;
  *ppPhysicalDevices = result.items;
finally:
  if (!ok) msca_rewind(up, cp);
  return ok;
}

static int deviceRank(VkPhysicalDevice phy) {
  VkPhysicalDeviceProperties prop;
  vkGetPhysicalDeviceProperties(phy, &prop);
  switch (prop.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      return 0;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      return 1;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      return 2;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      return 3;
    default:
      return 4;
  }
}

// Pinned struct
typedef struct {
  VkPhysicalDeviceFeatures2 features2;
  VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRendering;
  VkPhysicalDeviceSynchronization2FeaturesKHR sync2;
  VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timelineSemaphore;
  VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexing;
} RequiredFeatures;

static void initRequiredFeatures(RequiredFeatures *features) {
  *features = (RequiredFeatures){0};
  features->features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  features->features2.pNext = &features->dynamicRendering;
  features->dynamicRendering.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
  features->dynamicRendering.pNext = &features->sync2;
  features->sync2.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
  features->sync2.pNext = &features->timelineSemaphore;
  features->timelineSemaphore.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR;
  features->timelineSemaphore.pNext = &features->descriptorIndexing;
  features->descriptorIndexing.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
}

static void queryRequiredFeatures(RequiredFeatures *features,
                                  VkPhysicalDevice phys) {
  vkGetPhysicalDeviceFeatures2(phys, &features->features2);
}

static void setRequiredFeatures(RequiredFeatures *features) {
  features->dynamicRendering.dynamicRendering = true;
  features->sync2.synchronization2 = true;
  features->timelineSemaphore.timelineSemaphore = true;
  features->descriptorIndexing.descriptorBindingPartiallyBound = true;
  features->descriptorIndexing.descriptorBindingSampledImageUpdateAfterBind =
      true;
  features->descriptorIndexing.descriptorBindingVariableDescriptorCount = true;
  features->descriptorIndexing.runtimeDescriptorArray = true;
}

static bool requiredFeaturesIsAvailable(const RequiredFeatures *features) {
  return features->dynamicRendering.dynamicRendering &&
         features->sync2.synchronization2 &&
         features->timelineSemaphore.timelineSemaphore &&
         features->descriptorIndexing.descriptorBindingPartiallyBound &&
         features->descriptorIndexing
             .descriptorBindingSampledImageUpdateAfterBind &&
         features->descriptorIndexing
             .descriptorBindingVariableDescriptorCount &&
         features->descriptorIndexing.runtimeDescriptorArray;
}

static void summarizeDevice(VkPhysicalDevice phy) {
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(phy, &props);
  const char *type;
  switch (props.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      type = "CPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      type = "Discrete";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      type = "Integrated";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      type = "Virtual";
      break;
    default:
      type = "Other";
      break;
  }
  RequiredFeatures features;
  initRequiredFeatures(&features);
  queryRequiredFeatures(&features, phy);
  SDL_Log(
      "[%s] (%s): %sdynamicRendering %ssync2 %stimelineSemaphore "
      "%sdescriptorIndexing",
      props.deviceName, type,
      features.dynamicRendering.dynamicRendering ? "+" : "-",
      features.sync2.synchronization2 ? "+" : "-",
      features.timelineSemaphore.timelineSemaphore ? "+" : "-",
      (features.descriptorIndexing.descriptorBindingPartiallyBound &&
       features.descriptorIndexing
           .descriptorBindingSampledImageUpdateAfterBind &&
       features.descriptorIndexing.descriptorBindingVariableDescriptorCount &&
       features.descriptorIndexing.runtimeDescriptorArray)
          ? "+"
          : "-");
}

static int compareDevice(const void *a, const void *b) {
  const VkPhysicalDevice *pa = a;
  const VkPhysicalDevice *pb = b;
  return deviceRank(*pa) - deviceRank(*pb);
}

VkPhysicalDevice pickDevices(VkPhysicalDevice *pDevices, uint32_t len) {
  SDL_qsort(pDevices, len, sizeof(VkPhysicalDevice), &compareDevice);
  for (uint32_t i = 0; i < len; ++i) {
    summarizeDevice(pDevices[i]);
    RequiredFeatures features;
    initRequiredFeatures(&features);
    queryRequiredFeatures(&features, pDevices[i]);
    if (requiredFeaturesIsAvailable(&features)) {
      return pDevices[i];
    }
  }
  return VK_NULL_HANDLE;
}

static bool qfiSupportsGraphics(const VkQueueFamilyProperties *prop) {
  return (prop->queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
}

static const float queuePriorities[1] = {1.0};

/**
 * @brief Create one queue per family
 *
 * @param phy
 * @return DeviceQCIs
 */
static bool getDeviceQCIs(VkPhysicalDevice phy, VkSurfaceKHR surface,
                          struct msca *up, uint32_t *pQueueCICount,
                          VkDeviceQueueCreateInfo **pQueueCIs,
                          uint32_t *pGraphicsQCI, uint32_t *pPresentQFI) {
  struct {
    VkDeviceQueueCreateInfo *items;
    uint32_t len;
    uint32_t graphicsQFI;
    uint32_t presentQFI;
    bool ok;
  } result = {0};
  msca_cp_t cp = msca_checkpoint(up);
  uint32_t numQFIs;
  vkGetPhysicalDeviceQueueFamilyProperties(phy, &numQFIs, NULL);
  VkQueueFamilyProperties *props =
      msca_try_alloc(up, alignof(*props), numQFIs, sizeof(*props));
  vkGetPhysicalDeviceQueueFamilyProperties(phy, &numQFIs, props);
  bool foundGraphics = false, foundPresent = false;
  uint32_t graphicsQFI, presentQFI;
  for (ptrdiff_t i = 0; i < numQFIs; ++i) {
    if (!foundGraphics && qfiSupportsGraphics(&props[i])) {
      foundGraphics = true;
      graphicsQFI = i;
    }
    VkBool32 supportsPresent;
    if (vkGetPhysicalDeviceSurfaceSupportKHR(phy, i, surface,
                                             &supportsPresent)) {
      goto finally;
    }
    if (!foundPresent && supportsPresent) {
      foundPresent = true;
      presentQFI = i;
    }
  }
  if (!foundGraphics || !foundPresent) {
    goto finally;
  }
  if (graphicsQFI == presentQFI) {
    result.len = 1;
  } else {
    result.len = 2;
  }
  result.items = msca_try_alloc(up, alignof(VkDeviceQueueCreateInfo),
                                result.len, sizeof(VkDeviceQueueCreateInfo));
  result.items[0] = (VkDeviceQueueCreateInfo){
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = graphicsQFI,
      .queueCount = 1,
      .pQueuePriorities = queuePriorities,
  };
  if (result.len == 2) {
    result.items[1] = (VkDeviceQueueCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = presentQFI,
        .queueCount = 1,
        .pQueuePriorities = queuePriorities,
    };
  }
ok:
  result.ok = true;
  *pQueueCICount = result.len;
  *pQueueCIs = result.items;
  *pGraphicsQCI = result.graphicsQFI;
  *pPresentQFI = result.presentQFI;
finally:
  if (!result.ok) msca_rewind(up, cp);
  return result.ok;
}

static void fillRequiredExtensions(const VkExtensionProperties *props,
                                   uint32_t numProps, uint32_t *pLen,
                                   const char **ppExtensions) {
  *pLen = 0;
  for (uint32_t i = 0; i < numProps; ++i) {
    const VkExtensionProperties *prop = &props[i];
    if (SDL_strcmp(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                   prop->extensionName) == 0) {
      if (ppExtensions)
        ppExtensions[*pLen] = VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME;
      ++(*pLen);
    } else if (SDL_strcmp(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
                          prop->extensionName) == 0) {
      if (ppExtensions)
        ppExtensions[*pLen] = VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME;
      ++(*pLen);
    } else if (SDL_strcmp(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
                          prop->extensionName) == 0) {
      if (ppExtensions)
        ppExtensions[*pLen] = VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME;
      ++(*pLen);
    } else if (SDL_strcmp("VK_KHR_portability_subset", prop->extensionName) ==
               0) {
      if (ppExtensions) ppExtensions[*pLen] = "VK_KHR_portability_subset";
      ++(*pLen);
    }
  }
}

static bool getRequiredExtensions(VkPhysicalDevice phy, struct msca *up,
                                  uint32_t *extensionCount,
                                  const char ***pppExtensions) {
  struct {
    const char **items;
    uint32_t len;
    bool ok;
  } result = {0};
  msca_cp_t cp = msca_checkpoint(up);
  uint32_t numProps;
  if (vkEnumerateDeviceExtensionProperties(phy, NULL, &numProps, NULL)) {
    goto finally;
  }
  VkExtensionProperties *props =
      msca_try_alloc(up, alignof(VkExtensionProperties), numProps,
                     sizeof(VkExtensionProperties));
  if (vkEnumerateDeviceExtensionProperties(phy, NULL, &numProps, props)) {
    goto finally;
  }
  fillRequiredExtensions(props, numProps, &result.len, NULL);
  result.items = msca_try_alloc(up, alignof(const char *), result.len,
                                sizeof(const char *));
  fillRequiredExtensions(props, numProps, &result.len, result.items);
ok:
  result.ok = true;
  *extensionCount = result.len;
  *pppExtensions = result.items;
finally:
  if (!result.ok) msca_rewind(up, cp);
  return result.ok;
}

bool initDevice(struct Device *pDevice, VkInstance instance,
                VkSurfaceKHR surface, struct msca scratch) {
  SDL_Log("Initializing Vulkan device");
  bool ok = false;
  struct Device device = {0};
  //
  struct {
    uint32_t len;
    VkPhysicalDevice *items;
  } pdevs;
  if (!enumeratePhysicalDevices(instance, &scratch, &pdevs.len, &pdevs.items)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to enumerate physical devices");
    goto finally;
  }
  device.physical = pickDevices(pdevs.items, pdevs.len);
  if (device.physical == VK_NULL_HANDLE) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No suitable device");
    goto finally;
  }
  RequiredFeatures features;
  initRequiredFeatures(&features);
  setRequiredFeatures(&features);
  struct {
    uint32_t len;
    VkDeviceQueueCreateInfo *items;
    uint32_t graphicsQFI;
    uint32_t presentQFI;
  } qcis;
  if (!getDeviceQCIs(device.physical, surface, &scratch, &qcis.len, &qcis.items,
                     &qcis.graphicsQFI, &qcis.presentQFI)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No suitable queues");
    goto finally;
  }
  struct {
    const char **items;
    uint32_t len;
  } exts;
  if (!getRequiredExtensions(device.physical, &scratch, &exts.len,
                             &exts.items)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to get required device extensions");
    goto finally;
  }
  VkDeviceCreateInfo ci = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &features.features2,
      .queueCreateInfoCount = qcis.len,
      .pQueueCreateInfos = qcis.items,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = NULL,
      .enabledExtensionCount = exts.len,
      .ppEnabledExtensionNames = exts.items,
  };
  if (vkCreateDevice(device.physical, &ci, NULL, &device.device)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create device");
    goto finally;
  }
  vkGetDeviceQueue(device.device, qcis.graphicsQFI, 0, &device.graphicsQueue);
  vkGetDeviceQueue(device.device, qcis.presentQFI, 0, &device.presentQueue);
success:
  *pDevice = device;
  ok = true;
finally:
  return ok;
}

void deinitDevice(struct Device *pDevice) {
  SDL_Log("Deinitializing Vulkan device");
  vkDestroyDevice(pDevice->device, NULL);
  *pDevice = (struct Device){0};
}