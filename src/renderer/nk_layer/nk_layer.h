#pragma once

#include <volk/volk.h>

typedef struct NkLayer NkLayer;

typedef struct {
  NkLayer *value;
  b32 ok;
} NkLayerCreated;
NkLayerCreated nkLayerCreate(VkDevice device, VkPipelineCache cache,
                             VkSurfaceKHR surface);