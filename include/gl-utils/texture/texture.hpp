#pragma once

#include "TextureOpts.hpp"

namespace glUtils {

  /**
   *
   * Unsized vs Sized and format vs internal format (in normalized environment)
   *
   * Internal format is how memory will be allocated on GPU. It is expressed in 'sized' GLenum e.g. GL_RGBA8 (4 channels of unsigned byte each). On the other hand, when writing/reading we usually provide 3 arguments (GLenum format, GLenum type, GLvoid * pixels), where 'format' is 'unsized' info about channels (e.g. GL_R, GL_RG, GL_RGB, GL_RGBA), 'type' is type in each channel (there are special values like GL_UNSIGNED_SHORT_5_5_5_1, that escape normal conventions, since hard to expect short to be 5bits) and 'pixels' is data. Unsized are used to describe data that we provide so it can be written (see TextureWriteSource) or texture format that we want to read (see TextureReadTarget).
   * NOTE: format := what we provided in data. It is unsized meaning e.g. GL_R, GL_RG, GL_RGB, GL_RGBA instead of GL_RGBA8 etc.
   * NOTE: internal format := how in GPU will be visisble/allocated. Lists all bits variations
   *
   *
   * 'level' parameter: in OpenGL docs always mean mipmap levels
   *  - lvl0 - original,
   *  - lvl1 - 1st mipmap (each dimension halfed)
   *  - lvl2 - 2st mipmap (each dimension is quater of original) etc.
   *
   *
   * Normalized vs unnormalized
   *
   * Normalized means that all results will be converted to float in the range [0,1] ([-1,1] for signed formats). Unnormalized is left as is. See https://stackoverflow.com/questions/34497195/difference-between-format-and-internalformat#comment56736641_34497470 . Normally, normalized textures are used. In case raw ints should be used (unnormalized), please use GL_..._INTEGER (ex. GL_RGBA_INTEGER) variants of channels.
   * NOTE: unnormalized is also known as unclamped (name based on GLSL clamp function)
   * TODO add write_source.use_unnormalized_integers flag for unnormalized
   *
   */
  struct Texture {
    GLuint gl_id = GL_UTILS_NOT_CREATED_ID;

    /** GL_TEXTURE_2D, GL_TEXTURE_3D etc. */
    GLenum gl_type = GL_TEXTURE_2D;

    /** [0] - width,
      * [1] - height,
      * [2] - depth/array size
      */
    u32 dimensions[3] = {0,0,0};

    /** filtering etc. */
    TextureOpts opts;

    /** GL_R8 / GL_RGBA8 / GL_RGBA32F etc. */
    GLenum sized_pixel_format = GL_RGBA8;

    /** Number of mipmap levels (`0` means just the main texture) */
    u32 mipmap_levels = 0;

    /** Not always used */
    u32 multisample = 1;

    inline bool is_created () const noexcept { return gl_id != GL_UTILS_NOT_CREATED_ID; }
  };

}
