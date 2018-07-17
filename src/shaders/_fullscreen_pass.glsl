// START _fullscreen_pass

const vec2 TOP_LEFT  = vec2(-1.0, 1.0);
const vec2 TOP_RIGHT = vec2( 1.0, 1.0);
const vec2 BOT_LEFT  = vec2(-1.0, -1.0);
const vec2 BOT_RIGHT = vec2( 1.0, -1.0);

const vec2 FULLSCREEN_PASS_POSITIONS[6] = vec2[6](
	TOP_LEFT, BOT_LEFT, TOP_RIGHT,
	TOP_RIGHT, BOT_LEFT, BOT_RIGHT
);

vec2 get_fullscreen_pos () {
  return FULLSCREEN_PASS_POSITIONS[gl_VertexID];
}

// END _fullscreen_pass
