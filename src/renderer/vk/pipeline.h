#pragma once

#include <volk/volk.h>

typedef struct Pipeline {
  VkPipeline pipeline;
  VkPipelineLayout layout;
} Pipeline;

typedef struct SpvCode {
  const char *spv;
  u32 size;
} SpvCode;

b32 pipelineInitFromShaders(Pipeline *pipeline, VkDevice device,
                            VkPipelineCache cache, SpvCode vert, SpvCode frag);