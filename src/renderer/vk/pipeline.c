#include "pipeline.h"

#include <spirv_cross/spirv_cross_c.h>

static void spvcErrCallback(void *pUserData, const char *msg) {
  (void)pUserData;
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", msg);
}

typedef struct {
  b32 ok;
} ShaderResources;
ShaderResources getShaderResources(const char *spv, usize size) {
  ShaderResources result = {0};
  if ((uptr)spv % alignof(SpvId) != 0) {
    goto finally;
  }
  // Create compiler
  const SpvId *spirv = (const SpvId *)spv;
  usize wordCount = size / sizeof(SpvId);
  spvc_context context = NULL;
  spvc_parsed_ir ir = NULL;
  spvc_compiler compiler = NULL;
  spvc_resources resources = NULL;
  const spvc_reflected_resource *list = NULL;
  const char *spvcResult = NULL;
  usize count;
  if (spvc_context_create(&context) != SPVC_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create SPVC context");
    goto finally;
  }
  spvc_context_set_error_callback(context, &spvcErrCallback, NULL);
  if (spvc_context_parse_spirv(context, spirv, wordCount, &ir) !=
      SPVC_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to parse vertex SPV");
    goto clean_context;
  }
  if (spvc_context_create_compiler(context, SPVC_BACKEND_NONE, ir,
                                   SPVC_CAPTURE_MODE_TAKE_OWNERSHIP,
                                   &compiler) != SPVC_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to create SPVC compiler");
    goto clean_context;
  }
  // Get shader resources
  spvc_compiler_create_shader_resources(compiler, &resources);
  spvc_resources_get_resource_list_for_type(
      resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &list, &count);
  for (usize i = 0; i < count; ++i) {
    const spvc_reflected_resource *res = &list[i];
  }
clean_context:
  spvc_context_destroy(context);
finally:
  return result;
}

b32 pipelineInitFromShaders(Pipeline *pipeline, VkDevice device,
                            VkPipelineCache cache, SpvCode vert, SpvCode frag) {
  b32 result = false;
  ShaderResources vertRsc = getShaderResources(vert.spv, vert.size);
  if (!vertRsc.ok) {
    goto finally;
  }
  ShaderResources fragRsc = getShaderResources(frag.spv, frag.size);
  if (!fragRsc.ok) {
    goto clean_vert_rsc;
  }
  result = true;
clean_vert_rsc:
finally:
  return result;
}