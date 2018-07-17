#pragma once

static void setTexParam(const glUtils::Texture&, GLenum pname, i32);
static void setTexParam(const glUtils::Texture&, GLenum pname, u32*);

namespace glUtils {

  void apply_texture_options(const Texture& texture, const TextureOpts& opts) {
    // if (version >= ::utils::Version(4, 3)) {
      // not working?
      // setTexParam(texture, GL_DEPTH_STENCIL_TEXTURE_MODE, opts.read_depth_if_depth_stencil ? GL_DEPTH_COMPONENT : GL_STENCIL_COMPONENT);
    // }

    setTexParam(texture, GL_TEXTURE_BASE_LEVEL, (i32)opts.mipmap_min);
    setTexParam(texture, GL_TEXTURE_MAX_LEVEL,  (i32)opts.mipmap_max);
    setTexParam(texture, GL_TEXTURE_BORDER_COLOR, (u32*)opts.border_color);
    setTexParam(texture, GL_TEXTURE_MIN_FILTER, opts.filter_min);
    setTexParam(texture, GL_TEXTURE_MAG_FILTER, opts.filter_mag);
    setTexParam(texture, GL_TEXTURE_MIN_LOD, opts.lod_min);
    setTexParam(texture, GL_TEXTURE_MAX_LOD, opts.lod_max);

    setTexParam(texture, GL_TEXTURE_SWIZZLE_R, opts.swizzle[0]);
    setTexParam(texture, GL_TEXTURE_SWIZZLE_G, opts.swizzle[1]);
    setTexParam(texture, GL_TEXTURE_SWIZZLE_B, opts.swizzle[2]);
    setTexParam(texture, GL_TEXTURE_SWIZZLE_A, opts.swizzle[3]);
    setTexParam(texture, GL_TEXTURE_WRAP_S, opts.wrap[0]);
    setTexParam(texture, GL_TEXTURE_WRAP_T, opts.wrap[1]);
    setTexParam(texture, GL_TEXTURE_WRAP_R, opts.wrap[2]);

    // GL_TEXTURE_LOD_BIAS, must be < GL_MAX_TEXTURE_LOD_BIAS
    setTexParam(texture, GL_TEXTURE_LOD_BIAS, opts.lod_bias);

    // Just when You thought the OpenGL docs could not get any worse..
    /*if (opts.depth_compare_fn == TextureCompareFn::None) {
      setTexParam(texture, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    } else {
      setTexParam(texture, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
      setTexParam(texture, GL_TEXTURE_COMPARE_FUNC, opts.depth_compare_fn);
    }*/
  }

} // namespace glUtils

void setTexParam(const glUtils::Texture& texture, GLenum pname, i32 value) {
  GFX_GL_CALL(glTextureParameteri, texture.gl_id, pname, value);
}

void setTexParam(const glUtils::Texture& texture, GLenum pname, u32* data) {
  GFX_GL_CALL(glTextureParameterIuiv, texture.gl_id, pname, data);
}
