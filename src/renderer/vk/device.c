#include "device.h"

#include "msc_alg.h"

typedef struct {
  VkPhysicalDevice *items;
  uint32_t len;
  bool ok;
} PhysicalDevices;
PhysicalDevices enumeratePhysicalDevices(VkInstance instance) {
  PhysicalDevices result = {0};
  if (vkEnumeratePhysicalDevices(instance, &result.len, NULL)) {
    goto finally;
  }
  result.items = SDL_calloc(result.len, sizeof(VkPhysicalDevice));
  if (!result.items) {
    goto finally;
  }
  if (vkEnumeratePhysicalDevices(instance, &result.len, result.items)) {
    goto err_after_alloc;
  }
  result.ok = true;
  goto finally;
err_after_alloc:
  SDL_free(result.items);
finally:
  return result;
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

static ptrdiff_t compareDevice(const void *context, const void *a,
                               const void *b) {
  (void)context;
  const VkPhysicalDevice *pa = a, *pb = b;
  return deviceRank(*pa) - deviceRank(*pb);
}

VkPhysicalDevice pickDevices(VkPhysicalDevice *pDevices, uint32_t len) {
  mscalg_qsort(pDevices, len, sizeof(VkPhysicalDevice), &compareDevice, NULL);
  for (ptrdiff_t i = 0; i < len; ++i) {
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

typedef struct {
  VkDeviceQueueCreateInfo items[2];
  uint32_t len;
  uint32_t graphicsQFI;
  uint32_t presentQFI;
  bool ok;
} DeviceQCIs;
/**
 * @brief Create one queue per family
 *
 * @param phy
 * @return DeviceQCIs
 */
static DeviceQCIs getDeviceQCIs(VkPhysicalDevice phy, VkSurfaceKHR surface) {
  DeviceQCIs result = {0};
  uint32_t numQFIs;
  vkGetPhysicalDeviceQueueFamilyProperties(phy, &numQFIs, NULL);
  VkQueueFamilyProperties *props =
      SDL_calloc(numQFIs, sizeof(VkQueueFamilyProperties));
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
      goto clean_props;
    }
    if (!foundPresent && supportsPresent) {
      foundPresent = true;
      presentQFI = i;
    }
  }
  if (!foundGraphics || !foundPresent) {
    goto clean_props;
  }
  result.items[0] = (VkDeviceQueueCreateInfo){
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = graphicsQFI,
      .queueCount = 1,
      .pQueuePriorities = queuePriorities,
  };
  if (graphicsQFI == presentQFI) {
    result.len = 1;
  } else {
    result.len = 2;
    result.items[1] = (VkDeviceQueueCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = presentQFI,
        .queueCount = 1,
        .pQueuePriorities = queuePriorities,
    };
  }
  result.ok = true;
clean_props:
  SDL_free(props);
finally:
  return result;
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

typedef struct {
  const char **items;
  uint32_t len;
  bool ok;
} DeviceExtensions;
static DeviceExtensions getRequiredExtensions(VkPhysicalDevice phy) {
  DeviceExtensions result = {0};
  uint32_t numProps;
  if (vkEnumerateDeviceExtensionProperties(phy, NULL, &numProps, NULL)) {
    goto finally;
  }
  VkExtensionProperties *props =
      SDL_calloc(numProps, sizeof(VkExtensionProperties));
  if (vkEnumerateDeviceExtensionProperties(phy, NULL, &numProps, props)) {
    goto finally;
  }
  fillRequiredExtensions(props, numProps, &result.len, NULL);
  result.items = SDL_calloc(result.len, sizeof(const char *));
  fillRequiredExtensions(props, numProps, &result.len, result.items);
  result.ok = true;
clean_items:
  if (!result.ok) SDL_free(result.items);
clean_props:
  SDL_free(props);
finally:
  return result;
}

bool deviceCreate(VkPhysicalDevice *pPhysicalDevice, VkDevice *pDevice,
                  VkQueue *pGraphicsQueue, VkQueue *pPresentQueue,
                  VkInstance instance, VkSurfaceKHR surface) {
  bool ok = false;
  VkPhysicalDevice physicalDevice = NULL;
  VkDevice device = NULL;
  VkQueue graphicsQueue = NULL;
  VkQueue presentQueue = NULL;
  PhysicalDevices pdevs = enumeratePhysicalDevices(instance);
  if (!pdevs.ok) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to enumerate physical devices");
    goto finally;
  }
  physicalDevice = pickDevices(pdevs.items, pdevs.len);
  if (physicalDevice == VK_NULL_HANDLE) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No suitable device");
    goto clean_pdevs;
  }
  RequiredFeatures features;
  initRequiredFeatures(&features);
  setRequiredFeatures(&features);
  DeviceQCIs qcis = getDeviceQCIs(physicalDevice, surface);
  if (!qcis.ok) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No suitable queues");
    goto clean_pdevs;
  }
  DeviceExtensions exts = getRequiredExtensions(physicalDevice);
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
  if (vkCreateDevice(physicalDevice, &ci, NULL, &device)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create device");
    goto clean_exts;
  }
  vkGetDeviceQueue(device, qcis.graphicsQFI, 0, &graphicsQueue);
  vkGetDeviceQueue(device, qcis.presentQFI, 0, &presentQueue);
  *pPhysicalDevice = physicalDevice;
  *pDevice = device;
  *pGraphicsQueue = graphicsQueue;
  *pPresentQueue = presentQueue;
  ok = true;
clean_exts:
  SDL_free(exts.items);
clean_pdevs:
  SDL_free(pdevs.items);
finally:
  return ok;
}