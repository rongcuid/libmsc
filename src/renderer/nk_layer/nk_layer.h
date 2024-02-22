#pragma once

#include <volk.h>

typedef struct NkLayer NkLayer;

typedef struct {
  NkLayer *value;
  bool ok;
} NkLayerCreated;
NkLayerCreated nkLayerCreate(VkDevice device, VkPipelineCache cache,
                             VkSurfaceKHR surface);