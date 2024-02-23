#pragma once

#include <volk.h>

struct Device {
  VkPhysicalDevice physical;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
};

bool initDevice(struct Device *pDevice, VkInstance instance,
                VkSurfaceKHR surface);
void deinitDevice(struct Device *pDevice);