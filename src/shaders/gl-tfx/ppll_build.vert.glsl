#version 450

#pragma include "_utils.glsl"
#pragma include "lib/TressFXStrands.glsl"

// START ppll_build.vert
// (main)


// model-view-projection matrix
uniform mat4 g_mVP;
// camera position
uniform vec3 g_vEye;

// in layout(location=0) vec3 in_Position; // unused
out vec4 ps_Position;
out vec4 ps_Tangent;
out vec4 ps_p0p1;
out vec4 ps_strandColor;

void main(void) {
  TressFXVertex tressfxVert = GetExpandedTressFXVert(gl_VertexID, g_vEye, g_mVP);
  gl_Position = tressfxVert.Position;
  ps_Position = tressfxVert.Position;
  ps_Tangent  = tressfxVert.Tangent;
  ps_p0p1     = tressfxVert.p0p1;
  ps_strandColor = tressfxVert.strandColor;
}
