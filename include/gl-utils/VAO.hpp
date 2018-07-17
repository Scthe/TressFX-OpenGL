#pragma once

namespace glUtils {

  struct VertexAttributeType {
    // GL_INT, GL_FLOAT, etc.
    // only simple types, do not use FLOAT_VEC2 etc.
    GLenum base_type = GL_FLOAT;
    // one of: {1,2,3,4}
    u32 components = 1;
  };

  struct VertexAttribute {

    VertexAttribute(const RawBuffer*, VertexAttributeType, u32 offset, u32 stride);

    VertexAttributeType type;

    // Assigned location, this will be automatically assigned in create_vao
    // as a simple increment. Do not use this for matrices (I was too lazy
    // to handle manual location assignment)!
    //
    // NOTE: if attribute is a matrix, this value refers to 1st column.
    //       Additional 3 attributes (for mat4) will  be reserved for other columns
    // NOTE: You may want to use 'in layout(location=X) T v' inside shader
    //
    // @see glBindAttribLocation(program.gl_id, location_to_set, "position")
    // @see glGetAttribLocation(program.gl_id, "position");
    // @see https://www.youtube.com/watch?v=mL6BvXVtd9Y
    // @see https://stackoverflow.com/questions/4635913/explicit-vs-automatic-attribute-location-binding-for-opengl-shaders
    u32 gl_location = 0;

    const RawBuffer* buffer = nullptr;

    // From the beginning of the buffer, how much bytes till 1st
    // occurence of this attribute data. Mainly 2 use cases:
    // - when [{pos1,uv1}, {pos2,uv2}, ..]
    //   then offset for pos is 0 and for uv is sizeof(pos)
    // - when [pos1,pos2,...,posN,  uv1,uv2,...,uvN]
    //   then offset for pos is 0 and for uv is N*sizeof(pos)
    u32 offset = 0;

    // Bytes between next occurance of this attribute e.g.
    // - when [{pos1,uv1}, {pos2,uv2}, ..]
    //   then stride for both pos and uv is (sizeof(pos) + sizeof(uv)),
    //   as this is how much bytes is between each next vertex data
    // - when [pos1,pos2,...,posN,  uv1,uv2,...,uvN]
    //   then offset for pos is sizeof(pos) and for uv is sizeof(uv)
    u32 stride = 0;
  };

  struct VAO {
    // typedef gfx::Vector<gfx::VertexAttribute> VertextAttrList;

    // VAO(GfxAllocator& alloc) : attributes(alloc){}

  	// inline bool is_created () const noexcept { return gl_id != VAO_UNITIALIZED; }

    GLuint gl_id = GL_UTILS_NOT_CREATED_ID;
    // VertextAttrList attributes;

    inline bool is_created () const noexcept { return gl_id != GL_UTILS_NOT_CREATED_ID; }
  };

}
