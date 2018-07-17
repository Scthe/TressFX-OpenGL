// https://www.khronos.org/opengl/wiki/Program_Introspection
// https://www.khronos.org/opengl/wiki/GLAPI/glGetProgramResource
// https://www.geometrictools.com/GTEngine/Source/Graphics/GL4/GteGLSLReflection.cpp

#define INIT_PROP_VALUE (99999)

// shader_name_hash
static unsigned long sdbm(const char* str){
  unsigned long hash = 0;
  while (int c = *str++){
      hash = c + (hash << 6) + (hash << 16) - hash;
  }
  return hash;
}
#define shader_name_hash(str) sdbm((str).data())

static const i32 VARIABLE_NOT_IN_BLOCK = -1;

struct ResourceCount {
  // variables:
  i32 attr_in;
  i32 attr_out;
  i32 all_uniforms; // both block and 'standalone'
  i32 ssbo_members;
  // blocks:
  i32 ubo;
  i32 ssbo;
  // atomic
  i32 atomics;
};

static ResourceCount get_resource_count (GLuint program) {
  ResourceCount res_cnt;
  // variables
  GFX_GL_CALL(glGetProgramInterfaceiv, program, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &res_cnt.attr_in);
  GFX_GL_CALL(glGetProgramInterfaceiv, program, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &res_cnt.attr_out);
  GFX_GL_CALL(glGetProgramInterfaceiv, program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &res_cnt.all_uniforms);
  GFX_GL_CALL(glGetProgramInterfaceiv, program, GL_BUFFER_VARIABLE, GL_ACTIVE_RESOURCES, &res_cnt.ssbo_members);
  // blocks
  GFX_GL_CALL(glGetProgramInterfaceiv, program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &res_cnt.ubo);
  GFX_GL_CALL(glGetProgramInterfaceiv, program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &res_cnt.ssbo);
  // atomic
  GFX_GL_CALL(glGetProgramInterfaceiv, program, GL_ATOMIC_COUNTER_BUFFER, GL_ACTIVE_RESOURCES, &res_cnt.atomics);

  return res_cnt;
}

#define get_resource_name(VAR_NAME, RES_TYPE, LEN) do {\
    if (LEN < 1) { \
      GFX_GL_CALL(glGetProgramInterfaceiv, program, RES_TYPE, GL_MAX_NAME_LENGTH, &LEN); \
    } \
    GFX_FAIL_IF(LEN < 1, "Shader resource has name with length <1? This should not be possible") \
    VAR_NAME.name.resize((LEN)); \
    GFX_GL_CALL(glGetProgramResourceName, program, RES_TYPE, i, (LEN), nullptr, VAR_NAME.name.data()); \
    VAR_NAME.name_hash = shader_name_hash(VAR_NAME.name); \
  } while(0)

// <editor-fold attributes>

static const GLenum ATTRIBUTE_PROPERTIES[] = {
  GL_LOCATION,
  GL_TYPE,
  GL_ARRAY_SIZE,
  GL_NAME_LENGTH
};
static const u32 ATTRIBUTE_PROPERTIES_LEN = 4;

static void read_attributes(GLuint program, glUtils::ShaderVariables& arr,
    u32 count, glUtils::ShaderVariableType variable_type)
{
  GLenum gl_attr_type = variable_type == glUtils::ShaderVariableType::AttributeIn ? GL_PROGRAM_INPUT : GL_PROGRAM_OUTPUT;

  for (u32 i = 0; i < count; i++) {
    glUtils::ShaderVariable attr;

    GLint values[ATTRIBUTE_PROPERTIES_LEN] = {INIT_PROP_VALUE, INIT_PROP_VALUE, INIT_PROP_VALUE, INIT_PROP_VALUE};
    GFX_GL_CALL(glGetProgramResourceiv, program, gl_attr_type, i, ATTRIBUTE_PROPERTIES_LEN, ATTRIBUTE_PROPERTIES, ATTRIBUTE_PROPERTIES_LEN, nullptr, values);
    attr.gl_location = values[0];
    attr.gl_type = values[1];
    attr.array_size = values[2];
    attr.variable_type = variable_type;
    attr.gl_index_of_parent_block = VARIABLE_NOT_IN_BLOCK;

    get_resource_name(attr, gl_attr_type, values[3]);

    arr.push_back(attr);
  }
}

// </editor-fold>


// <editor-fold blocks>

static const GLenum BLOCK_PROPERTIES[] = {
  GL_BUFFER_BINDING,
  GL_BUFFER_DATA_SIZE,
  GL_NAME_LENGTH
};
static const u32 BLOCK_PROPERTIES_LEN = 3;

static void read_blocks (GLuint program, glUtils::ShaderBlocks& arr, u32 count, bool is_ubo) {
  auto gl_block_type = is_ubo ? GL_UNIFORM_BLOCK : GL_SHADER_STORAGE_BLOCK;

  for (u32 i = 0; i < count; i++) {
    glUtils::ShaderBlock block;

    i32 values[BLOCK_PROPERTIES_LEN] = {INIT_PROP_VALUE, INIT_PROP_VALUE, INIT_PROP_VALUE};
    GFX_GL_CALL(glGetProgramResourceiv, program, gl_block_type,
        i,
        BLOCK_PROPERTIES_LEN, BLOCK_PROPERTIES, BLOCK_PROPERTIES_LEN,
        nullptr, values);
    block.is_ubo = is_ubo;
    block.gl_index = i;
    block.gl_binding = (u32) values[0];
    block.bytes = (u32) values[1];

    get_resource_name(block, gl_block_type, values[2]);

    // LOGI << block.name
         // << "(index=" << values[0]
         // << ", binding=" << values[1] << ")" ;
    arr.push_back(block);
  }
  // TODO check if all bindings set and no repeats
}

// </editor-fold>


// <editor-fold uniforms_and_block_members>

static const GLenum VARIABLE_PROPERTIES[] = {
  GL_TYPE,
  GL_BLOCK_INDEX,
  GL_ARRAY_SIZE,
  GL_NAME_LENGTH,
  GL_LOCATION // SSBO variables do not have location
};

static void read_uniforms_and_block_members (GLuint program,
  glUtils::ShaderVariables& arr, u32 count, bool is_uniforms)
{
  GLsizei prop_count = is_uniforms ? 5 : 4;
  auto gl_variable_type = is_uniforms ? GL_UNIFORM : GL_BUFFER_VARIABLE;

  // read variable data
  for (u32 i = 0; i < count; i++) {
    glUtils::ShaderVariable uni_var;

    GLint values[5] = {INIT_PROP_VALUE, INIT_PROP_VALUE, INIT_PROP_VALUE, INIT_PROP_VALUE, INIT_PROP_VALUE};
    GFX_GL_CALL(glGetProgramResourceiv, program, gl_variable_type, i, prop_count, VARIABLE_PROPERTIES, prop_count, NULL, values);
    uni_var.gl_location = is_uniforms ? values[4] : 0;
    uni_var.gl_type = values[0];
    uni_var.array_size = values[2];
    uni_var.gl_index_of_parent_block = values[1];
    uni_var.variable_type = uni_var.gl_index_of_parent_block == VARIABLE_NOT_IN_BLOCK
      ? glUtils::ShaderVariableType::Uniform : glUtils::ShaderVariableType::BlockMember;

    get_resource_name(uni_var, gl_variable_type, values[3]);

    arr.push_back(uni_var);

    // print 'vec3 member in block' warning
    auto is_vec3 = uni_var.gl_type == GL_FLOAT_VEC3
                || uni_var.gl_type == GL_INT_VEC3
                || uni_var.gl_type == GL_UNSIGNED_INT_VEC3;
    if (is_vec3 && uni_var.variable_type == glUtils::ShaderVariableType::BlockMember) {
      LOGW << "Variable " << uni_var.name << " is a 3 component vector (float/int/unsigned int) "
           << "that belongs to the shader block (ubo/ssbo). "
           << "This is not recommended, since different glsl packing options "
           << "may 'promote' it to vec4, which conflicts with c++ packing rules. "
           << "Consider using [ ui]vec4 instead";
    }
  }
}

// </editor-fold>


// <editor-fold atomics>

static const GLenum ATOMICS_PROPERTIES[] = {
  GL_BUFFER_BINDING,
  GL_BUFFER_DATA_SIZE,
  GL_NUM_ACTIVE_VARIABLES
};
static const u32 ATOMICS_PROPERTIES_LEN = 3;

static void read_atomics (GLuint program, glUtils::ShaderAtomics& atomics, u32 count) {
  // LOGW << "will read info about " << count << "atomics";
  for (u32 i = 0; i < count; ++i) {
    glUtils::ShaderAtomicUint atomic_uint;

    GLint results[ATOMICS_PROPERTIES_LEN] = {INIT_PROP_VALUE, INIT_PROP_VALUE, INIT_PROP_VALUE};
    GFX_GL_CALL(glGetProgramResourceiv, program, GL_ATOMIC_COUNTER_BUFFER, i, ATOMICS_PROPERTIES_LEN, ATOMICS_PROPERTIES, ATOMICS_PROPERTIES_LEN, nullptr, results);
    atomic_uint.gl_binding = (u32) results[0];
    atomic_uint.bytes = (u32) results[1];
    atomic_uint.member_count = (u32) results[2];
    // LOGW << "atomic(gl_binding="<<results[0]<<", bytes=" << results[1]<< ")";

    atomics.push_back(atomic_uint);
  }
}

// </editor-fold>

#undef get_resource_name
