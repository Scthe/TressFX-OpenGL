#pragma once

#define GET_VARIABLE(VAR_NAME, ...)               \
  auto VAR_NAME = sh.get_uniform(name); \
  GFX_FAIL_IF(!VAR_NAME && force,            \
    "Tried to set uniform '", name, "', but uniform with this name does not exist. ",  \
    "Normally this is allowed, but parameter 'forced' was true, so we crash instead"); \
  if (!VAR_NAME) { return __VA_ARGS__; }

static void verify_type_match (bool forced, GLenum exp_type, const glUtils::ShaderVariable& var) {
  if (!forced) { return; }
  GFX_FAIL_IF(exp_type != var.gl_type, "Tried to set uniform '", var.name, "' that has type ",
    exp_type, ", but provided ", var.gl_type, ". Check if all types match. "
    "Normally this is allowed, but parameter 'forced' was true, so we crash instead");
}

namespace glUtils {

  u32 set_uniform(const Shader& sh, const UniformName name, void* data, bool force) {
    GET_VARIABLE(variable, 0);
    return set_uniform(*variable, data);
  }

	u32 set_uniform(const ShaderVariable& uni, void* data) {
    u32 consumed_bytes = 0;

    #define UNIFORM_MACRO(CPP_TYPE, GL_ENUM, GL_SET_FN, VALUE_EXTRACT) \
      case GL_ENUM: glUtils::set_uniform(uni, uniform_convert<CPP_TYPE>(data, consumed_bytes)); break;
    #define UNIFY_UNIFORM_MACRO

    switch (uni.gl_type) {
      #include "../../include/gl-utils/uniform.types.hpp"
      default:
        GFX_FAIL("Unsupported uniform type for set_uniform(ShaderVariable, void*), given: ", uni.gl_type);
    }
    #undef UNIFY_UNIFORM_MACRO
    #undef UNIFORM_MACRO

    return consumed_bytes;
  }

	#define UNIFORM_MACRO(CPP_TYPE, GL_TYPE, GL_SET_FN, VALUE_EXTRACT) \
	  void set_uniform(const Shader& sh, const UniformName name, const CPP_TYPE v, bool force){ \
      GET_VARIABLE(variable); set_uniform(*variable, v, force); } \
	  void set_uniform(const ShaderVariable& uni, const CPP_TYPE v, bool force) { \
      verify_type_match(force, GL_TYPE, uni); \
      GFX_GL_CALL(GL_SET_FN, uni.gl_location, 1, VALUE_EXTRACT); \
    }

  #define UNIFORM_MACRO_MAT(CPP_TYPE, GL_TYPE, GL_SET_FN, VALUE_EXTRACT) \
	  void set_uniform(const Shader& sh, const UniformName name,   const CPP_TYPE v, bool transpose, bool force){ \
      GET_VARIABLE(variable); set_uniform(*variable, v, transpose, force); } \
	  void set_uniform(const ShaderVariable& uni, const CPP_TYPE v, bool transpose, bool force) { \
      verify_type_match(force, GL_TYPE, uni); \
      GFX_GL_CALL(GL_SET_FN, uni.gl_location, 1, transpose, VALUE_EXTRACT); \
    }

  #include "../../include/gl-utils/uniform.types.hpp"

  #undef UNIFORM_MACRO
	#undef UNIFORM_MACRO_MAT

}; // namespace glUtils


#undef GET_VARIABLE
