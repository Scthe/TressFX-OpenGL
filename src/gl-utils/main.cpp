#include "../../include/gl-utils/main.hpp"

#include "buffer/buffer.impl.hpp"
#include "buffer/copy_to.impl.hpp"
#include "buffer/malloc.impl.hpp"
#include "buffer/read.impl.hpp"
#include "buffer/write.impl.hpp"
#include "draw/apply_parameters.impl.hpp"
#include "texture/allocate_texture.impl.hpp"
#include "texture/apply_texture_options.impl.hpp"
#include "setShaderBuffer.impl.hpp"
#include "shader/shader.impl.hpp"
#include "uniforms.impl.hpp"
#include "vao.impl.hpp"
#include "window.impl.hpp"
#include "debug_callback.impl.hpp"


namespace glUtils {

  void async_write_barrier (AsyncWriteableResource_ resource_type) {
    GFX_GL_CALL(glMemoryBarrier, resource_type);
  }

} // namespace glUtils
