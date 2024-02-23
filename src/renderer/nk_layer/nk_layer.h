#pragma once

#include <msc_arena.h>
#include <volk.h>

typedef struct NkLayer_T *NkLayer;

bool createNkLayer(NkLayer *pLayer, VkDevice device, VkPipelineCache cache,
                   VkFormat format, struct msca scratch);
void destroyNkLayer(NkLayer layer);