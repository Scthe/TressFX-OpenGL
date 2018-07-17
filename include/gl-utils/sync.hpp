#pragma once

namespace glUtils {

  /**
    There are certain GPU operations that do not affect memory (buffers/textures) in synchronous manner.
    This means that when we write to memory in such manner, the next operation may not receive updated result.
    This is known as "incoherent memory accesses". It includes following operations:
      - writes (atomic or otherwise) via Image Load Store (RandomAccessTexture)
      - writes (atomic or otherwise) via Shader Storage Buffer Objects (SSBO)
      - writes to variables declared as shared in Compute Shaders (but not output variables in Tessellation Control Shaders)
    This means that before we read the value, we have to synchronize using glMemoryBarier with appropriate bits set.
    Each mode says how we READ the opengl object. E.g. we write to raw memory, but then we interpret this memory
    during READ as BufferDrawCommand - that would require call to glMemoryBarier with
    GL_COMMAND_BARRIER_BIT (or gfx::async_write_barrier(AsyncWriteableResource::BufferDrawCommand))

    NOTE: memoryBarier is GLSL function. glMemoryBarier is opengl function. Be careful when reading the docs

    @see https://www.khronos.org/opengl/wiki/Memory_Model
    @see https://www.khronos.org/opengl/wiki/GLAPI/glMemoryBarrier
  */
  struct AsyncWriteableResource {
    // we write to buffer as e.g. SSBO, the use this buffer as:
    static const GLenum BufferVertex = GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
    static const GLenum BufferIndices = GL_ELEMENT_ARRAY_BARRIER_BIT;
    static const GLenum BufferUbo = GL_UNIFORM_BARRIER_BIT;
    static const GLenum BufferDrawCommand = GL_COMMAND_BARRIER_BIT; // GL_DRAW_INDIRECT_BUFFER and GL_DISPATCH_INDIRECT_BUFFER
    static const GLenum BufferPixels = GL_PIXEL_BUFFER_BARRIER_BIT; // GL_PIXEL_PACK_BUFFER and GL_PIXEL_UNPACK_BUFFER
    static const GLenum BufferQuery = GL_QUERY_BUFFER_BARRIER_BIT; // (v>=44)
    static const GLenum BufferAtomic = GL_ATOMIC_COUNTER_BARRIER_BIT;
    static const GLenum BufferSsbo = GL_SHADER_STORAGE_BARRIER_BIT; // (v>=43)
    static const GLenum BufferTransformFeedback = GL_TRANSFORM_FEEDBACK_BARRIER_BIT;
    static const GLenum BufferAllOps = GL_BUFFER_UPDATE_BARRIER_BIT;

    // textures - write using Image load/store technique (RandomAccessTexture)
    static const GLenum RandomAccessTexture = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; // Image load/store
    static const GLenum Framebuffer = GL_FRAMEBUFFER_BARRIER_BIT; // ?we can async write to texture that is fbo and then we have to wait for it to be visible?
    static const GLenum Texture = GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT; // Texture e.g. buffer textures
             // Texture eg.glTex(Sub)Image*, glCopyTex(Sub)Image*, glClearTex*Image, glCompressedTex(Sub)Image*, glGetTexImage

    // Other
    static const GLenum MappedBuffer = GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT; // when buffer is mapped, we need to wait for wrtite-through to read async changes
    static const GLenum All = GL_ALL_BARRIER_BITS;
  };
  typedef GLenum AsyncWriteableResource_;

}
