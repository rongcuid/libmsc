#pragma once

#include <volk.h>

struct Device {
  VkPhysicalDevice physical;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
};

bool createDevice(struct Device *pDevice, VkInstance instance,
                  VkSurfaceKHR surface);