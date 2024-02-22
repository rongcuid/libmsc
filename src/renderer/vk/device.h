#pragma once

#include <volk.h>

bool deviceCreate(VkPhysicalDevice *pPhysicalDevice, VkDevice *pDevice,
                  VkQueue *pGraphicsQueue, VkQueue *pPresentQueue,
                  VkInstance instance, VkSurfaceKHR surface);