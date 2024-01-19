#pragma once

#include <volk/volk.h>

typedef struct {
  VkPhysicalDevice phy;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  b32 ok;
} DeviceCreated;
DeviceCreated deviceCreate(VkInstance instance, VkSurfaceKHR surface);