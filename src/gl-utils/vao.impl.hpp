#pragma once

static void bind_buffer_attribute_to_location(u32 location, const glUtils::VertexAttribute&);

namespace glUtils {

  VertexAttribute::VertexAttribute(const RawBuffer* buffer,
			VertexAttributeType type, u32 offset, u32 stride)
		: type(type),
		  buffer(buffer),
			offset(offset),
			stride(stride)
	{
		GFX_FAIL_IF(!buffer, "Tried to initialize vertex attribute using non existing buffer(nullptr)");
	}

	VAO create_vao(std::initializer_list<VertexAttribute> attrs) {
		VAO vao;
		GFX_GL_CALL(glGenVertexArrays, 1, &vao.gl_id);
		GFX_GL_CALL(glBindVertexArray, vao.gl_id);

		u32 location = 0;
		for (auto &attr : attrs) {
      GFX_FAIL_IF(!attr.buffer, "Provided VAO with attribute that does not point to buffer. "
        "This is legal in OpenGL (attribute has some default value provided by user), "
        "but not implemented in create_vao");
			bind_buffer_attribute_to_location(location, attr);
			GFX_GL_CALL(glEnableVertexAttribArray, location);
      // TODO store in VAO data for attribs, though why?
			// attr.gl_location = location; // if You ever want to read back this value
			++location;
		}

		GFX_GL_CALL(glBindVertexArray, 0);
		return vao;
	}

  void destroy(VAO& vao) {
    if (!vao.is_created()) {
      return;
    }

    // gfx::utils::activate_for_edit(ctxt, &vao);

		// for(u32 i = 0; i < vao.attributes.size(); i++) {
			// if (vao.attributes[i].has_buffer()) {
				// GFX_GL_CALL(glDisableVertexAttribArray, i);
			// }
		// }

		GFX_GL_CALL(glDeleteVertexArrays, 1, &vao.gl_id);

		// gfx::utils::activate_for_edit(ctxt, (VAO*) nullptr);

    vao.gl_id = GL_UTILS_NOT_CREATED_ID;
  }

}

static bool is_integer (GLenum type) {
	switch(type) {
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_INT:
		case GL_UNSIGNED_INT:
			return true;
		default:
			return false;
	}
}

void bind_buffer_attribute_to_location(u32 location, const glUtils::VertexAttribute& attr) {
	glBindBuffer(glUtils::BufferBindType::VertexBuffer, attr.buffer->gl_id);

	auto offset = (void*)(size_t)attr.offset;
	auto gl_type = attr.type.base_type;
	auto components = attr.type.components;

  if (is_integer(gl_type)) {
    GFX_GL_CALL(glVertexAttribIPointer,
      location, components, gl_type,
      attr.stride, offset);
  } else if (gl_type == GL_DOUBLE) {
    GFX_GL_CALL(glVertexAttribLPointer,
      location, components, gl_type,
      attr.stride, offset);
  } else {
    GFX_GL_CALL(glVertexAttribPointer,
      location, components, gl_type,
      GL_FALSE,
			attr.stride, offset);
  }
}
