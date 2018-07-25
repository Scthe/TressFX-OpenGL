#version 450

// in vec3 position;

uniform vec4 g_WindDirection;
uniform vec3 g_Eye;
uniform mat4 g_VP;
// uniform mat4 g_matProj;
// uniform mat4 g_matView;

const float TIP_DISTANCE = 0.3;
const float END_DISTANCE = 0.5;
const float END_SPREAD = 0.05;

void main() {
	vec3 windDir = -g_WindDirection.xyz;

	vec3 tail = windDir * END_DISTANCE;
	vec3 towardsCamera = normalize(tail.xyz - g_Eye);
	vec3 up = cross(normalize(windDir), towardsCamera);
	up = normalize(up);

	vec3 vertices[3] = {
		windDir * TIP_DISTANCE,
		tail + END_SPREAD * up,
		tail - END_SPREAD * up
	};
  gl_Position = g_VP * vec4(vertices[gl_VertexID], 1);
	// gl_Position = g_matProj * g_matView * vec4(vertices[gl_VertexID], 1);
	// gl_Position = p / p.w;
}
