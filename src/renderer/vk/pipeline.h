#pragma once

#include <volk.h>

typedef struct Pipeline {
  VkPipeline pipeline;
  VkPipelineLayout layout;
} Pipeline;

b32 pipelineInitFromShaders(Pipeline *pipeline, VkDevice device,
                            VkPipelineCache cache, const uint32_t *vertSpv,
                            u32 vertSpvSize, const uint32_t *fragSpv,
                            u32 fragSpvSize);