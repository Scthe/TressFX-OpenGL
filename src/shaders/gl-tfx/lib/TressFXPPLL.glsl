// START TressFXPPLL.glsl

struct PPLL_STRUCT {
  uint depth;
  uint data;
  uint color;
  uint uNext;
};

#define FRAGMENT_LIST_NULL 0xffffffff
// #define HAS_COLOR 1

uint PackFloat4IntoUint(vec4 vValue) {
  return ((uint(vValue.x * 255)) << 24) |
         ((uint(vValue.y * 255)) << 16) |
         ((uint(vValue.z * 255)) << 8) |
           uint(vValue.w * 255);
}

vec4 UnpackUintIntoFloat4(uint uValue) {
  uvec4 v = uvec4(
    (uValue & 0xFF000000) >> 24,
    (uValue & 0x00FF0000) >> 16,
    (uValue & 0x0000FF00) >> 8,
    (uValue & 0x000000FF)
  );
  return vec4(v) / 255.0;
}

// END TressFXPPLL.glsl
