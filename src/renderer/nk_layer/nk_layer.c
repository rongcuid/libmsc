#include "nk_layer.h"

#include <volk.h>

#include "pipeline.h"

struct NkLayer_T {
  VkDevice device;
  VkPipelineCache cache;
  struct NkPipeline pipeline;
};

bool createNkLayer(NkLayer *pLayer, VkDevice device, VkPipelineCache cache,
                   VkFormat format, struct msca scratch) {
  SDL_Log("Creating Nk layer");
  bool ok = false;
  NkLayer layer = SDL_malloc(sizeof(*layer));
  if (!layer) goto finally;
  layer->device = device;
  layer->cache = cache;
  if (!initNkPipeline(&layer->pipeline, device, cache, format, scratch))
    goto err_create_pipeline;
ok:
  ok = true;
  *pLayer = layer;
cleanup:
err_create_pipeline:
  if (!ok) SDL_free(layer);
finally:
  return ok;
}

void destroyNkLayer(NkLayer layer) {
  SDL_Log("Destroying Nk layer");
  deinitNkPipeline(&layer->pipeline);
  SDL_free(layer);
}