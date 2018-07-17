
UNIFORM_MACRO(i32,        GL_INT,               glUniform1iv,  &v)
UNIFORM_MACRO(u32,        GL_UNSIGNED_INT,      glUniform1uiv, &v)
UNIFORM_MACRO(f32,        GL_FLOAT,             glUniform1fv,  &v)
UNIFORM_MACRO(glm::vec2,  GL_FLOAT_VEC2,        glUniform2fv,  glm::value_ptr(v))
UNIFORM_MACRO(glm::ivec2, GL_INT_VEC2,          glUniform2iv,  glm::value_ptr(v))
UNIFORM_MACRO(glm::uvec2, GL_UNSIGNED_INT_VEC2, glUniform2uiv, glm::value_ptr(v))
UNIFORM_MACRO(glm::vec3,  GL_FLOAT_VEC3,        glUniform3fv,  glm::value_ptr(v))
UNIFORM_MACRO(glm::ivec3, GL_INT_VEC3,          glUniform3iv,  glm::value_ptr(v))
UNIFORM_MACRO(glm::uvec3, GL_UNSIGNED_INT_VEC3, glUniform3uiv, glm::value_ptr(v))
UNIFORM_MACRO(glm::vec4,  GL_FLOAT_VEC4,        glUniform4fv,  glm::value_ptr(v))
UNIFORM_MACRO(glm::ivec4, GL_INT_VEC4,          glUniform4iv,  glm::value_ptr(v))
UNIFORM_MACRO(glm::uvec4, GL_UNSIGNED_INT_VEC4, glUniform4uiv, glm::value_ptr(v))

#ifndef UNIFY_UNIFORM_MACRO
// matrices
UNIFORM_MACRO_MAT(glm::mat2, GL_FLOAT_MAT2, glUniformMatrix2fv, glm::value_ptr(v))
UNIFORM_MACRO_MAT(glm::mat3, GL_FLOAT_MAT3, glUniformMatrix3fv, glm::value_ptr(v))
UNIFORM_MACRO_MAT(glm::mat4, GL_FLOAT_MAT4, glUniformMatrix4fv, glm::value_ptr(v))
#else
UNIFORM_MACRO(glm::mat2, GL_FLOAT_MAT2, glUniformMatrix2fv, glm::value_ptr(v))
UNIFORM_MACRO(glm::mat3, GL_FLOAT_MAT3, glUniformMatrix3fv, glm::value_ptr(v))
UNIFORM_MACRO(glm::mat4, GL_FLOAT_MAT4, glUniformMatrix4fv, glm::value_ptr(v))
#endif
