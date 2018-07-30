#version 450
in layout(location=0) vec3 in_Position;
uniform mat4 g_MVP;
uniform vec4 g_Capsules[4];

void main(void) {
  vec4 capsule = g_Capsules[gl_InstanceID];
  vec4 position = vec4(in_Position * capsule.w, 1.0f);
  position.xyz += capsule.xyz;
  gl_Position = g_MVP * position;
}
