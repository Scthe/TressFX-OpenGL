#pragma once

static u32 calculate_read_range(const glUtils::RawBuffer& buf, const glUtils::BufferRange range_bytes_, void*const out_ptr) {
  GFX_FAIL_IF(!buf.is_created(), "Tried to read ", range_bytes_.size(), " bytes from deleted buffer");
  GFX_FAIL_IF(out_ptr == nullptr, "Tried to read data into nullptr. You probably wanted to read into allocated buffer");

	i32 size_left = buf.bytes - range_bytes_.offset(),
			bytes_to_read = std::min(size_left, (i32) range_bytes_.size());

  GFX_FAIL_IF(size_left < 1, "Tried to read at offset ", range_bytes_.offset(), ", but buffer has only ", buf.bytes, " bytes");
  GFX_FAIL_IF(bytes_to_read < 1, "Tried to read ", bytes_to_read, " bytes. You probably wanted to read >0 bytes");

	return bytes_to_read;
}

namespace glUtils {

	u32 read (const RawBuffer& buf, const BufferRange range_bytes_, void*const out_ptr) {
		BufferRange range_bytes = {
			range_bytes_.offset(),
			calculate_read_range(buf, range_bytes_, out_ptr)
		};
		if (range_bytes.size() == 0) {
			return 0;
		}

		if (buf.is_persistent_mapping()) {
			GFX_FAIL("Reading persistently mapped buffers is not supported ATM");
		} else {
			GFX_GL_CALL(glGetNamedBufferSubData, buf.gl_id, range_bytes.offset(), range_bytes.size(), out_ptr);
		}

		return range_bytes.size();
	}

}
