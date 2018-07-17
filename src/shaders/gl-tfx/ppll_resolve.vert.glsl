#version 450

#pragma include "../_fullscreen_pass.glsl"

// out vec4 ps_vPosition;
// out vec2 ps_vTex;

void main() {
	vec2 xy = get_fullscreen_pos();
	gl_Position = vec4(xy, 1.0, 1.0);

  // ps_vPosition = vec4(xy, 0.0, 1.0);
  // ps_vTex = vec2(xy.x, -xy.y) * 0.5 + 0.5; // y down.
}
