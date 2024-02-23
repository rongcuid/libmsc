#include "pipeline.h"

// Note that this is always aligned to 4-byte
typedef struct {
  uint32_t size;
  const char code[];
} Spv;

////// Pipeline definitions
static const Spv vertSpv = {
    .size = sizeof(vertSpv) - sizeof(uint32_t),
    .code =
        {
#include "nk.vert.spv.h"
        },
};
static const Spv fragSpv = {
    .size = sizeof(fragSpv) - sizeof(uint32_t),
    .code =
        {
#include "nk.frag.spv.h"
        },
};

static const VkPipelineVertexInputStateCreateInfo visCI = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 0,
    .vertexAttributeDescriptionCount = 0,
};
static const VkPipelineInputAssemblyStateCreateInfo iasCI = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
};
static const VkPipelineTessellationStateCreateInfo tsCI = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
};
static const VkPipelineViewportStateCreateInfo vsCI = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .scissorCount = 1,
};
static const VkPipelineRasterizationStateCreateInfo rsCI = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .lineWidth = 1.0,
};
static const VkPipelineMultisampleStateCreateInfo msCI = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
};
static const VkPipelineDepthStencilStateCreateInfo dssCI = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
};
static const VkPipelineColorBlendStateCreateInfo cbsCI = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments =
        (const VkPipelineColorBlendAttachmentState[]){
            (VkPipelineColorBlendAttachmentState){
                .blendEnable = true,
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask =
                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            },
        },
};
static const VkPipelineDynamicStateCreateInfo dsCI = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = 2,
    .pDynamicStates =
        (const VkDynamicState[]){
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        },
};

////// Descriptor set definitions
static const VkDescriptorSetLayoutBinding dsBindings0[] = {
    {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 32,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    },
};
static const uint32_t dsBindings0Len =
    sizeof(dsBindings0) / sizeof(VkDescriptorSetLayoutBinding);
static const uint32_t dslBinding0Flags[] = {
    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
};
static const VkDescriptorSetLayoutBinding dsBindings1[] = {
    (VkDescriptorSetLayoutBinding){
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        .descriptorType = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    },
};
static const uint32_t dsBindings1Len =
    sizeof(dsBindings1) / sizeof(VkDescriptorSetLayoutBinding);

////// Descriptor set create infos
static const VkDescriptorSetLayoutBindingFlagsCreateInfo dslbfCreateInfos[] = {
    (VkDescriptorSetLayoutBindingFlagsCreateInfo){
        .sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = dsBindings0Len,
        .pBindingFlags = dslBinding0Flags,
    },
    (VkDescriptorSetLayoutBindingFlagsCreateInfo){
        .sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = 0,
        .pBindingFlags = NULL,
    },
};
static const VkDescriptorSetLayoutCreateInfo dslCreateInfos[] = {
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &dslbfCreateInfos[0],
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = dsBindings0Len,
        .pBindings = dsBindings0,
    },
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &dslbfCreateInfos[1],
        .flags = 0,
        .bindingCount = dsBindings1Len,
        .pBindings = dsBindings1,
    },
};

static bool createShaderStages(VkDevice device, struct msca *up,
                               VkShaderModule *pVertModule,
                               VkShaderModule *pFragModule,
                               uint32_t *pStageCICount,
                               VkPipelineShaderStageCreateInfo **ppStageCIs) {
  struct {
    VkShaderModule vertModule;
    VkShaderModule fragModule;
    VkPipelineShaderStageCreateInfo *stages;
    bool ok;
  } result = {0};
  msca_cp_t cp = msca_checkpoint(up);
  // Create stages
  VkShaderModuleCreateInfo vertCI = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = vertSpv.size,
      .pCode = (uint32_t *)vertSpv.code,
  };
  if (vkCreateShaderModule(device, &vertCI, NULL, &result.vertModule)) {
    goto finally;
  }
  VkShaderModuleCreateInfo fragCI = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = fragSpv.size,
      .pCode = (uint32_t *)fragSpv.code,
  };
  if (vkCreateShaderModule(device, &fragCI, NULL, &result.fragModule)) {
    goto clean_vert;
  }
  // Create stage create infos
  result.stages =
      msca_alloc(up, alignof(*result.stages), 2, alignof(*result.stages));
  if (!result.stages) goto clean_frag;
  result.stages[0] = (VkPipelineShaderStageCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = result.vertModule,
      .pName = "main",
  };
  result.stages[1] = (VkPipelineShaderStageCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = result.fragModule,
      .pName = "main",
  };
  result.ok = true;
  *pVertModule = result.vertModule;
  *pFragModule = result.fragModule;
  *pStageCICount = 2;
  *ppStageCIs = result.stages;
clean_frag:
  if (!result.ok) vkDestroyShaderModule(device, result.fragModule, NULL);
clean_vert:
  if (!result.ok) vkDestroyShaderModule(device, result.vertModule, NULL);
finally:
  if (!result.ok) msca_rewind(up, cp);
  return result.ok;
}

static bool createDescriptorSetLayouts(VkDevice device, uint32_t *pLayoutCount,
                                       VkDescriptorSetLayout **ppLayouts) {
  struct {
    VkDescriptorSetLayout *items;
    uint32_t len;
    bool ok;
  } result = {0};
  result.items = SDL_calloc(2, sizeof(VkDescriptorSetLayout));
  if (!result.items) goto finally;
  // Set 0
  if (vkCreateDescriptorSetLayout(device, &dslCreateInfos[0], NULL,
                                  &result.items[0])) {
    goto clean_items;
  }
  // Set 1
  if (vkCreateDescriptorSetLayout(device, &dslCreateInfos[1], NULL,
                                  &result.items[1])) {
    goto clean_set0;
  }
  result.ok = true;
  *pLayoutCount = result.len;
  *ppLayouts = result.items;
clean_set1:
  if (!result.ok) vkDestroyDescriptorSetLayout(device, result.items[1], NULL);
clean_set0:
  if (!result.ok) vkDestroyDescriptorSetLayout(device, result.items[0], NULL);
clean_items:
  if (!result.ok) SDL_free(result.items);
finally:
  return result.ok;
}

static bool createPipelineLayout(VkDevice device,
                                 const VkDescriptorSetLayout *setLayouts,
                                 uint32_t setLen,
                                 VkPipelineLayout *pPipelineLayout) {
  struct {
    VkPipelineLayout value;
    bool ok;
  } result = {0};
  VkPipelineLayoutCreateInfo ci = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = setLen,
      .pSetLayouts = setLayouts,
  };
  if (vkCreatePipelineLayout(device, &ci, NULL, &result.value)) {
    goto finally;
  }
  result.ok = true;
  *pPipelineLayout = result.value;
finally:
  return result.ok;
}

bool nkCreatePipeline(VkDevice device, VkPipelineCache cache, VkFormat format,
                      struct msca scratch, struct NkPipeline *pPipeline) {
  struct {
    struct NkPipeline value;
    bool ok;
  } result = {0};
  struct msca tmp;
  msca_half(&scratch, &tmp);
  struct {
    VkShaderModule vertModule, fragModule;
    uint32_t len;
    VkPipelineShaderStageCreateInfo *stages;
  } stages;
  if (!createShaderStages(device, &tmp, &stages.vertModule, &stages.fragModule,
                          &stages.len, &stages.stages))
    goto finally;
  result.value.vert = stages.vertModule;
  result.value.frag = stages.fragModule;
  if (!createDescriptorSetLayouts(device, &result.value.setLayouts.len,
                                  &result.value.setLayouts.items))
    goto clean_stages;
  VkPipelineRenderingCreateInfoKHR rendering = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &format,
  };

  if (!createPipelineLayout(device, result.value.setLayouts.items,
                            result.value.setLayouts.len, &result.value.layout))
    goto clean_dslayout;
  VkGraphicsPipelineCreateInfo ci = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &rendering,
      .stageCount = 2,
      .pStages = stages.stages,
      .pVertexInputState = &visCI,
      .pInputAssemblyState = &iasCI,
      .pTessellationState = &tsCI,
      .pViewportState = &vsCI,
      .pRasterizationState = &rsCI,
      .pMultisampleState = &msCI,
      .pDepthStencilState = &dssCI,
      .pColorBlendState = &cbsCI,
      .pDynamicState = &dsCI,
      .layout = result.value.layout,
  };
  if (!vkCreateGraphicsPipelines(device, cache, 1, &ci, NULL,
                                 &result.value.pipeline)) {
    goto clean_layout;
  }
  result.ok = true;
  *pPipeline = result.value;
clean_layout:
  if (!result.ok) vkDestroyPipelineLayout(device, result.value.layout, NULL);
clean_dslayout:
  if (!result.ok)
    for (uint32_t i = 0; i < result.value.setLayouts.len; ++i)
      vkDestroyDescriptorSetLayout(device, result.value.setLayouts.items[i],
                                   NULL);
clean_stages:
  if (!result.ok) {
    vkDestroyShaderModule(device, stages.vertModule, NULL);
    vkDestroyShaderModule(device, stages.fragModule, NULL);
  }
finally:
  return result.ok;
}