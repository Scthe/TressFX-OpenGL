#pragma once

#include "./malloc.impl.hpp"
#include "./read.impl.hpp"
#include "./write.impl.hpp"
#include "./copy_to.impl.hpp"

namespace glUtils {

	void invalidate (const RawBuffer& buf, const BufferRange range_bytes) {
		auto is_whole_buffer = range_bytes.offset() == 0 && range_bytes.size() == buf.bytes;

		if (is_whole_buffer){
			GFX_GL_CALL(glInvalidateBufferData, buf.gl_id);
		} else {
			GFX_GL_CALL(glInvalidateBufferSubData, buf.gl_id, range_bytes.offset(), range_bytes.size());
		}
	}

	void destroy(RawBuffer& buf) {
		if (!buf.is_created()) {
			return;
		}

		invalidate(buf, {0, buf.bytes});

		GFX_GL_CALL(glDeleteBuffers, 1, &buf.gl_id);

		buf.gl_id = GL_UTILS_NOT_CREATED_ID;
	}

}
