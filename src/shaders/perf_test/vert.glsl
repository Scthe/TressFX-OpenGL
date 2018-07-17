#version 450

in layout(location=0) int in_Const1;
out vec4 ps_test;

const float DD = 0.2;
const vec2 TOP_LEFT  = vec2(-DD,  DD);
const vec2 TOP_RIGHT = vec2( DD,  DD);
const vec2 BOT_LEFT  = vec2(-DD, -DD);
const vec2 BOT_RIGHT = vec2( DD, -DD);

const vec2 array[6] = vec2[6](
	TOP_LEFT, BOT_LEFT, TOP_RIGHT,
	TOP_RIGHT, BOT_LEFT, BOT_RIGHT
);

void main(void) {
  vec2 xy = array[gl_VertexID % 6];
  // vec2 xy = array[2];
  gl_Position = vec4(xy, 1.0, 1.0);
	// gl_Position = vec4(BOT_RIGHT, 1.0, 1.0);
	ps_test = vec4(1,0,0,1);
}
