#pragma once

#include <volk/volk.h>

typedef struct {
  VkPipeline pipeline;
  VkPipelineLayout layout;
  struct {
    VkDescriptorSetLayout* items;
    uint32_t len;
  } setLayouts;
  VkShaderModule vert;
  VkShaderModule frag;
} NkPipeline;

typedef struct {
  NkPipeline value;
  bool ok;
} NkPipelineCreated;
NkPipelineCreated nkCreatePipeline(VkDevice device, VkPipelineCache cache,
                                   VkFormat format);