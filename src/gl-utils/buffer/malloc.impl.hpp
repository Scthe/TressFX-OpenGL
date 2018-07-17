#pragma once

static void allocate_buffer (glUtils::RawBuffer& buf);

namespace glUtils {

	RawBuffer malloc(u32 bytes, BufferUsagePattern usage_pattern) {
		RawBuffer buffer;

		GFX_FAIL_IF(bytes == 0, "Could not allocate buffer of size 0. Use clear() instead.");

		GFX_GL_CALL(glCreateBuffers, 1, &buffer.gl_id);
		buffer.bytes = bytes;
		buffer.usage_pattern = usage_pattern;

		allocate_buffer(buffer);

		return buffer;
	}

}

void allocate_buffer (glUtils::RawBuffer& buf) {
	using namespace glUtils;

	GLuint //mutable_storage_flags,   // flags for glBufferData
				 immutable_storage_flags; // flags for glBufferStorage
	 bool immutable = false;

	switch (buf.usage_pattern) {
		case BufferUsagePattern::Persistent:
			// mutable_storage_flags = GL_DYNAMIC_DRAW;
			immutable_storage_flags = GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
			immutable = true;
			break;
		case BufferUsagePattern::Dynamic:
			// mutable_storage_flags = GL_DYNAMIC_DRAW;
			immutable_storage_flags = GL_DYNAMIC_STORAGE_BIT | GL_CLIENT_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
			break;
		case BufferUsagePattern::Default:
			// mutable_storage_flags = GL_STATIC_DRAW;
			immutable_storage_flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
			break;
		case BufferUsagePattern::Immutable:
			// mutable_storage_flags = GL_STATIC_DRAW;
			immutable_storage_flags = 0;
			immutable = true;
			break;
	}

	GLint true_allocated_size = -1;

	GFX_GL_CALL(glNamedBufferStorage, buf.gl_id, buf.bytes, nullptr, immutable_storage_flags);
	GFX_GL_CALL(glGetNamedBufferParameteriv, buf.gl_id, GL_BUFFER_SIZE, &true_allocated_size);

	if (buf.bytes != (u32)true_allocated_size) {
		destroy(buf);
		GFX_FAIL("Could not allocate buffer - out of memory. Tried allocating ",
			buf.bytes, " bytes, only ", true_allocated_size, " were allocated");
		return;
	}

	/*
	TODO create mapping during buffer init if persistent
	if (immutable && usage_pattern == BufferUsagePattern::Persistent) {
		GFX_GL_CALL(glMapNamedBufferRange, id, 0, size as GL_types::GLsizeiptr,
																GL_MAP_READ_BIT | GL_MAP_WRITE_BIT |
																GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

		if ptr.is_null() {
				let error = ::get_gl_error(ctxt);
				panic!("GFX_GL_CALL(glMapBufferRange returned null , error: {:?})", error);
		}
	}
	*/
}
