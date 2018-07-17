#pragma once

namespace glUtils {

  struct TextureFilterMin {
    static const GLenum Nearest = GL_NEAREST;
    static const GLenum Linear = GL_LINEAR;
    static const GLenum NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST;
    static const GLenum LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST;
    static const GLenum NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR;
    static const GLenum LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR;
  };
  typedef GLenum TextureFilterMin_;

  struct TextureFilterMag {
    static const GLenum Nearest = GL_NEAREST;
    static const GLenum Linear = GL_LINEAR;
  };
  typedef GLenum TextureFilterMag_;

  /*
  struct TextureCompareFn {
    static const GLenum Lequal = GL_LEQUAL;
    static const GLenum Gequal = GL_GEQUAL;
    static const GLenum Less = GL_LESS;
    static const GLenum Greater = GL_GREATER;
    static const GLenum Equal = GL_EQUAL;
    static const GLenum Not_equal = GL_NOTEQUAL;
    static const GLenum Always = GL_ALWAYS;
    static const GLenum Never = GL_NEVER;
    static const GLenum None = GL_NONE; //GL_TEXTURE_COMPARE_MODE == GL_NONE
  };
  typedef GLenum TextureCompareFn_;
  */

  struct TextureSwizzle {
    static const GLenum Red = GL_RED;
    static const GLenum Green = GL_GREEN;
    static const GLenum Blue = GL_BLUE;
    static const GLenum Alpha = GL_ALPHA;
    static const GLenum Zero = GL_ZERO;
    static const GLenum One = GL_ONE;
  };
  typedef GLenum TextureSwizzle_;

  struct TextureWrap {
    static const GLenum UseEdgePixel = GL_CLAMP_TO_EDGE;
    static const GLenum UseBorderColor = GL_CLAMP_TO_BORDER;
    static const GLenum MirroredRepeat = GL_MIRRORED_REPEAT;
    static const GLenum Repeat = GL_REPEAT;
    static const GLenum MirrorThenUseEdgePixel = GL_MIRROR_CLAMP_TO_EDGE;
  };
  typedef GLenum TextureWrap_;

  /**
   * @see http://docs.gl/gl4/glTexParameter#Description
   */
  struct TextureOpts {
    /** GL_DEPTH_STENCIL_TEXTURE_MODE (it true will read depth, if false will read stencil) TODO should be set dynamically per read basis?*/
    // bool read_depth_if_depth_stencil = true;
    /** GL_TEXTURE_BASE_LEVEL */
    u32 mipmap_min = 0;
    /** GL_TEXTURE_MAX_LEVEL */
    u32 mipmap_max = 0;
    /** GL_TEXTURE_BORDER_COLOR, order: RGBA */
    u32 border_color[4] = {0,0,0,0}; //
    /** GL_TEXTURE_COMPARE_FUNC */
    // TextureCompareFn_ depth_compare_fn = TextureCompareFn::None;
    /** GL_TEXTURE_MIN_FILTER */
    TextureFilterMin_ filter_min = TextureFilterMin::Linear;
    /** GL_TEXTURE_MAG_FILTER */
    TextureFilterMag_ filter_mag = TextureFilterMag::Linear;
    /** GL_TEXTURE_MIN_LOD, */
    i32 lod_min = -1000;
    /** GL_TEXTURE_MAX_LOD, */
    i32 lod_max = 1000;
    /** GL_TEXTURE_LOD_BIAS, must be < GL_MAX_TEXTURE_LOD_BIAS */
    f32 lod_bias = 0.0;
    /** GL_TEXTURE_SWIZZLE_RGBA, GL_TEXTURE_SWIZZLE_R, GL_TEXTURE_SWIZZLE_G, GL_TEXTURE_SWIZZLE_B, GL_TEXTURE_SWIZZLE_A, */
    TextureSwizzle_ swizzle[4] = {
      TextureSwizzle::Red,
      TextureSwizzle::Green,
      TextureSwizzle::Blue,
      TextureSwizzle::Alpha
    };
    /** GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R. */
    TextureWrap_ wrap[3] = {
      TextureWrap::Repeat,
      TextureWrap::Repeat,
      TextureWrap::Repeat
    };
  };

}
