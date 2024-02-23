#pragma once

#include <msc_arena.h>
#include <volk.h>

struct NkPipeline {
  VkDevice device;
  VkPipeline pipeline;
  VkFormat format;
  VkPipelineLayout layout;
  struct {
    uint32_t len;
    VkDescriptorSetLayout* items;
  } setLayouts;
  VkShaderModule vert;
  VkShaderModule frag;
};

bool initNkPipeline(struct NkPipeline* pipeline, VkDevice device,
                    VkPipelineCache cache, VkFormat format,
                    struct msca scratch);
void deinitNkPipeline(struct NkPipeline* pPipeline);