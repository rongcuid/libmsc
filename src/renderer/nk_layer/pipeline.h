#pragma once

#include <volk/volk.h>

typedef struct {
  VkPipeline pipeline;
  VkPipelineLayout layout;
  struct {
    VkDescriptorSetLayout* items;
    u32 len;
  } setLayouts;
  VkShaderModule vert;
  VkShaderModule frag;
} NkPipeline;

typedef struct {
  NkPipeline value;
  b32 ok;
} NkPipelineCreated;
NkPipelineCreated nkCreatePipeline(VkDevice device, VkPipelineCache cache,
                                   VkFormat format);