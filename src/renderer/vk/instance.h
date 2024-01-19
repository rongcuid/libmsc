#pragma once

#include <string.h>
#include <volk.h>

typedef struct {
  VkInstance instance;
  VkDebugUtilsMessengerEXT debug;
  b32 ok;
} InstanceCreated;
InstanceCreated instanceCreate(b32 validate);
