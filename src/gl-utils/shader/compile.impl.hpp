#pragma once

static const u32 SHADER_TYPE_COUNT = 6;

struct ShaderTypeCompileProgress;
static void delete_used_shader_types (const GLuint& program_id, ShaderTypeCompileProgress*);
static bool link_program (const GLuint& program_id, glUtils::ShaderErrorsScratchpad&);
static bool compile_shader_type (ShaderTypeCompileProgress&, glUtils::ShaderErrorsScratchpad&);

struct ShaderTypeCompileProgress {
  const char* const human_name = nullptr;
  GLenum gl_shader_type = 0; // GL_VERTEX_SHADER/GL_FRAGMENT_SHADER etc.
  glUtils::ShaderTexts::ShaderText text = nullptr;
  GLuint gl_shader_id = 0;
  bool used = false;

  void discard (GLuint shader) {
    if (this->used) {
      GFX_GL_CALL(glDetachShader, shader, gl_shader_id);
      GFX_GL_CALL(glDeleteShader, gl_shader_id);
    }
    this->used = false;
  }
};

static bool compile_program(const glUtils::ShaderTexts& sources, GLuint& program_id, glUtils::ShaderErrorsScratchpad& es){
  ShaderTypeCompileProgress shader_types[] = {
    {"vertex_shader",                  GL_VERTEX_SHADER,          sources.vertex},
    {"fragment_shader",                GL_FRAGMENT_SHADER,        sources.fragment},
    {"geometry_shader",                GL_GEOMETRY_SHADER,        sources.geometry},
    {"tessellation_control_shader",    GL_TESS_CONTROL_SHADER,    sources.tessellation_control},
    {"tessellation_evaluation_shader", GL_TESS_EVALUATION_SHADER, sources.tessellation_evaluation},
    {"compute_shader",                 GL_COMPUTE_SHADER,         sources.compute}
  };

  // compile all shader stages
  for (size_t i = 0; i < SHADER_TYPE_COUNT; i++) {
    auto& shader_type = shader_types[i];

    if (compile_shader_type(shader_type, es)) {
      if (shader_type.used) {
        GFX_GL_CALL(glAttachShader, program_id, shader_type.gl_shader_id);
      }
    } else {
      delete_used_shader_types(program_id, shader_types);
      GFX_GL_CALL(glDeleteProgram, program_id);
      return false;
    }
  }

  if (link_program(program_id, es)) {
    delete_used_shader_types(program_id, shader_types);
    return true;
  } else {
    delete_used_shader_types(program_id, shader_types); // regardless if ok/false we have to cleanup
    GFX_GL_CALL(glDeleteProgram, program_id);
    return false;
  }
}

///
/// Impl:

static u32 print_error_prefix (const char*const prefix, std::string& str, u32 req_mem) {
  auto name_len = strlen(prefix),
       reserved = name_len + 5;
  str.resize(req_mem + reserved, ' ');
  str[0] = '[';
  str.replace(1, 0, prefix, name_len); // wtf is going on with gcc impl? why is len allowed to be?
  str[name_len + 1] = ']';
  str[name_len + 2] = ' ';
  return name_len + 3;
}

bool compile_shader_type (ShaderTypeCompileProgress& shader, glUtils::ShaderErrorsScratchpad& es) {
  // early out if no text provided
  if(!shader.text || *shader.text == '\0' || *shader.text == 0){
    return true;
  }

  shader.gl_shader_id = GFX_GL_CALL(glCreateShader, shader.gl_shader_type);
  GFX_GL_CALL(glShaderSource, shader.gl_shader_id, 1, (const GLchar **)&shader.text, nullptr);
  GFX_GL_CALL(glCompileShader, shader.gl_shader_id);

  GLint res;
  GFX_GL_CALL(glGetShaderiv, shader.gl_shader_id, GL_COMPILE_STATUS, &res);

  if (res == GL_FALSE) {
    GLint req_mem = 0;
    GFX_GL_CALL(glGetShaderiv, shader.gl_shader_id, GL_INFO_LOG_LENGTH, &req_mem);
    auto offset = print_error_prefix(shader.human_name, es.msg, req_mem);
    GFX_GL_CALL(glGetShaderInfoLog, shader.gl_shader_id, req_mem, nullptr, es.msg.data() + offset);

    GFX_GL_CALL(glDeleteShader, shader.gl_shader_id);
    return false;
  } else {
    shader.used = true;
    return true;
  }
}

void delete_used_shader_types (const GLuint& program_id, ShaderTypeCompileProgress* statuses) {
  for (size_t j = 0; j < SHADER_TYPE_COUNT; j++) {
    if (statuses[j].used) {
      statuses[j].discard(program_id);
    }
  }
}

bool link_program (const GLuint& program_id, glUtils::ShaderErrorsScratchpad& es) {
  GFX_GL_CALL(glLinkProgram, program_id);

  GLint res;
  GFX_GL_CALL(glGetProgramiv, program_id, GL_LINK_STATUS, &res);

  if (res == GL_FALSE) {
    GLint req_mem = 0;
    GFX_GL_CALL(glGetProgramiv, program_id, GL_INFO_LOG_LENGTH, &req_mem);
    auto offset = print_error_prefix("linking", es.msg, req_mem);
    GFX_GL_CALL(glGetProgramInfoLog, program_id, req_mem, nullptr, es.msg.data() + offset);

    return false;
  } else {
    return true;
  }
}
