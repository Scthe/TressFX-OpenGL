#pragma once

static i32 min_3(i32 a, i32 b, i32 c) {
	return std::min(a, std::min(b, c));
}

static u32 calculate_copy_range(
		const glUtils::RawBuffer& src, const glUtils::RawBuffer& dest,
		const glUtils::BufferRange src_range_, const u32 dest_offset)
{
	i32 size_left_src = src.bytes - src_range_.offset(),
			size_left_dest = dest.bytes - dest_offset,
			bytes_to_copy = min_3(size_left_src, size_left_dest, (i32)src_range_.size());

	GFX_FAIL_IF(!src.is_created(), "Tried to copy ", src_range_.size(), " bytes from deleted buffer");
	GFX_FAIL_IF(!dest.is_created(), "Tried to copy ", src_range_.size(), " bytes into deleted buffer");
	GFX_FAIL_IF(size_left_src < 1, "Tried to copy ", src_range_.size(), " bytes from source at offset ",
		src_range_.offset(), ", but buffer has only ", src.bytes, " bytes");
	GFX_FAIL_IF(size_left_dest < 1, "Tried to copy ", src_range_.size(),
		" bytes into destination buffer at offset ", dest_offset, ", but buffer has only ", dest.bytes, " bytes");
	GFX_FAIL_IF(bytes_to_copy < 1, "Tried to copy ", bytes_to_copy,
		" bytes between buffers. You probably wanted to copy >0 bytes");

	return (u32) bytes_to_copy;
}

namespace glUtils {

	u32 copy_to(const RawBuffer& src, const RawBuffer& dest, const BufferRange src_range_, const u32 dest_offset) {
		const BufferRange copy_range = {
			src_range_.offset(),
			calculate_copy_range(src, dest, src_range_, dest_offset)
		};

		if (copy_range.size() == 0) {
			return 0;
		}

		GFX_GL_CALL(glCopyNamedBufferSubData,
			src.gl_id, dest.gl_id,
			copy_range.offset(), dest_offset, copy_range.size());

		return copy_range.size();
	}

}
