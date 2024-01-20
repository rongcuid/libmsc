#include "pipeline.h"

// Note that this is always aligned to 4-byte
typedef struct {
  u32 size;
  const char code[];
} Spv;

////// Pipeline definitions
static const Spv vertSpv = {
    .size = sizeof(vertSpv) - sizeof(u32),
    .code =
        {
#include "nk.vert.spv.h"
        },
};
static const Spv fragSpv = {
    .size = sizeof(fragSpv) - sizeof(u32),
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
static const u32 dsBindings0Len =
    sizeof(dsBindings0) / sizeof(VkDescriptorSetLayoutBinding);
static const u32 dslBinding0Flags[] = {
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
static const u32 dsBindings1Len =
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

typedef struct {
  VkShaderModule vertModule;
  VkShaderModule fragModule;
  VkPipelineShaderStageCreateInfo stages[2];
  b32 ok;
} ShaderStagesCreated;
static ShaderStagesCreated createShaderStages(VkDevice device) {
  ShaderStagesCreated result = {0};
  // Create stages
  VkShaderModuleCreateInfo vertCI = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = vertSpv.size,
      .pCode = (u32 *)vertSpv.code,
  };
  if (vkCreateShaderModule(device, &vertCI, NULL, &result.vertModule)) {
    goto finally;
  }
  VkShaderModuleCreateInfo fragCI = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = fragSpv.size,
      .pCode = (u32 *)fragSpv.code,
  };
  if (vkCreateShaderModule(device, &fragCI, NULL, &result.fragModule)) {
    goto clean_vert;
  }
  // Create stage create infos
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
clean_frag:
  if (!result.ok) vkDestroyShaderModule(device, result.fragModule, NULL);
clean_vert:
  if (!result.ok) vkDestroyShaderModule(device, result.vertModule, NULL);
finally:
  return result;
}

typedef struct {
  VkDescriptorSetLayout *items;
  u32 len;
  b32 ok;
} DSLayoutsCreated;
static DSLayoutsCreated createDescriptorSetLayouts(VkDevice device) {
  DSLayoutsCreated result = {0};
  result.items = SDL_calloc(2, sizeof(VkDescriptorSetLayout));
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
clean_set1:
  if (!result.ok) vkDestroyDescriptorSetLayout(device, result.items[1], NULL);
clean_set0:
  if (!result.ok) vkDestroyDescriptorSetLayout(device, result.items[0], NULL);
clean_items:
  if (!result.ok) SDL_free(result.items);
finally:
  return result;
}

typedef struct {
  VkPipelineLayout value;
  b32 ok;
} PipelineLayoutCreated;
static PipelineLayoutCreated createPipelineLayout(
    VkDevice device, const VkDescriptorSetLayout *setLayouts, u32 setLen) {
  PipelineLayoutCreated result = {0};
  VkPipelineLayoutCreateInfo ci = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = setLen,
      .pSetLayouts = setLayouts,
  };
  if (vkCreatePipelineLayout(device, &ci, NULL, &result.value)) {
    goto finally;
  }
  result.ok = true;
finally:
  return result;
}

NkPipelineCreated nkCreatePipeline(VkDevice device, VkPipelineCache cache,
                                   VkFormat format) {
  NkPipelineCreated result = {0};
  ShaderStagesCreated stages = createShaderStages(device);
  if (!stages.ok) goto finally;
  result.value.vert = stages.vertModule;
  result.value.frag = stages.fragModule;
  DSLayoutsCreated dsLayouts = createDescriptorSetLayouts(device);
  if (!dsLayouts.ok) goto clean_stages;
  result.value.setLayouts.items = dsLayouts.items;
  result.value.setLayouts.len = dsLayouts.len;
  VkPipelineRenderingCreateInfoKHR rendering = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &format,
  };
  PipelineLayoutCreated layout =
      createPipelineLayout(device, dsLayouts.items, dsLayouts.len);
  if (!layout.ok) goto clean_dslayout;
  result.value.layout = layout.value;
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
      .layout = layout.value,
  };
  if (!vkCreateGraphicsPipelines(device, cache, 1, &ci, NULL,
                                 &result.value.pipeline)) {
    goto clean_layout;
  }
  result.ok = true;
clean_layout:
  if (!result.ok) vkDestroyPipelineLayout(device, layout.value, NULL);
clean_dslayout:
  if (!result.ok)
    for (u32 i = 0; i < dsLayouts.len; ++i)
      vkDestroyDescriptorSetLayout(device, dsLayouts.items[i], NULL);
clean_stages:
  if (!result.ok) {
    vkDestroyShaderModule(device, stages.vertModule, NULL);
    vkDestroyShaderModule(device, stages.fragModule, NULL);
  }
finally:
  return result;
}