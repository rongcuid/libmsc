#pragma once

#include <msc_arena.h>
#include <volk.h>

struct NkPipeline {
  VkPipeline pipeline;
  VkPipelineLayout layout;
  struct {
    uint32_t len;
    VkDescriptorSetLayout* items;
  } setLayouts;
  VkShaderModule vert;
  VkShaderModule frag;
};

bool nkCreatePipeline(VkDevice device, VkPipelineCache cache, VkFormat format,
                      struct msca scratch, struct NkPipeline* pPipeline);