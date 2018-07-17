#pragma once

struct ShaderBindingDescriptor {
  const GLuint gl_binding;
  const glUtils::RawBuffer& buffer;
  const glUtils::BufferRange range;
};

static void bind_ubo   (const glUtils::Shader&, glUtils::ShaderBlock& block, const ShaderBindingDescriptor&);
static void bind_ssbo  (const glUtils::Shader&, glUtils::ShaderBlock& block, const ShaderBindingDescriptor&);
static void bind_to_binding(const char*const buffer_type_str, GLenum buffer_type,
     	GLuint binding_index, const glUtils::RawBuffer&, const glUtils::BufferRange);

namespace glUtils {

  void set_shader_buffer(Shader& shader, GLuint gl_binding, UniformName name,
    const RawBuffer& buffer, BufferRange range)
  {
    const auto block_ubo = shader.get_block(name, true);
    if (block_ubo) {
      glUtils::set_shader_buffer(shader, gl_binding, *block_ubo, buffer, range);
    } else {
      const auto block_ssbo = shader.get_block(name, false);
      if (block_ssbo) {
        glUtils::set_shader_buffer(shader, gl_binding, *block_ssbo, buffer, range);
      } else {
        GFX_FAIL("Requested to set bindings for shader buffer ",
          name, ", but there is no such UBO/SSBO");
      }
    }
  }

  void set_shader_buffer(Shader& shader, GLuint gl_binding, ShaderBlock& block,
      const RawBuffer& buffer, BufferRange range)
  {
    // TODO type is not needed, just read from block
    ShaderBindingDescriptor tmp = {gl_binding, buffer, range};
    if (block.is_ubo) {
      bind_ubo(shader, block, tmp);
    } else {
      bind_ssbo(shader, block, tmp);
    }
  }

  void set_shader_atomic(const Shader& shader, u32 gl_binding, const RawBuffer& buffer, u32 offset) {
    const auto u_atomic = shader.get_atomic(gl_binding);

    GFX_FAIL_IF(!u_atomic, "No atomics to set were found on binding ",
      gl_binding, ". Verify they are actually used");

    GFX_FAIL_IF(u_atomic->bytes <= 0, "Could not set atomic on binding",
      gl_binding,  ", the block of atomic_uints has size <=0 based on introspection API");

    // finally, we can do stuff:
    glUtils::BufferRange range = {offset, u_atomic->bytes};
    bind_to_binding("AtomicUInt", glUtils::BufferBindType::AtomicCounterBuffer,
      gl_binding, buffer, range);
  }


} // namespace glUtils


//
// Impl:

void bind_to_binding(const char*const buffer_type_str, GLenum buffer_type,
     	GLuint binding_index, const glUtils::RawBuffer& buffer, const glUtils::BufferRange range)
{
  if (range.offset() + range.size() > buffer.bytes) {
    LOGE << "Could not set " << buffer_type_str << " binding " << binding_index
         << ", tried to bind " << range.size() << " bytes starting from "
         << range.offset() << ", but that would overflow the buffer of size "
         << buffer.bytes << " bytes";
    GFX_FAIL("Binding ", buffer_type_str, " buffer would result in buffer overflow."
      " See above for details");
  }

  GFX_GL_CALL(glBindBufferRange, buffer_type, binding_index, buffer.gl_id,
    range.offset(), range.size());
}


static void get_offset_allignment (GLenum buf_type, i32* value) {
  if (*value == -1) {
    GFX_GL_CALL(glGetIntegerv, buf_type, value);
  }
}

void bind_ubo(const glUtils::Shader& shader, glUtils::ShaderBlock& block, const ShaderBindingDescriptor& uni) {
  GFX_FAIL_IF(block.bytes <= 0,
    "Could not set UBO ", block.name, ", the block has size <=0 based on introspection API");

  static i32 offset_alignment = -1;
  get_offset_allignment(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &offset_alignment);
  if (uni.range.offset() % offset_alignment != 0) {
    LOGE << "Offset given to uniform block buffer " << block.name
         << " must be multiplicity of GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT("
         << offset_alignment << "), " << uni.range.offset() << " given";
    GFX_FAIL("Incorrect offset given when setting ubo data, see above for deatils");
  }

  // finally, we can do stuff:
  // set buffer->binding
  glUtils::BufferRange range = {uni.range.offset(), block.bytes};
  bind_to_binding("UBO", glUtils::BufferBindType::UniformBuffer,
    uni.gl_binding, uni.buffer, range);
  // set binding<-shader resource
  if (block.gl_binding != uni.gl_binding) {
    GFX_GL_CALL(glUniformBlockBinding, shader.gl_id, block.gl_index, uni.gl_binding);
    block.gl_binding = uni.gl_binding;
  }
}

void bind_ssbo(const glUtils::Shader& shader, glUtils::ShaderBlock& block, const ShaderBindingDescriptor& uni) {
  GFX_FAIL_IF(uni.range.size() <= 0, "Size given to SSBO block buffer ", block.name,
    " must have size > 0, provided: ", uni.range.size());

  static i32 offset_alignment = -1;
  get_offset_allignment(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &offset_alignment);
  if (uni.range.offset() % offset_alignment != 0) {
    LOGE << "Offset given to SSBO block buffer " << block.name
         << " must be multiplicity of GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT("
         << offset_alignment << "), " << uni.range.offset() << " given";
    GFX_FAIL("Incorrect offset given when setting ssbo data, see above for deatils");
  }

  // finally, we can do stuff
  // set buffer->binding
  bind_to_binding("SSBO", glUtils::BufferBindType::ShaderStorageBuffer,
    uni.gl_binding, uni.buffer, uni.range);
  // set binding<-shader resource
  if (block.gl_binding != uni.gl_binding) {
    GFX_GL_CALL(glShaderStorageBlockBinding, shader.gl_id, block.gl_index, uni.gl_binding);
    block.gl_binding = uni.gl_binding;
  }
}
