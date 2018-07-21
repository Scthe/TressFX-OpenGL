#version 450

#pragma include "_utils.glsl"
#pragma include "lib/TressFXPPLL.glsl"
#pragma include "lib/TressFXPPLL.resolve.glsl"

// START ppll_resolve.frag
// (main)

// uniform mat4 g_mInvViewProj;
// uniform vec4 g_vViewport;
uniform vec3 g_vEye;

// in vec4 ps_vPosition; // unused: gl_FragCoord?
// in vec2 ps_vTex;
out vec4 ps_outputColor;

layout(early_fragment_tests) in; // [earlydepthstencil]

void main () {
  ps_outputColor = GatherLinkedList(gl_FragCoord.xy);

  // uint pointer = imageLoad(tFragmentListHead, ivec2(gl_FragCoord.xy)).r;
  // if (pointer == FRAGMENT_LIST_NULL) {
    // discard;
  // }

  // if (gl_FragCoord.x < 0) {
    // vec4 c = vec4( 1.0 );
    // c = vec4( float(NODE_DATA(0)) );
    // imageAtomicExchange(tFragmentListHead, ivec2(gl_FragCoord.xy), 32);
  // }
  ps_outputColor = vec4(1,1,0,1);
}


// unroll K-buffer
/*
vec4 ComputeSushiRGBA(vec2 pixelCoord, float depth, vec4 vTangentCoverage, vec3 baseColor) {
  vec3 vTangent = 2.0 * vTangentCoverage.xyz - 1.0;
  vec3 vNDC = ScreenToNDC(vec3(pixelCoord, depth), g_vViewport);
  vec3 vPositionWS = NDCToWorld(vNDC, g_mInvViewProj);
  vec3 vViewWS = g_vEye - vPositionWS;

  // TODO remove params, since we are using globals anyways.
  HairShadeParams params;

  params.cColor = baseColor;
  params.fRadius = g_FiberRadius;
  params.fSpacing = g_FiberSpacing;
  params.fAlpha = g_HairShadowAlpha;

  vec3 color = AccumulateHairLight(vTangent, vPositionWS, vViewWS, params);
  return vec4(color, vTangentCoverage.w);
 return vec4(baseColor, 1.0);
}
#define HEAD_SHADING ComputeSushiRGBA
#define TAIL_SHADING ComputeSushiRGBA
*/
