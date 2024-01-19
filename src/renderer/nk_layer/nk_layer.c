#include "nk_layer.h"

#include <volk/volk.h>

#include "renderer/vk/pipeline.h"

struct NkLayer {
  Pipeline pipeline;
  VkSurfaceKHR attachment;
};

NkLayerCreated nkLayerCreate(VkDevice device, VkPipelineCache cache,
                             VkSurfaceKHR surface) {}