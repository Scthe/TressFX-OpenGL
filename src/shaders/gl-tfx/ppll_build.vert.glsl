#version 450

#pragma include "_utils.glsl"
#pragma include "lib/TressFXStrands.glsl"

// START ppll_build.vert
// (main)


// model-view-projection matrix
uniform mat4 g_mVP;
// camera position
uniform vec3 g_vEye;
// some viewport info
uniform vec4 g_vViewport;

// in layout(location=0) vec3 in_Position; // unused
out vec4 ps_Position;
out vec4 ps_Tangent;
out vec4 ps_p0p1;
out vec3 ps_strandColor;
// out vec4 ps_test;

// const vec2 TOP_LEFT  = vec2(-1.0, 1.0);
// const vec2 TOP_RIGHT = vec2( 1.0, 1.0);
// const vec2 BOT_LEFT  = vec2(-1.0, -1.0);
// const vec2 BOT_RIGHT = vec2( 0.5, -0.5);

// const float DD = 0.20;
// const vec2 TOP_LEFT  = vec2(-DD,  DD);
// const vec2 TOP_RIGHT = vec2( DD,  DD);
// const vec2 BOT_LEFT  = vec2(-DD, -DD);
// const vec2 BOT_RIGHT = vec2( DD, -DD);

// const vec2 array[6] = vec2[6](
	// TOP_LEFT, BOT_LEFT, TOP_RIGHT,
	// TOP_RIGHT, BOT_LEFT, BOT_RIGHT
// );

void main(void) {
	bool isOdd = (gl_VertexID & 0x01) > 0;

	// if (g_FiberRadius > 20.0) {
  TressFXVertex tressfxVert = GetExpandedTressFXVert(gl_VertexID, g_vEye, g_mVP);
  gl_Position = tressfxVert.Position;
  ps_Position = tressfxVert.Position;
  ps_Tangent  = tressfxVert.Tangent;
  ps_p0p1     = tressfxVert.p0p1;
  // ps_strandColor = tressfxVert.strandColor;
	ps_strandColor = isOdd ? vec3(1,0,0) : vec3(0,1,0);


  // vec2 xy = array[gl_VertexID % 6];
  // gl_Position = vec4(xy, 1.0, 1.0);
	// gl_Position = vec4(BOT_RIGHT, 1.0, 1.0);
	// ps_test = vec4(1,0,0,1);
}
