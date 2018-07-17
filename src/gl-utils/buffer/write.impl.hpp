#pragma once

static u32 calculate_write_range(const glUtils::RawBuffer& buf, const glUtils::BufferRange range_bytes_, const void*const data) {
  GFX_FAIL_IF(!buf.is_created(), "Tried to write ", range_bytes_.size(), " bytes into deleted buffer");
  GFX_FAIL_IF(data == nullptr, "Tried to write data from nullptr. You probably wanted to write some existing data");

	i32 size_left = buf.bytes - range_bytes_.offset(),
			bytes_to_write = std::min(size_left, (i32) range_bytes_.size());

  GFX_FAIL_IF(size_left < 1, "Tried to write at offset ", range_bytes_.offset(), ", but buffer has only ", buf.bytes, " bytes");
  GFX_FAIL_IF(bytes_to_write < 1, "Tried to write ", bytes_to_write, " bytes. You probably wanted to write >0 bytes");

	return bytes_to_write;
}

namespace glUtils {

	u32 write (const RawBuffer& buf, const BufferRange range_bytes_, const void*const data) {
		BufferRange range_bytes = {
			range_bytes_.offset(),
			calculate_write_range(buf, range_bytes_, data)
		};
		if (range_bytes.size() == 0) {
			return 0;
		}

		if (buf.is_persistent_mapping()) {
			// NOTE: immpossible to realocate buffer this way
			// Mapping guard = this->map(range_el, false, true);
			// memcpy(guard, data, range_bytes.size());
			GFX_FAIL("Writing to persistently mapped buffers is not supported ATM");
		} else if (buf.is_immutable()) {
			// we are immutable -> create new buffer and copy into ourself
			// GpuBuffer buf;
			// buf.init(ctxt.last_error, *ctxt, BufferBindType::CopyReadBuffer, format, BufferUsagePattern::Dynamic)
				//  .allocate(ctxt.last_error, range_el.size)
				//  .write(ctxt.last_error, range_el, data) // we can write to newly created buffer
				//  .copy_to(ctxt.last_error, *this, {range_el.size}, range_el.offset)
				//  .release(ctxt.last_error);
			// if (!es2.is_ok()) { es.reset(); es << es2.msg(); return *this; }
			GFX_FAIL("Writing to immutable buffer is not supported ATM");
		} else {
			// normal case, inlcuding when allocating new buffer of any type
			// if (ctxt.version() > Version(4,3)) {
				// glInvalidateBufferData(buf.gl_id); // give hint is not needed
			// }

			GFX_GL_CALL(glNamedBufferSubData, buf.gl_id, range_bytes.offset(), range_bytes.size(), data);
		}

		return range_bytes.size();
	}

}
