#pragma once

#include <volk.h>

struct Instance {
  VkInstance instance;
  VkDebugUtilsMessengerEXT messenger;
};

bool createInstance(struct Instance *pInstance, bool validate);
