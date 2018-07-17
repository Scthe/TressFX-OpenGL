#pragma once

static bool init_sdl(glUtils::GlWindow& window, const glUtils::WindowInitOpts&);

namespace glUtils {

  GlWindow create_window(const WindowInitOpts& opt) {
    GlWindow window;
    if (init_sdl(window, opt)) {
      i32 size[2];
      SDL_GL_GetDrawableSize(window.sdl_window, &size[0], &size[1]);
      window.screen_size[0] = size[0];
      window.screen_size[1] = size[1];

      SDL_RaiseWindow(window.sdl_window);
    }
    return window;
  }

  void destroy(GlWindow& window) {
    SDL_FreeSurface(window.sdl_surface);
    SDL_GL_DeleteContext(window.gl_context);
    SDL_DestroyWindow(window.sdl_window);

    SDL_Quit();
  }

} // namespace glUtils


//
// Impl:

#define CHECK_ERROR(code, msg) \
    if (!check_error(code, msg, __FILE__, __LINE__)){ return false; }

static bool check_error(int code, const char* const msg, const char* const file_name, const int line) {
  if (code != 0) {
		LOGE << "SDL init error '" << msg << "'."
       << " Code: " << code << ", SDL Message '" << SDL_GetError() << "'"
			 << " In " << file_name << ":" << line;
    return false;
  }

  return true;
}


static bool set_sdl_atributes(const glUtils::WindowInitOpts& opt) {
  /** https://wiki.libsdl.org/SDL_GL_SetAttribute */

  if (opt.depth_buffer > 16 && opt.stencil_buffer > 0) {
    LOGW << "SDL may skip stencil buffer (requested " << opt.stencil_buffer << "bits), "
            "when depth buffer uses whole 24 bits. If You encounter some problems with "
            "stencil not working, switch to 16/8 combination. "
            "SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE) is also buggy";
  }

  // depth bits
  auto  err = SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, opt.depth_buffer);
  CHECK_ERROR(err, "Could not set required number of depth buffer bits")

  // stencil bits
  err = SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, opt.stencil_buffer);
  CHECK_ERROR(err, "Could not set required number of stencil buffer bits")

  // multisampling
  err = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  err = err || SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, opt.msaa_samples);
  CHECK_ERROR(err, "Could not set multisampling window options")

  // context flags
  // SDL_GL_CONTEXT_DEBUG_FLAG - more insight in errors
  // SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG - remove deprecated opengl functions
  auto sdl_context_flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
  #if defined (GFX_DEBUG)
  sdl_context_flags |= SDL_GL_CONTEXT_DEBUG_FLAG;
  #endif
  err = SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, sdl_context_flags);
  CHECK_ERROR(err, "Could not set opengl window context flags")

  // required opengl version
  err = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, opt.opengl_version.major);
  CHECK_ERROR(err, "Could not set opengl major version")
  err = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, opt.opengl_version.minor);
  CHECK_ERROR(err, "Could not set opengl minor version")

  // opengl profile
  // SDL_GL_CONTEXT_PROFILE_CORE - remove deprecated opengl functions
  err = SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  CHECK_ERROR(err, "Could not set opengl core profile")

  // srgb
  if (opt.use_srgb) {
      err = SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
      CHECK_ERROR(err, "Could not srgb mode")
  }

  return true;
}

static SDL_Window* create_window (const glUtils::WindowInitOpts& opt) {
  SDL_Window* window = nullptr;
  auto window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_MOUSE_FOCUS | opt.sdl_window_flags;

  if (opt.fullscreen){
      window_flags |= SDL_WINDOW_FULLSCREEN;
      window = SDL_CreateWindow(opt.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, window_flags);
  } else {
      window = SDL_CreateWindow(opt.title.c_str(), opt.x, opt.y, opt.w, opt.h, window_flags);
  }

  return window;
}

bool init_sdl(glUtils::GlWindow& window, const glUtils::WindowInitOpts& opt) {
	auto err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | opt.sdl_init_flags);
	CHECK_ERROR(err, "Could not init SDL")

  if (!set_sdl_atributes(opt)){
			return false;
	}

  err = SDL_GL_LoadLibrary(NULL);
  CHECK_ERROR(err, "Could not load default opengl library")

	window.sdl_window = ::create_window(opt);
	CHECK_ERROR(window.sdl_window ? 0 : SDL_UNSUPPORTED, "Could not create Window")

  window.gl_context = SDL_GL_CreateContext(window.sdl_window);
  CHECK_ERROR(window.gl_context ? 0 : SDL_UNSUPPORTED,
    "Could not create SDL opengl context used as a source for opengl functions")

  // vsync - https://wiki.libsdl.org/SDL_GL_SetSwapInterval
  err = SDL_GL_SetSwapInterval(opt.vsync ? 1 : 0);
  CHECK_ERROR(err, "Could not set vsync")

	// SDL has only subset of opengl fn pointers, this will load all of them
	gladLoadGLLoader(SDL_GL_GetProcAddress);
	// CHECK_ERROR(err, "Could not load default opengl library");

	window.sdl_surface = SDL_GetWindowSurface(window.sdl_window);
  return true;
}
