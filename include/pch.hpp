#pragma once

// TODO actually use as pch

// #include <cassert>
#include <string>

///
/// types
#include <inttypes.h>
typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;


///
/// macros

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

/* This will let macros expand before concating them */
#define CONCAT_(x, y) x ## y
#define CONCAT(x, y) CONCAT_(x, y)

#define UNUSED(x) (void)(x)

/* This counts the number of args */
#define NARGS_SEQ(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,N,...) N
#define NARGS(...) NARGS_SEQ(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

/* This will call a macro on each argument passed in */
#define APPLY(macro, ...) CONCAT(APPLY_, NARGS(__VA_ARGS__))(macro, __VA_ARGS__)
#define  APPLY_1(m, x1)                                                m(x1)
#define  APPLY_2(m, x1, x2)                                            m(x1) m(x2)
#define  APPLY_3(m, x1, x2, x3)                                        m(x1) m(x2) m(x3)
#define  APPLY_4(m, x1, x2, x3, x4)                                    m(x1) m(x2) m(x3) m(x4)
#define  APPLY_5(m, x1, x2, x3, x4, x5)                                m(x1) m(x2) m(x3) m(x4) m(x5)
#define  APPLY_6(m, x1, x2, x3, x4, x5, x6)                            m(x1) m(x2) m(x3) m(x4) m(x5) m(x6)
#define  APPLY_7(m, x1, x2, x3, x4, x5, x6, x7)                        m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7)
#define  APPLY_8(m, x1, x2, x3, x4, x5, x6, x7, x8)                    m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8)
#define  APPLY_9(m, x1, x2, x3, x4, x5, x6, x7, x8, x9)                m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9)
#define APPLY_10(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10)           m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10)
#define APPLY_11(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11)      m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10) m(x11)
#define APPLY_12(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12) m(x1) m(x2) m(x3) m(x4) m(x5) m(x6) m(x7) m(x8) m(x9) m(x10) m(x11) m(x12)



/*
///
/// Breakpoint from code

namespace utils {
  bool is_debugger_present();
}

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
  #define DEBUG_BREAK() if (::utils::is_debugger_present()){ __debugbreak();}
#else
  #define DEBUG_BREAK() if (::utils::is_debugger_present()){ asm("int3");}
#endif
*/

///
/// enum class utils
#include <type_traits>

template<class enumeration>
constexpr std::size_t enum_count() noexcept {
  static_assert(std::is_enum<enumeration>::value, "Not an enum" );
  return static_cast< std::size_t >( enumeration::Count );
}

template<class enumeration>
constexpr std::size_t enum_int(const enumeration value) noexcept {
  static_assert(std::is_enum<enumeration>::value, "Not an enum" );
  return static_cast< std::size_t>(value);
}

template <class enumeration>
constexpr enumeration int_enum(const std::size_t value) noexcept {
  return static_cast<enumeration>(value);
}


///
/// functions / classes
#include "logging/Logger.hpp"
#include "gl-utils/main.hpp"
