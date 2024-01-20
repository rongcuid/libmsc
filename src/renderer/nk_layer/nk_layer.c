#include "nk_layer.h"

#include <volk/volk.h>

#include "pipeline.h"

struct NkLayer {
  VkPipelineCache cache;
  NkPipeline pipeline;
  VkSurfaceKHR attachment;
};

NkLayerCreated nkLayerCreate(VkDevice device, VkPipelineCache cache,
                             VkSurfaceKHR surface) {}