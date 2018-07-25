#version 450

#pragma include "../_fullscreen_pass.glsl"

void main() {
	vec2 xy = get_fullscreen_pos();
	gl_Position = vec4(xy, 1.0, 1.0);
}
