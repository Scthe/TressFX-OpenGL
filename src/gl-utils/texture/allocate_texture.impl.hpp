#pragma once

// ARB_texture_storage (core 4.2) provides immutable textures whose definition is complete up-front.
//          The classic glTexImage never "knew" how many mip-maps would actually be specified,
//          often causing lazy allocation. glTextureStorage is the better alternative for the driver.
//          glTextureStorage produces immutable memory boundaries

static void allocate_storage (const glUtils::Texture& texture);

namespace glUtils {

  void allocate_texture(Texture& texture, bool apply_texture_options) {
    GFX_GL_CALL(glCreateTextures, texture.gl_type, 1, &texture.gl_id);

    allocate_storage(texture);

    if (apply_texture_options) {
      glUtils::apply_texture_options(texture, texture.opts);
    }
  }

  void destroy(Texture& texture) {
    if (!texture.is_created()) {
      return;
    }

    GFX_GL_CALL(glDeleteTextures, 1, &texture.gl_id);

    texture.gl_id = GL_UTILS_NOT_CREATED_ID;
  }

} // namespace glUtils

static void verify_dimension (const u32*const& dims, const u32 to_check) {
  GFX_FAIL_IF(to_check > 3, "Tried to verify dimension of 4D texture? WHAT?!");
  const char* order_sufix[3] = {"st", "nd", "rd"};

  for (size_t i = 0; i < to_check; i++) {
    GFX_FAIL_IF(dims[i] == 0, "Tried to allocate ", to_check,
      " dimensional texture, but ", (i+1), order_sufix[i], " dimension (or multisample) is 0");
  }
}

static bool is_3d(const glUtils::Texture& texture) {
  return texture.gl_type == GL_TEXTURE_3D
    || texture.gl_type == GL_TEXTURE_2D_ARRAY
    || texture.gl_type == GL_TEXTURE_CUBE_MAP_ARRAY;
}

static bool is_2d(const glUtils::Texture& texture) {
  return texture.gl_type == GL_TEXTURE_2D
    || texture.gl_type == GL_TEXTURE_1D_ARRAY
    || texture.gl_type == GL_TEXTURE_CUBE_MAP;
}

static bool is_1d(const glUtils::Texture& texture) {
  return texture.gl_type == GL_TEXTURE_1D;
}

void allocate_storage (const glUtils::Texture& texture) {
  const u32* dimensions = texture.dimensions;
  auto gl_id = texture.gl_id;
  auto mipmap_levels = texture.mipmap_levels + 1;
  auto storage_internal_format = texture.sized_pixel_format;

  // multisample related
  bool ms_fixedsamplelocations = GL_TRUE;
  auto ms_sample_count = texture.multisample;

  if (is_3d(texture)) {
    verify_dimension(dimensions, 3);
    GFX_GL_CALL(glTextureStorage3D, gl_id, mipmap_levels, storage_internal_format,
          dimensions[0], dimensions[1], dimensions[2]);

  } else if (is_2d(texture)) {
    verify_dimension(dimensions, 2);
    GFX_GL_CALL(glTextureStorage2D, gl_id, mipmap_levels, storage_internal_format,
          dimensions[0], dimensions[1]);

  } else if (is_1d(texture)) {
    verify_dimension(dimensions, 1);
    GFX_GL_CALL(glTextureStorage1D, gl_id, mipmap_levels, storage_internal_format,
        dimensions[0]);

  } else if (texture.gl_type == GL_TEXTURE_2D_MULTISAMPLE) {
    verify_dimension(dimensions, 2);
    verify_dimension(&ms_sample_count, 1);
    GFX_GL_CALL(glTextureStorage2DMultisample, gl_id, ms_sample_count, storage_internal_format,
        dimensions[0], dimensions[1], ms_fixedsamplelocations);

  } else if (texture.gl_type == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
    verify_dimension(dimensions, 3);
    verify_dimension(&ms_sample_count, 1);
    GFX_GL_CALL(glTextureStorage3DMultisample, gl_id, ms_sample_count, storage_internal_format,
        dimensions[0], dimensions[1], dimensions[2], ms_fixedsamplelocations);

  } else {
    GFX_FAIL("Tried to allocate_texture using unsupported texture type "
      "(probably GL_TEXTURE_RECTANGLE or GL_TEXTURE_BUFFER or GL_PROXY_TEXTURE_* or some compressed type)");
  }
}
