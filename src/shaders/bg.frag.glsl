#version 450

uniform vec4 g_color_top;
uniform vec4 g_color_bottom;
uniform float g_screenHeight;

out vec4 out_color;

void main() {
	float fac = gl_FragCoord.y / g_screenHeight;
	out_color = mix(g_color_bottom, g_color_top, fac);
}
