#version 450
precision highp float;

in vec3 v_Normal;
out vec4 gl_FragColor;

const vec3 LIGHT_POS = vec3(30.0, 30.0, 30.0);
const float LIGHT_FALLOFF = 1.5;
const vec3 AMBIENT = vec3(0.5);
const float GAMMA = 2.2;

vec3 tonemapReinhard (vec3 color) {
  return color / (color + vec3(1.0));
}

vec3 gammaFix (vec3 color, float gamma) {
  return pow(color, vec3(1.0/gamma));
}

void main(void) {
  float dd = dot(normalize(v_Normal), normalize(LIGHT_POS));
  /*
  if (dd < 0.0) {
    gl_FragColor = vec4(0.5, 0.5, 0.5, 1.0);
    // discard;
  } else {
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
  }
  */

  float phong = max(0, dd);
  vec3 direct_light = vec3(phong) * LIGHT_FALLOFF;
  vec3 color = AMBIENT + direct_light;

  color = tonemapReinhard(color);
  gl_FragColor = vec4(gammaFix(color, GAMMA), 1.0);
}
