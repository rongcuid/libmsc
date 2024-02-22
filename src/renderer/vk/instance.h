#pragma once

#include <volk/volk.h>

typedef struct {
  VkInstance instance;
  VkDebugUtilsMessengerEXT messenger;
  bool ok;
} InstanceCreated;
InstanceCreated instanceCreate(bool validate);
