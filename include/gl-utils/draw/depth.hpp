#pragma once

namespace glUtils {

  struct DepthTest {
    static const GLenum AlwaysFail = GL_NEVER;
    static const GLenum AlwaysPass = GL_ALWAYS;
    static const GLenum IfEqual = GL_EQUAL;
    static const GLenum IfNotEqual = GL_NOTEQUAL;
    static const GLenum IfMore = GL_GREATER;
    static const GLenum IfMoreOrEqual = GL_GEQUAL;
    static const GLenum IfLess = GL_LESS;
    static const GLenum IfLessOrEqual = GL_LEQUAL;
  };
  typedef GLenum DepthTest_;

  struct Depth {
    DepthTest_ test = DepthTest::IfLess;
    bool write = true;
    // f32 range[2] = {0, 1};
    // DepthClamp clamp = DepthClamp::NoClamp;
  };

  inline bool operator==(const Depth& a, const Depth& b) {
    return (a.test == b.test) && (a.write == b.write);
  }

  inline bool operator!=(const Depth& a, const Depth& b) {
    return !(a == b);
  }

}
