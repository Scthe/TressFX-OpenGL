#pragma once

#include "blend.hpp"
#include "depth.hpp"
#include "stencil.hpp"

namespace glUtils {

  struct CullingMode {
    static const GLenum None = GL_NONE; // show all
    static const GLenum CullFront = GL_FRONT; // CCW, hides front-facing faces
    static const GLenum CullBack = GL_BACK; // CW, hides back-facing faces
  };
  typedef GLenum CullingMode_;

  struct PolygonMode {
    static const GLenum Point = GL_POINT;
    static const GLenum Line = GL_LINE;
    static const GLenum Fill = GL_FILL;
  };
  typedef GLenum PolygonMode_;

  // enum class Smooth {
      // Fastest = GL_FASTEST,
      // Nicest = GL_NICEST,
      // DontCare = GL_DONT_CARE,
  // }

  /**
   * https://github.com/glium/glium/blob/master/src/draw_parameters/mod.rs#L246
   */
  struct DrawParameters {
    Depth depth;
    Stencil stencil;
    Blend blend;
    f32 line_width = 1.0f;
    f32 point_size =  1.0f;
    PolygonMode_ polygon_mode = PolygonMode::Fill;
    bool dithering = false; // smoothen transition between colors
    CullingMode_ culling = CullingMode::CullBack;

    // It also affects glClear!
    bool color_write[4] = {true, true, true, true};

    // Unimplemented:
    // Smooth smooth; // Requires blending to be on
    // bool multisampling = false; // better do at window creation
    // ? polygon_offset; // glPolygonOffset
    // SampleCoverage coverage; // glSampleCoverage
    // params.draw_primitives =  true;
    // params.samples_passed_query =  None;
    // params.time_elapsed_query =  None;
    // params.primitives_generated_query =  None;
    // params.transform_feedback_primitives_written_query =  None;
    // params.condition =  None;
    // params.transform_feedback =  None;
    // params.provoking_vertex =  ProvokingVertex::LastVertex;
    // params.primitive_bounding_box =  (-1.0 .. 1.0, -1.0 .. 1.0, -1.0 .. 1.0, -1.0 .. 1.0);
    // params.primitive_restart_index =  false;

    DrawParameters(){}
  };

}
