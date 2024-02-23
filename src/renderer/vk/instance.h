#pragma once

#include <msc_arena.h>
#include <volk.h>

struct Instance {
  VkInstance instance;
  VkDebugUtilsMessengerEXT messenger;
};

bool initInstance(struct Instance *pInstance, bool validate,
                  struct msca scratch);
void deinitInstance(struct Instance *pInstance);