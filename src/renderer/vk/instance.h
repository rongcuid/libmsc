#pragma once

#include <volk.h>

typedef struct {
  VkInstance instance;
  VkDebugUtilsMessengerEXT messenger;
  b32 ok;
} InstanceCreated;
InstanceCreated instanceCreate(b32 validate);
