#version 450

in vec3 position;


const vec2 TOP_LEFT  = vec2(-1.0, 1.0);
const vec2 TOP_RIGHT = vec2( 1.0, 1.0);
const vec2 BOT_LEFT  = vec2(-1.0, -1.0);
const vec2 BOT_RIGHT = vec2( 1.0, -1.0);

const vec2 array[6] = vec2[6](
	TOP_LEFT, BOT_LEFT, TOP_RIGHT,
	TOP_RIGHT, BOT_LEFT, BOT_RIGHT
);

void main() {
	vec2 xy = array[gl_VertexID];
	gl_Position = vec4(xy, 1.0, 1.0);
}
