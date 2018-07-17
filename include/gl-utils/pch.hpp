#pragma once

// No really pch, but..

#define GL_UTILS_NOT_CREATED_ID 0xffffffff

// <editor-fold macro: assertions>
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define GFX_FAIL(...) \
  { \
    LOGE << "Assert failed (" << __FILE__ << ":" << __LINE__ << ") " APPLY(<<, __VA_ARGS__); \
		/*DEBUG_BREAK();*/ \
    assert(false); \
  }

#define GFX_FAIL_IF(condition, ...) \
  if ((condition)) { \
     GFX_FAIL("Failed condition '" , #condition , "' " , __VA_ARGS__) \
	}

#define GFX_FAIL_IF_NULL(PTR) \
		if ((PTR) == nullptr) { \
			GFX_FAIL("Expected pointer '" , #PTR , "' to not be null") \
		}

// </editor-fold>

// <editor-fold GFX_GL_CALL>
// @see https://github.com/JPGygax68/libGPCGLWrappers/blob/master/lib/include/gpc/gl/wrappers.hpp

// #if defined (GFX_DEBUG)
// #  define GFX_GL_CALL(fn, ...) ::glUtils::utils::gl_call(#fn, __LINE__, __FILE__, fn, __VA_ARGS__)
// #else
#  define GFX_GL_CALL(fn, ...) fn(__VA_ARGS__)
// #endif

// </editor-fold>
