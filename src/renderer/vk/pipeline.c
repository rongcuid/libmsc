#include "pipeline.h"

#include <spirv_cross/spirv_cross_c.h>

static void spvcErrCallback(void *pUserData, const char *msg) {
  (void)pUserData;
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", msg);
}

b32 pipelineInitFromShaders(Pipeline *pipeline, VkDevice device,
                            VkPipelineCache cache, const uint32_t *vertSpv,
                            u32 vertSpvSize, const uint32_t *fragSpv,
                            u32 fragSpvSize) {
  b32 result = false;
  usize vertWordCount = vertSpvSize / sizeof(SpvId);
  usize fragWordCount = fragSpvSize / sizeof(SpvId);
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
  if (spvc_context_parse_spirv(context, vertSpv, vertWordCount, &ir) !=
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
  spvc_compiler_create_shader_resources(compiler, &resources);
  spvc_resources_get_resource_list_for_type(
      resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &list, &count);
  for (usize i = 0; i < count; ++i) {
    const spvc_reflected_resource *res = &list[i];
  }
  result = true;
clean_context:
  spvc_context_destroy(context);
finally:
  return result;
}