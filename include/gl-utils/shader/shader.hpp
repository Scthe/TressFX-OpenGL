#pragma once

/**
 * (I prefer to use shader as the final result of glsl linking, which referrs
 * to notion of 'program' in normal OpenGL language. IDK, I just prefer the word)
 *
 *
 * Note on shader introspection
 *
 * To each shader we can provide following resources:
 *   - vertex attributes
 *   - uniforms (standalone and block members)
 *   - ssbo
 *   - misc (atomics, transform feedback etc.)
 *
 * When shader is compiled and linked, each of resource is assigned certain
 * indentifier - location for attributes, index for blocks. Resources that are
 * never used are not assigned with this identifier (though attributes can be
 * exception). We can read how many resources for particular type are e.g.
 * use GL_ACTIVE_UNIFORM_BLOCKS to get number of *active/used* UBOs.
 *
 *
 *
 *
 * Note on connecting buffers (UBO, SSBO etc.) to shader variables
 *
 * When connecting ssbo/ubo to buffer we use intermediary 'table' called bindings
 * as follows:
 * 1) bind shader's ubo/ssbo to binding. All ubo/ssbo have the index property, so we call
 *    [glUniformBlockBinding/glShaderStorageBlockBinding](program.gl_id, block.gl_index, binding_id) for this
 * 2) assign buffer to same binding using:
 *    glBindBuffer(GL_UNIFORM_BUFFER, binding_id, buffer.gl_id)
 *    OR
 *    glBindBufferRange(GL_UNIFORM_BUFFER, binding_id, buffer.gl_id, range.offset, range.size)
 *
 *
 *
 * Note on atomics
 *
 * Unfortunatelly, opengl does not allow for querying that much info on atomic uints.
 * Instead of name we have to use raw binding value. We do not get info about each
 * atomic. Instead, we are provided with cumulative info about all atomics
 * that are on this binding.
 *
 * Example:
 * layout(binding = 3, offset = 4) uniform atomic_uint three;
 * layout(binding = 3) uniform atomic_uint seven; // implicit offset 8, cause (last_offset + 4) is the rule
 * layout(binding = 1) uniform atomic_uint four;
 *
 * This would return two AtomicUintBindingData, with gl_binding values of 3 and 1. Data for each binding:
 *  - binding 3: has 2 members and size in bytes 12 (highest offset is 8,
 *    we add 4 bytes to it to acommodate actual uint at this offset)
 *  - binding 1: has 1 member and size in bytes 4 (highest/default offset 0 + 4 bytes for uint)
 *
 * struct AtomicUintBindingData {
 *   // Always the value of 'binding' from layout attribute
 *   u32 gl_binding;
 *   // Total bytes in binding. Equals to (max_offset + sizeof(uint)).
 *   // This is amount of memory required for underlaying binded buffer!
 *   u32 bytes;
 *   // member count, not actually that informative, since the memory required
 *   // depends on max offset (@see bytes). In other words, it is legal to
 *   // have have 3 members, that are at offsets [0, 8, 20]. This would mean
 *   // we need 24 bytes allocated in buffer, despite only using 3 uints.
 *   u32 members;
 * };
 *
 */

#include "introspect.hpp"

namespace glUtils {
  typedef std::vector<ShaderVariable> ShaderVariables;
  typedef std::vector<ShaderBlock> ShaderBlocks;
  typedef std::vector<ShaderAtomicUint> ShaderAtomics;


  struct ShaderErrorsScratchpad {
    std::string msg = "";
  };

  struct ShaderTexts {
    typedef const char* ShaderText;

    ShaderText vertex = nullptr;
    ShaderText fragment = nullptr;
    ShaderText geometry = nullptr;
    ShaderText tessellation_control = nullptr;
    ShaderText tessellation_evaluation = nullptr;
    ShaderText compute = nullptr;
  };

  struct Shader {
    GLuint gl_id = GL_UTILS_NOT_CREATED_ID;

    inline bool is_created () const noexcept { return gl_id != GL_UTILS_NOT_CREATED_ID; }

    const ShaderVariable* get_attribute(ShaderResourceName) const noexcept;
    const ShaderVariable* get_uniform(ShaderResourceName) const noexcept;
    /** Can't return const, as sometimes You have to change binding by hand */
    ShaderBlock* get_block(ShaderResourceName, bool is_ubo) noexcept;
    const ShaderAtomicUint* get_atomic(u32 gl_binding) const noexcept;

    /** All used variables: attributes, uniforms, block members */
    ShaderVariables used_variables;
    /** All used blocks */
    ShaderBlocks used_blocks;
    /** All used atomics */
    ShaderAtomics used_atomics;
  };

}
