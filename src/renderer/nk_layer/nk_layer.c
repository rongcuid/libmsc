#include "nk_layer.h"

#include <volk.h>

#include "pipeline.h"

struct NkLayer {
  VkPipelineCache cache;
  NkPipeline pipeline;
  VkSurfaceKHR attachment;
};

NkLayerCreated nkLayerCreate(VkDevice device, VkPipelineCache cache,
                             VkSurfaceKHR surface) {}