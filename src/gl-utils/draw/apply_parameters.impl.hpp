#pragma once

static void sync_depth(const glUtils::Depth&, const glUtils::Depth*const);
static void sync_stencil(const glUtils::DrawParameters& new_params, const glUtils::DrawParameters*const old_state);
// static void sync_blend(const Blend&);
// static void sync_viewport_scissor(const Rect*const viewport, const Rect*const scissor);
static void toggle_state_bool(GLenum name, bool);

#define CHANGED(PARAM_NAME) (!old_state || old_state->PARAM_NAME != new_params.PARAM_NAME)

namespace glUtils {

  void apply_draw_parameters(const DrawParameters& new_params, const DrawParameters*const old_state) {
    // https://github.com/glium/glium/blob/master/src/draw_parameters/mod.rs#L476

    // depth
    sync_depth(new_params.depth, old_state ? &old_state->depth : nullptr);

    // stencil
    sync_stencil(new_params, old_state);

    // line_width
    if (CHANGED(line_width)) {
      GFX_GL_CALL(glLineWidth, new_params.line_width);
    }

    // point_size
    if (CHANGED(point_size)) {
      GFX_GL_CALL(glPointSize, new_params.point_size);
    }

    // polygon_mode
    if (CHANGED(polygon_mode)) {
      GFX_GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, (GLenum)new_params.polygon_mode);
    }

    // dithering
    if (CHANGED(dithering)) {
      toggle_state_bool(GL_DITHER, new_params.dithering);
    }

    // backface_culling
    if (new_params.backface_culling == BackfaceCullingMode::CullingDisabled) {
      toggle_state_bool(GL_CULL_FACE, false);
    } else {
      toggle_state_bool(GL_CULL_FACE, true);
      GFX_GL_CALL(glCullFace, (GLenum)new_params.backface_culling);
    }

    //color write
    bool colorWriteChanged = !old_state || (
      old_state->color_write[0] != new_params.color_write[0] ||
      old_state->color_write[1] != new_params.color_write[1] ||
      old_state->color_write[2] != new_params.color_write[2] ||
      old_state->color_write[3] != new_params.color_write[3]
    );
    if (colorWriteChanged) {
      const bool*const mask = new_params.color_write;
      GFX_GL_CALL(glColorMask, mask[0], mask[1], mask[2], mask[3]);
    }

  }

}

void toggle_state_bool(GLenum name, bool value) {
  if (value) {
    GFX_GL_CALL(glEnable, name);
  } else {
    GFX_GL_CALL(glDisable, name);
  }
}

void sync_depth(const glUtils::Depth& depth, const glUtils::Depth*const old_depth) {
  // https://github.com/glium/glium/blob/cd1bab4f6b7c3b48391289b3a10be115f660b252/src/draw_parameters/depth.rs#L166
  if (old_depth && depth == *old_depth) {
    return;
  }

  toggle_state_bool(GL_DEPTH_TEST, depth.test != glUtils::DepthTest::AlwaysPass || depth.write);

  GFX_GL_CALL(glDepthFunc, (GLenum)depth.test);
  GFX_GL_CALL(glDepthMask, (GLenum)depth.write);
}

/** test if we always pass but never write */
static bool is_stencil_noop(const glUtils::StencilSettings& settings) {
  using namespace glUtils;
  bool all_noop = settings.op_stencil_fail == StencilOperation::Keep
               && settings.op_stencil_pass_depth_fail == StencilOperation::Keep
               && settings.op_pass == StencilOperation::Keep;
  return all_noop && settings.test == StencilTest::AlwaysPass;
}

static void sync_stencil(GLenum face, const glUtils::StencilSettings& settings) {
  GFX_GL_CALL(glStencilFuncSeparate, face, (GLenum)settings.test,
    (GLenum)settings.reference_value, (GLenum)settings.compare_mask);
  GFX_GL_CALL(glStencilOpSeparate, face,
    (GLenum)settings.op_stencil_fail,
    (GLenum)settings.op_stencil_pass_depth_fail,
    (GLenum)settings.op_pass);
  GFX_GL_CALL(glStencilMaskSeparate, face, (GLenum)settings.write_bytes);
}

void sync_stencil(const glUtils::DrawParameters& new_params, const glUtils::DrawParameters*const old_state) {
  // https://github.com/glium/glium/blob/cd1bab4f6b7c3b48391289b3a10be115f660b252/src/draw_parameters/stencil.rs#L223
  auto& front = new_params.stencil_counter_clockwise;
  auto& back = new_params.stencil_clockwise;
  auto old_front = old_state ? &old_state->stencil_counter_clockwise : nullptr;
  auto old_back = old_state ? &old_state->stencil_clockwise : nullptr;

  if (old_state && (*old_front == front && *old_back == back)) {
    return;
  }

  if (is_stencil_noop(front) && is_stencil_noop(back)) {
    toggle_state_bool(GL_STENCIL_TEST, false);
  } else {
    toggle_state_bool(GL_STENCIL_TEST, true);
    sync_stencil(GL_FRONT, front);
    sync_stencil(GL_BACK, back);
  }
}

#undef CHANGED
