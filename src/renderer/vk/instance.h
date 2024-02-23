#pragma once

#include <volk.h>

struct Instance {
  VkInstance instance;
  VkDebugUtilsMessengerEXT messenger;
};

bool initInstance(struct Instance *pInstance, bool validate);
void deinitInstance(struct Instance *pInstance);