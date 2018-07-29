#version 450
in layout(location=0) vec3 in_Position;
in layout(location=1) vec3 in_Normal;
out vec3 v_Normal;
uniform mat4 g_MVP;

void main(void) {
  gl_Position = g_MVP * vec4(in_Position, 1.0f);
  v_Normal = in_Normal;
}
