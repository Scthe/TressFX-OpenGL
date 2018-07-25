#version 450

#pragma include "_utils.glsl"
#pragma include "sim/_SimParams.comp.glsl"
#pragma include "sim/_SimCommon.comp.glsl"
#pragma include "sim/_SimBuffers.comp.glsl"
#pragma include "sim/_SimQuat.comp.glsl"

// Propagate velocity shock resulted by attached based mesh
//   1) Calculate (rotation + translation) that root strand vertex was subjected to
//   2) Propagate (rotation + translation) from parent to child vertices in same strand,
//      using vspCoeff(set as uniform in simulation settings) as a weight
//   3) Write values to g_HairVertexPositions_, g_HairVertexPositionsPrev_
//      for child vertices
//
// One thread computes one strand.
//
layout (local_size_x = THREAD_GROUP_SIZE) in; // [numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main() {
	uint local_id, group_id, globalStrandIndex, numVerticesInTheStrand, globalRootVertexIndex, strandType;
	CalcIndicesInStrandLevelMaster(gl_LocalInvocationIndex, gl_WorkGroupID.x, globalStrandIndex, numVerticesInTheStrand, globalRootVertexIndex, strandType);

	// since one thread == one strand, we base the work on (root) -> (root+1) vertices
	vec4 pos_old_old[2]; // *previous previous* positions for vertex 0 (root) and vertex 1.
	vec4 pos_old[2];     //          *previous* positions for vertex 0 (root) and vertex 1.
	vec4 pos_new[2];     //           *current* positions for vertex 0 (root) and vertex 1.

	pos_old_old[0] = g_HairVertexPositionsPrevPrev_[globalRootVertexIndex];
	pos_old_old[1] = g_HairVertexPositionsPrevPrev_[globalRootVertexIndex + 1];

	pos_old[0] = g_HairVertexPositionsPrev_[globalRootVertexIndex];
	pos_old[1] = g_HairVertexPositionsPrev_[globalRootVertexIndex + 1];

	pos_new[0] = g_HairVertexPositions_[globalRootVertexIndex];
	pos_new[1] = g_HairVertexPositions_[globalRootVertexIndex + 1];

	// 'down the strand' normalized vectors
	vec3 u = normalize(pos_old[1].xyz - pos_old[0].xyz);
	vec3 v = normalize(pos_new[1].xyz - pos_new[0].xyz);

	// Compute rotation and translation which transform pos_old to pos_new.
	// Since the first two vertices are immovable, we can assume that there
	// is no scaling during tranform.
	vec4 rot = QuatFromTwoUnitVectors(u, v);
	vec3 trans = pos_new[0].xyz - MultQuaternionAndVector(rot, pos_old[0].xyz);

	float vspCoeff = GetVelocityShockPropogation();
	float vspAccelThreshold  = GetVSPAccelThreshold(); // purely shock propagation driven threshold
	float restLength0 = g_HairRestLengthSRV_[globalRootVertexIndex];

	// Increase the VSP coefficient by checking pseudo-acceleration
	// to handle over-stretching when the character moves very fast
	// (this is just A from Verlet Integration)
	float accel = length(pos_new[1] - 2.0 * pos_old[1] + pos_old_old[1]);
	if (accel > vspAccelThreshold){ // TODO: expose this value?
		vspCoeff = 1.0f;
	}

	// for all children vertices that are further from root then 1st vertex
	for (uint i = 2; i < numVerticesInTheStrand; i++) {
		uint globalVertexIndex = globalRootVertexIndex + i; // i := localVertexIndex

		vec4 pos_new_n = g_HairVertexPositions_[globalVertexIndex];
		vec4 pos_old_n = g_HairVertexPositionsPrev_[globalVertexIndex];

		// Using vspCoeff as a weight combine position from sim0_IntegrationAndGlobalShapeConstraints
		// and 'where it would be if we applied same (rotation + trans) that parent was subjected to'.
		// This essentially propagates some of the movement from parent to children
		vec3 pos_new_propagated = MultQuaternionAndVector(rot, pos_new_n.xyz) + trans;
		vec3 pos_old_propagated = MultQuaternionAndVector(rot, pos_old_n.xyz) + trans;
		pos_new_n.xyz = (1.f - vspCoeff) * pos_new_n.xyz
									  + vspCoeff * pos_new_propagated;
		pos_old_n.xyz = (1.f - vspCoeff) * pos_old_n.xyz
									  + vspCoeff * pos_old_propagated;

		// simple write back to where we red from in 1st place
		g_HairVertexPositions_[globalVertexIndex].xyz = pos_new_n.xyz;
		g_HairVertexPositionsPrev_[globalVertexIndex].xyz = pos_old_n.xyz;
	}
}
