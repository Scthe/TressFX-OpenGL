#pragma once

namespace glUtils {

  /** @see https://github.com/glium/glium/blob/cd1bab4f6b7c3b48391289b3a10be115f660b252/src/draw_parameters/blend.rs#L123 for good overview */
  struct BlendingFactor {
      static const GLenum Zero                      = GL_ZERO; // nope
      static const GLenum One                       = GL_ONE; // replace
      // static const GLenum SourceAlphaSaturate       = GL_SRC_ALPHA_SATURATE; // use GL_SRC_ALPHA_SATURATE instead of BlendingFactor::_

      static const GLenum SourceColor               = GL_SRC_COLOR;// src
      static const GLenum OneMinusSourceColor       = GL_ONE_MINUS_SRC_COLOR;// 1-src
      static const GLenum DestinationColor          = GL_DST_COLOR;// dest
      static const GLenum OneMinusDestinationColor  = GL_ONE_MINUS_DST_COLOR;// 1-dest
      static const GLenum ConstantColor             = GL_CONSTANT_COLOR; // C
      static const GLenum OneMinusConstantColor     = GL_ONE_MINUS_CONSTANT_COLOR; // 1-C

      static const GLenum SourceAlpha               = GL_SRC_ALPHA; // Alpha: src
      static const GLenum OneMinusSourceAlpha       = GL_ONE_MINUS_SRC_ALPHA; // Alpha: 1-src
      static const GLenum DestinationAlpha          = GL_DST_ALPHA; // Alpha: dest
      static const GLenum OneMinusDestinationAlpha  = GL_ONE_MINUS_DST_ALPHA; // Alpha: 1-dest
      static const GLenum ConstantAlpha             = GL_CONSTANT_ALPHA; // Alpha: C
      static const GLenum OneMinusConstantAlpha     = GL_ONE_MINUS_CONSTANT_ALPHA; // Alpha: 1-C
  };
  typedef GLenum BlendingFactor_;

  /** @see https://github.com/glium/glium/blob/cd1bab4f6b7c3b48391289b3a10be115f660b252/src/draw_parameters/blend.rs#L55 for good overview */
  struct BlendingFunction {
    static const GLenum AlwaysReplace =  GL_NONE; // dummy value, will be handled manually
    static const GLenum Min =  GL_MIN;
    static const GLenum Max =  GL_MAX;
    static const GLenum Addition =  GL_FUNC_ADD;
    static const GLenum Subtraction =  GL_FUNC_SUBTRACT;
    static const GLenum ReverseSubtraction =  GL_FUNC_REVERSE_SUBTRACT;
  };
  typedef GLenum BlendingFunction_;

  struct BlendingMode {
    BlendingFunction_ function = BlendingFunction::AlwaysReplace;

    /** destination color (the one that is already on texture) */
    BlendingFactor_ current_value_factor = BlendingFactor::Zero;

    /** source color (the one from pixel shader) */
    BlendingFactor_ new_value_factor = BlendingFactor::One;
  };

  struct Blend {
    BlendingMode color;
    BlendingMode alpha;
    glm::vec4 constant_value = {1.0, 1.0, 1.0, 1.0};
  };

  inline bool operator==(const BlendingMode& a, const BlendingMode& b) {
    return (a.function == b.function)
        && (a.current_value_factor == b.current_value_factor)
        && (a.new_value_factor == b.new_value_factor);
  }
  inline bool operator!=(const BlendingMode& a, const BlendingMode& b) {
    return !(a == b);
  }

  inline bool operator==(const Blend& a, const Blend& b) {
    return (a.alpha == b.alpha)
        && (a.color == b.color)
        && (a.constant_value == b.constant_value);
  }
  inline bool operator!=(const Blend& a, const Blend& b) {
    return !(a == b);
  }

}
