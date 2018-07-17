#pragma once

#include "compile.impl.hpp"
#include "introspect.impl.hpp"

static const glUtils::ShaderVariable* get_var(glUtils::ShaderResourceName,
  glUtils::ShaderVariableType*, const glUtils::ShaderVariables&);

namespace glUtils {

  Shader create_shader(const ShaderTexts& texts, ShaderErrorsScratchpad& es) {
    Shader shader;
    shader.gl_id = GFX_GL_CALL(glCreateProgram);

    if (!compile_program(texts, shader.gl_id, es)) {
      destroy(shader);
      return shader;
    }

    auto res_cnt = get_resource_count(shader.gl_id);
    read_attributes(shader.gl_id, shader.used_variables, res_cnt.attr_in,  ShaderVariableType::AttributeIn);
    read_attributes(shader.gl_id, shader.used_variables, res_cnt.attr_out, ShaderVariableType::AttributeOut);
    read_blocks(shader.gl_id, shader.used_blocks, res_cnt.ubo, true);
    read_blocks(shader.gl_id, shader.used_blocks, res_cnt.ssbo, false);
    read_uniforms_and_block_members(shader.gl_id, shader.used_variables, res_cnt.all_uniforms, true);
    read_uniforms_and_block_members(shader.gl_id, shader.used_variables, res_cnt.ssbo_members, false);
    read_atomics(shader.gl_id, shader.used_atomics, res_cnt.atomics);

    return shader;
  }

  void destroy(Shader& shader) {
    if (!shader.is_created()) {
      return;
    }

    glUseProgram(0); // ugh, just in case..
    GFX_GL_CALL(glDeleteProgram, shader.gl_id);

    shader.gl_id = GL_UTILS_NOT_CREATED_ID;
  }


  const ShaderVariable* Shader::get_attribute(ShaderResourceName name) const noexcept {
    ShaderVariableType svt[] = {ShaderVariableType::AttributeIn, ShaderVariableType::AttributeOut};
    return get_var(name, svt, this->used_variables);
  }

  const ShaderVariable* Shader::get_uniform(ShaderResourceName name) const noexcept {
    ShaderVariableType svt[] = {ShaderVariableType::Uniform, ShaderVariableType::Uniform}; // :)
    return get_var(name, svt, this->used_variables);
  }

  ShaderBlock* Shader::get_block(ShaderResourceName name, bool is_ubo) noexcept {
    auto hash = shader_name_hash(name);
    glUtils::ShaderBlock* result = nullptr;

    for (auto& v : this->used_blocks) {
      if (v.is_ubo == is_ubo && v.name_hash == hash) {
        result = &v;
      }
    }

    return result;
  }

  const ShaderAtomicUint* Shader::get_atomic(u32 gl_binding) const noexcept {
    const ShaderAtomicUint* result = nullptr;

    for (const auto& v : this->used_atomics) {
      if (v.gl_binding == gl_binding) {
        result = &v;
      }
    }

    return result;
  }

} // namespace glUtils

const glUtils::ShaderVariable* get_var(glUtils::ShaderResourceName name, glUtils::ShaderVariableType* types,
    const glUtils::ShaderVariables& variables)
{
  auto hash = shader_name_hash(name);
  const glUtils::ShaderVariable* result = nullptr;

  for (const auto& v : variables) {
    bool match_type = v.variable_type == types[0]
                   || v.variable_type == types[1];
    if (match_type && v.name_hash == hash) {
      result = &v;
    }
  }

  return result;
}
