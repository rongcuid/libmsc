#pragma once

#include <volk.h>

typedef struct {
  VkInstance instance;
  VkDebugUtilsMessengerEXT messenger;
  bool ok;
} InstanceCreated;
InstanceCreated instanceCreate(bool validate);
