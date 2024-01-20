#include "nk_layer.h"

#include <volk/volk.h>

#include "renderer/vk/pipeline.h"

static const char vert[] = {
#include "nk.vert.spv.h"
};
static const SpvCode vertSpv = {
    .spv = vert,
    .size = sizeof(vert),
};
static const char frag[] = {
#include "nk.frag.spv.h"
};
static const SpvCode fragSpv = {
    .spv = frag,
    .size = sizeof(frag),
};

struct NkLayer {
  Pipeline pipeline;
  VkSurfaceKHR attachment;
};

NkLayerCreated nkLayerCreate(VkDevice device, VkPipelineCache cache,
                             VkSurfaceKHR surface) {}