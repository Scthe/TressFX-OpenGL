#version 450

// #define TAIL_COMPRESS 1

#pragma include "_utils.glsl"
// #pragma include "lib/TressFXRendering.shading.glsl"
#pragma include "lib/TressFXPPLL.glsl"
#pragma include "lib/TressFXPPLL.resolve.glsl" // uses TressFXRendering.shading


// START ppll_resolve.frag
// (main)

uniform vec3 g_vEye;

const int RENDER_MODE_FILL_ONE_COLOR = 1;
const int RENDER_MODE_PPLL_DEPTH = 2;
uniform int g_RenderMode;

out vec4 ps_outputColor;

layout(early_fragment_tests) in; // [earlydepthstencil]


// <editor-fold debug>
const float MAX_DEBUG_LIST_DEPTH = 8;
const vec3 DEBUG_LIST_0 = vec3(0,0,1);
const vec3 DEBUG_LIST_1 = vec3(1,0,0);

uint debug_count_list_len (vec2 vfScreenAddress) {
  uint result = 0;
  uint pointer = imageLoad(tFragmentListHead, ivec2(vfScreenAddress)).r;
  uint iter = 0;

  while (pointer != FRAGMENT_LIST_NULL && iter < MAX_DEBUG_LIST_DEPTH) {
    pointer = NODE_NEXT(pointer);
    ++result;
    ++iter;
  }

  return result;
}
// </editor-fold>


void main () {
  // we could split the shaders into 3. Or we can shamelessly 'if'

  if (g_RenderMode == RENDER_MODE_FILL_ONE_COLOR) {
    ps_outputColor = vec4(1,1,0,1);
  } else if (g_RenderMode == RENDER_MODE_PPLL_DEPTH) {
    uint depth = debug_count_list_len(gl_FragCoord.xy);
    float fac = clamp(float(depth) / MAX_DEBUG_LIST_DEPTH, 0, 1);
    ps_outputColor = vec4(mix(DEBUG_LIST_0, DEBUG_LIST_1, fac), 1);
  } else {
    vec4 color = GatherLinkedList(gl_FragCoord.xy);
    // ps_outputColor = vec4(gammaFix(tonemapReinhard(color.rgb), GAMMA), color.a);
    // ps_outputColor = vec4(gammaFix(color.rgb, GAMMA), color.a);
    ps_outputColor = vec4(color.rgb, color.a);
  }
}


// unroll K-buffer
/*

*/
