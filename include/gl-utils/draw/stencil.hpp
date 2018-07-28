#pragma once

namespace glUtils {

  struct StencilTest {
    static const GLenum AlwaysPass = GL_ALWAYS;
    static const GLenum AlwaysFail = GL_NEVER;
    static const GLenum IfRefIsLessThenCurrent = GL_LESS;      // write if this->reference_value < current_stencil_value
    static const GLenum IfRefIsLessOrEqualCurrent = GL_LEQUAL;
    static const GLenum IfRefIsMoreThenCurrent = GL_GREATER;   // write if this->reference_value > current_stencil_value
    static const GLenum IfRefIsMoreOrEqualCurrent = GL_GEQUAL;
    static const GLenum IfRefIsEqualCurrent = GL_EQUAL;        // write if this->reference_value == current_stencil_value
    static const GLenum IfRefIsNotEqualCurrent =  GL_NOTEQUAL; // write if this->reference_value != current_stencil_value
  };
  typedef GLenum StencilTest_;

  struct StencilOperation {
    static const GLenum Keep = GL_KEEP;          // keep current value
    static const GLenum Zero = GL_ZERO;          // write zero
    static const GLenum Replace = GL_REPLACE;    // write reference value
    static const GLenum Increment = GL_INCR;     // min(current_value + 1, MAX_INT)
    static const GLenum IncrementWrap = GL_INCR_WRAP; // let next = current_value + 1; return next == MAX_INT ? 0 : next
    static const GLenum Decrement = GL_DECR;     // max(current_value - 1, MIN_INT)
    static const GLenum DecrementWrap = GL_DECR_WRAP; // let next = current_value - 1; return current_value == 0 ? MAX_INT : next
    static const GLenum Invert = GL_INVERT;      // invert bits
  };
  typedef GLenum StencilOperation_;

  struct StencilPerSide {
    /** Comparison against the existing value in the stencil buffer. */
    StencilTest_ test = StencilTest::AlwaysPass;

    /** Specifies the operation to do when a fragment fails the stencil test. */
    StencilOperation_ op_stencil_fail = StencilOperation::Keep;

    /** Specifies the operation to do when a fragment passes the stencil test but fails the depth test.*/
    StencilOperation_ op_stencil_pass_depth_fail = StencilOperation::Keep;

    /** Specifies the operation to do when a fragment passes both the stencil and depth tests. */
    StencilOperation_ op_pass = StencilOperation::Keep;
  };


  /**
   * Stencil test procedure:
   *   (StencilRefVal & CompareMask) CompTestFunc (StencilBufferValue & CompareMask)
   * Stencil write new value procedure:
   *   (~StencilWriteMask & StencilBufferValue) | (StencilWriteMask & StencilOp(StencilBufferValue))
   *
   * Inspired by D3D11_DEPTH_STENCIL_DESC
   */
  struct Stencil {
    /**
     * Reference value, can be used to:
     *   * compare to in stencil test
     *   * write to stencil buffer (StencilOperation::Replace)
     */
    GLint reference_value = 0;

    /** used for compare, see last arg to glStencilFunc. Also known as ReadMask */
    GLuint compare_mask = 0xffffffff;

    /** Allows specifying a mask when writing data on the stencil buffer. Also known as WriteMask */
    GLuint write_bytes = 0xffffffff;

    StencilPerSide front;
    StencilPerSide back;
  };

  inline bool operator==(const StencilPerSide& a, const StencilPerSide& b) {
    return (a.test == b.test)
        && (a.op_stencil_fail == b.op_stencil_fail)
        && (a.op_stencil_pass_depth_fail == b.op_stencil_pass_depth_fail)
        && (a.op_pass == b.op_pass);
  }
  inline bool operator!=(const StencilPerSide& a, const StencilPerSide& b) {
    return !(a == b);
  }

  inline bool operator==(const Stencil& a, const Stencil& b) {
    return (a.reference_value == b.reference_value)
        && (a.compare_mask == b.compare_mask)
        && (a.write_bytes == b.write_bytes)
        && (a.front == b.front)
        && (a.back == b.back);
  }
  inline bool operator!=(const Stencil& a, const Stencil& b) {
    return !(a == b);
  }

}
