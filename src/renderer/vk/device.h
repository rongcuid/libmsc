#pragma once

#include <msc_arena.h>
#include <volk.h>

struct Device {
  VkPhysicalDevice physical;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
};

bool initDevice(struct Device *pDevice, VkInstance instance,
                VkSurfaceKHR surface, struct msca scratch);
void deinitDevice(struct Device *pDevice);