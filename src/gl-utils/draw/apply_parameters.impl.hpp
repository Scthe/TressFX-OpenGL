#pragma once

static void sync_depth(const glUtils::Depth&, const glUtils::Depth*const);
static void sync_stencil(const glUtils::Stencil&, const glUtils::Stencil*const);
static void sync_blend(const glUtils::Blend&, const glUtils::Blend*const);
static void toggle_state_bool(GLenum name, bool);

#define CHANGED(PARAM_NAME) (!old_state || old_state->PARAM_NAME != new_params.PARAM_NAME)

namespace glUtils {

  void apply_draw_parameters(const DrawParameters& new_params, const DrawParameters*const old_state) {
    // https://github.com/glium/glium/blob/master/src/draw_parameters/mod.rs#L476

    sync_depth(new_params.depth, old_state ? &old_state->depth : nullptr);
    sync_stencil(new_params.stencil, old_state ? &old_state->stencil : nullptr);
    sync_blend(new_params.blend, old_state ? &old_state->blend : nullptr);

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
      GFX_GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, new_params.polygon_mode);
    }

    // dithering
    if (CHANGED(dithering)) {
      toggle_state_bool(GL_DITHER, new_params.dithering);
    }

    // backface_culling
    if (new_params.culling == CullingMode::None) {
      toggle_state_bool(GL_CULL_FACE, false);
    } else {
      toggle_state_bool(GL_CULL_FACE, true);
      GFX_GL_CALL(glCullFace, new_params.culling);
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
static bool is_stencil_noop(const glUtils::StencilPerSide& settings) {
  using namespace glUtils;
  bool all_noop = settings.op_stencil_fail == StencilOperation::Keep
               && settings.op_stencil_pass_depth_fail == StencilOperation::Keep
               && settings.op_pass == StencilOperation::Keep;
  return all_noop && settings.test == StencilTest::AlwaysPass;
}

static bool is_same_stencil_ops (const glUtils::StencilPerSide& front, const glUtils::StencilPerSide& back) {
  return front.op_stencil_fail == back.op_stencil_fail
      && front.op_stencil_pass_depth_fail == back.op_stencil_pass_depth_fail
      && front.op_pass == back.op_pass;
}

void sync_stencil(const glUtils::Stencil& new_params, const glUtils::Stencil*const old_state) {
  // https://github.com/glium/glium/blob/cd1bab4f6b7c3b48391289b3a10be115f660b252/src/draw_parameters/stencil.rs#L223
  if (old_state && (*old_state == new_params)) {
    return;
  }
  const auto& front = new_params.front;
  const auto& back = new_params.back;
  const GLint ref_value = new_params.reference_value;
  const GLuint compare_mask = new_params.compare_mask;

  toggle_state_bool(GL_STENCIL_TEST, !is_stencil_noop(front) || is_stencil_noop(back));

  // sync test
  if (front.test == back.test) {
    GFX_GL_CALL(glStencilFunc, front.test, ref_value, compare_mask);
  } else {
    GFX_GL_CALL(glStencilFuncSeparate, GL_FRONT, front.test, ref_value, compare_mask);
    GFX_GL_CALL(glStencilFuncSeparate, GL_BACK, back.test, ref_value, compare_mask);
  }

  // sync write mask
  GFX_GL_CALL(glStencilMask, new_params.write_bytes);

  // sync ops
  if (is_same_stencil_ops(front, back)) {
    GFX_GL_CALL(glStencilOp,
      front.op_stencil_fail,
      front.op_stencil_pass_depth_fail,
      front.op_pass);
  } else {
    GFX_GL_CALL(glStencilOpSeparate, GL_FRONT,
      front.op_stencil_fail,
      front.op_stencil_pass_depth_fail,
      front.op_pass);
    GFX_GL_CALL(glStencilOpSeparate, GL_BACK,
      back.op_stencil_fail,
      back.op_stencil_pass_depth_fail,
      back.op_pass);
  }
}

glUtils::BlendingMode eval_blend_mode (const glUtils::BlendingMode src) {
  using namespace glUtils;
  if (src.function != BlendingFunction::AlwaysReplace) {
    return src;
  }
  return {BlendingFunction::Addition, BlendingFactor::Zero, BlendingFactor::One};
}

// TODO expose, since multiple textures in fbo are possible. Tho vary of glEnable
void sync_blend(const glUtils::Blend& new_params, const glUtils::Blend*const old_state) {
  // https://github.com/glium/glium/blob/cd1bab4f6b7c3b48391289b3a10be115f660b252/src/draw_parameters/blend.rs#L205
  using namespace glUtils;
  if (old_state && new_params == *old_state) {
    return;
  }

  bool just_replace_col = new_params.color.function == BlendingFunction::AlwaysReplace;
  bool just_replace_alpha = new_params.alpha.function == BlendingFunction::AlwaysReplace;
  toggle_state_bool(GL_BLEND, !just_replace_col || !just_replace_alpha);

  GFX_GL_CALL(glBlendColor,
    new_params.constant_value.r,
    new_params.constant_value.g,
    new_params.constant_value.b,
    new_params.constant_value.a
  );

  // handle AlwaysReplace artificial mode
  const auto color = eval_blend_mode(new_params.color);
  const auto alpha = eval_blend_mode(new_params.alpha);

  GFX_GL_CALL(glBlendEquationSeparate, color.function, alpha.function);

  GFX_GL_CALL(glBlendFuncSeparate,
    color.new_value_factor,
    color.current_value_factor,
    alpha.new_value_factor,
    alpha.current_value_factor
  );
}

#undef CHANGED
