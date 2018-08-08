#pragma once

namespace glUtils {

  typedef std::string ShaderResourceName;
  typedef unsigned long ShaderResourceNameHash;

  enum class ShaderVariableType {
    AttributeIn, AttributeOut, Uniform, BlockMember
  };

  /**
   * Represents:
   *   - Vertex attribute
   *   - 'standalone' uniforms
   *   - block members (both ssbo & ubo)
   */
  struct ShaderVariable {
    ShaderVariableType variable_type;

    /** attribute location, used to address it in all opengl functions like setting value for uniform */
    GLuint gl_location;

    /** gl_index of parent block, or -1 if is_attribute is true or is 'standalone' uniform */
    GLint gl_index_of_parent_block = -1;

    /**
     * e.g. GL_FLOAT, GL_INT_VEC2, GL_FLOAT_MAT4, GL_SAMPLER_3D
     * (there are a lot of them)
     *
     * @see table 7.3 on page 114 of OpenGL 4.6 specs
     */
    GLenum gl_type;

    /** allowed for uniforms/block members*/
    u32 array_size = 0;

    /** human readable */
    ShaderResourceName name;
    ShaderResourceNameHash name_hash;
  };

  struct ShaderBlock {
    /** true if ubo, false if ssbo */
    bool is_ubo;

    /**
     * Block index, used to address it in all opengl functions.
     * This is just global unique id for this block, just as name is human readable
     * NOTE: block indices may not be assigned continously / in order they appear in GLSL,
     * but they go 0..GL_ACTIVE_UNIFORM_BLOCKS, as indicated by indices.
     * TL;DR we get the blocks in random order.
     * Source: "uniformBlockIndex is an active uniform block index of program,
     *          and must be less than the value of GL_ACTIVE_UNIFORM_BLOCKS"
     *         http://docs.gl/gl4/glGetActiveUniformBlock
     */
    GLuint gl_index;

    // is 0 by default and managing it by hand is quite complex
    GLuint gl_binding;

    /** size in bytes. Should match sizeof of c++, but may differ depending on packing and vec3 usage*/
    u32 bytes;

    /** human readable */
    ShaderResourceName name;
    ShaderResourceNameHash name_hash;
  };

  struct ShaderAtomicUint {
    // Always the value of 'binding' from layout attribute
    u32 gl_binding;
    // Total bytes in binding/slot. Equals to (max_offset + sizeof(uint)).
    // This is amount of memory required for underlaying (binded) buffer.
    u32 bytes;
    // member count, not actually that informative, since the memory required
    // depends on max offset (@see bytes). In other words, it is legal to
    // have have 3 members, that are at offsets [0, 8, 20]. This would mean
    // we need 24 bytes allocated in buffer, despite only using 3 uints.
    u32 member_count;
  };

}
