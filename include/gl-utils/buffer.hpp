#pragma once

/*
 *  NOTE:
 *   - persistent - allow buffer to be maped when gpu is using it
 *   - coherent - like volatile in java - immediately visible to gpu
 */

namespace glUtils {

  struct BufferBindType {
    /// geometry related
    static const GLenum VertexBuffer = GL_ARRAY_BUFFER;
    static const GLenum IndexBuffer = GL_ELEMENT_ARRAY_BUFFER;

    /// Every function that performs a pixel transfer operation can use buffer objects
    /// instead of client memory. Functions that perform:
    /// * upload operation (a pixel unpack) can use GL_PIXEL_UNPACK_BUFFER f.e. glDrawPixels, glTexImage2D, glTexSubImage2D
    /// * download operation (a pixel pack) can use GL_PIXEL_PACK_BUFFER f.e. glReadPixels, glGetTexImage
    ///
    /// Example usage: when want to copy buffer->texture or texture->buffer. Since we can
    /// easily manipulate buffers (at least easier then textures) this is quite helpful.
    /// For example it was only way to read only part of texture before
    /// opengl 4.5 (glGetTextureSubImage). It also helps with sync/async stalls.
    ///
    /// @see https://www.khronos.org/opengl/wiki/Pixel_Buffer_Object
    /// @see http://www.songho.ca/opengl/gl_pbo.html
    static const GLenum ReadFromTextureBuffer = GL_PIXEL_PACK_BUFFER;
    static const GLenum WriteIntoTextureBuffer = GL_PIXEL_UNPACK_BUFFER;

    /// Helper buffer targets when we need to copy between 2 buffers in old opengl (no dsa).
    /// We would bind source to GL_COPY_READ_BUFFER and target to GL_COPY_WRITE_BUFFER
    /// and initiate copy operation between them
    static const GLenum CopyReadBuffer = GL_COPY_READ_BUFFER;
    static const GLenum CopyWriteBuffer = GL_COPY_WRITE_BUFFER;

    /// Accessible from shaders. Each represents quite different feature, so best to read about them one by one
    static const GLenum UniformBuffer = GL_UNIFORM_BUFFER;
    static const GLenum AtomicCounterBuffer = GL_ATOMIC_COUNTER_BUFFER;
    static const GLenum ShaderStorageBuffer = GL_SHADER_STORAGE_BUFFER; // SSBO
    static const GLenum TransformFeedbackBuffer = GL_TRANSFORM_FEEDBACK_BUFFER;

    /// When drawing we usually provide how many vertices (count in both
    /// glDrawArrays and glDrawElements) to render. This corresponds to number of
    /// triangles (number of vertices divided by 3).
    ///
    /// Where does the data (vertex positions / triangle vertex indices) come from?
    /// Well it is usually from current
    /// * VertexBuffer(GL_ARRAY_BUFFER)
    /// * IndexBuffer(GL_ELEMENT_ARRAY_BUFFER)
    /// assigned either manually or through Vertex Array Object (VAO).
    ///
    /// Other common way is to do f.e. occlusion culling on GPU. This means that
    /// the list of objects to draw is created on GPU. We do not know how much
    /// objects (and triangles) will be drawn (unless we read GPU->CPU memory,
    /// which is slow). Then we can use:
    /// * void glDrawArraysIndirect(GL_TRIANGLES, offset_into_buffer);
    /// * void glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, offset_into_buffer);
    /// to draw instead - all the data will be taken from DrawIndirectBuffer(GL_DRAW_INDIRECT_BUFFER).
    ///
    /// Similarly for dispatch.
    ///
    /// @see https://www.khronos.org/opengl/wiki/Vertex_Rendering#Indirect_rendering
    static const GLenum DrawIndirectBuffer = GL_DRAW_INDIRECT_BUFFER;
    static const GLenum DispatchIndirectBuffer = GL_DISPATCH_INDIRECT_BUFFER;

    /// Buffer backed texture. Some glTexBuffer shinanigans
    static const GLenum TextureBuffer = GL_TEXTURE_BUFFER;

    /// Store query result in buffer, then read the buffer etc.
    /// @see ARB_query_buffer_object docs, especially 'Usage Examples'
    static const GLenum QueryBuffer = GL_QUERY_BUFFER;
  };
  typedef GLenum BufferBindType_;


  enum class BufferUsagePattern {
    // Discouraged, but usable.
    // Flags:
    // - glBufferStorage: GL_DYNAMIC_STORAGE_BIT
    // - glBufferData: GL_STATIC_DRAW (if glBufferStorage not available)
    Default,

    // When buffer's content is modified multiple times per frame
    // Flags:
    // - glBufferStorage: GL_DYNAMIC_STORAGE_BIT | GL_CLIENT_STORAGE_BIT
    // - glBufferData: GL_DYNAMIC_DRAW (if glBufferStorage not available)
    Dynamic,

    // When buffer content is modified once per frame. Fences are automatically inserted. Recommended to be used with triple buffering
    // Flags:
    // - glBufferStorage: GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT
    // - glBufferData: GL_DYNAMIC_DRAW (if glBufferStorage not available)
    Persistent,

    // Modifications are permitted, but strongly discouraged
    // Flags:
    // - glBufferStorage:
    // - glBufferData: GL_STATIC_DRAW (if glBufferStorage not available)
    Immutable
  };

  struct BufferRange {
    explicit BufferRange(const u32 off=0, const u32 s=0)
  		: _size(s),
  			_offset(off) {}

    BufferRange(std::initializer_list<u32> l) {
      auto it = l.begin();
      if (l.size() == 1) {_size = *it;}
      if (l.size() >= 2) {_offset = *it; ++it; _size = *it;}
    }

    inline u32 end   () const noexcept { return offset() + size(); }
    inline u32 offset() const noexcept { return _offset; }
    inline u32 size  () const noexcept { return _size; }

    private:
    u32 _size = 0;
    u32 _offset = 0;
  };

  /// Usefull general note about opengl buffers:
  ///
  /// There are no different 'types' of buffers. Buffers are just long sequences of 010101010010101001.. (some memory with no meaning).
  /// We can use memory in buffers for different purposes. In that case we bind it (whole buffer or just some range) in opengl (glBindBufferRange).
  /// If the buffer range contains vertex data we bind it as GL_ARRAY_BUFFER, and then bind indices (some other buffer range) as GL_ELEMENT_ARRAY_BUFFER. Then we call function that makes use of both GL_ARRAY_BUFFER and GL_ELEMENT_ARRAY_BUFFER f.e. glDrawElements.
  ///
  /// It should be possible to use only a SINGLE huge buffer in your whole app. Part of it will be vertex data, some other part indices etc.
  ///
  struct RawBuffer {
    GLuint gl_id = GL_UTILS_NOT_CREATED_ID;
    u32 bytes;
    BufferUsagePattern usage_pattern;

    inline BufferRange range () const noexcept { return BufferRange(0, bytes); };

    inline bool is_created () const noexcept { return gl_id != GL_UTILS_NOT_CREATED_ID; }
    inline bool is_persistent_mapping () const noexcept { return usage_pattern == BufferUsagePattern::Persistent; }
    inline bool is_immutable () const noexcept { return usage_pattern == BufferUsagePattern::Immutable; }
  };

} // namespace glUtils

inline bool operator==(const glUtils::BufferRange& a, const glUtils::BufferRange& b) {
  return (a.size() == b.size()) && (a.offset() == b.offset());
}

inline bool operator!=(const glUtils::BufferRange& a, const glUtils::BufferRange& b) {
  return !(a == b);
}
