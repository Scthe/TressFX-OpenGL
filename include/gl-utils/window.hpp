#pragma once
namespace glUtils {

  struct WindowInitOpts {
    u32 x = 50,
        y = 50;
    u32 w = 640,
        h = 320;
    bool fullscreen = false;
    std::string title = "GlWindow";

    bool vsync            = false;
    i32 depth_buffer      = 16;
    i32 stencil_buffer    = 8;
    i32 msaa_samples      = 1;
    bool use_srgb         = false;
    u32 sdl_init_flags   = 0;
    u32 sdl_window_flags = 0;
    // TODO double buffer using SDL_GL_DOUBLEBUFFER?

    struct {
      u8 major = 4;
      u8 minor = 5; // too lazy to update driver for 4.6
    } opengl_version;
  };

  struct GlWindow {
    glm::uvec2 screen_size;
    SDL_Window* sdl_window = nullptr;
		SDL_Surface* sdl_surface = nullptr;
		void* gl_context;
  };

}
