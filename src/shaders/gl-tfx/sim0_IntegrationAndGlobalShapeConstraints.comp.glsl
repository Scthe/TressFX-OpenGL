#version 450

#pragma include "_utils.glsl"
#pragma include "sim/_SimParams.mock.comp.glsl"
// #pragma include "sim/_SimParams.comp.glsl"
#pragma include "sim/_SimCommon.comp.glsl"
#pragma include "sim/_SimBuffers.comp.glsl"
#pragma include "sim/_SimQuat.comp.glsl"

shared vec4 sharedPos[THREAD_GROUP_SIZE];
shared vec4 sharedTangent[THREAD_GROUP_SIZE];
shared float sharedLength[THREAD_GROUP_SIZE];

// Uses Verlet integration to calculate the new position for the current time step
vec4 Integrate(
  vec4 curPosition, vec4 oldPosition, vec4 initialPos,
  vec4 force, float dampingCoeff,
  uint globalVertexIndex, uint localVertexIndex, uint numVerticesInTheStrand)
{
  vec4 outputPos = curPosition;

  force.xyz += g_GravityMagnitude * vec3(0, -1.0f, 0);
  vec3 posDelta = curPosition.xyz - oldPosition.xyz;
  outputPos.xyz = curPosition.xyz
                  + (1.0 - dampingCoeff) * posDelta
                  + force.xyz * g_TimeStep * g_TimeStep;

  return outputPos;
}

// Updates the  hair vertex positions based on the physics simulation
void UpdateFinalVertexPositions(
  vec4 oldPosition, vec4 newPosition,
  uint globalVertexIndex, uint localVertexIndex, uint numVerticesInTheStrand)
{
  g_HairVertexPositionsPrevPrev_[globalVertexIndex] = g_HairVertexPositionsPrev_[globalVertexIndex];
  g_HairVertexPositionsPrev_[globalVertexIndex] = oldPosition;
  g_HairVertexPositions_[globalVertexIndex] = newPosition;
}

vec3 ApplyVertexBoneSkinning(vec3 vertexPos, inout vec4 bone_quat) {
  // Simplified for now, at the bottom of the file there is full skinning procedure
  mat4 bone_matrix = mat4(1.0);
  bone_quat = MakeQuaternion(bone_matrix);
  return (bone_matrix * vec4(vertexPos, 1)).xyz;
}

// Compute shader to simulate the gravitational force with integration
// and to maintain the global shape constraints.
//   1) Apply skinning
//   2) Integrate using forces (only gravity ATM)
//   3) Try to go back to initial position (global shape constaint)
//   4) Write to all g_HairVertexPositions*_ SSBOs
//
// One thread computes one vertex.
//
layout (local_size_x = THREAD_GROUP_SIZE) in; // [numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main() {
  uint globalStrandIndex, localStrandIndex, globalVertexIndex, localVertexIndex, numVerticesInTheStrand, indexForSharedMem, strandType;
  CalcIndicesInVertexLevelMaster(gl_LocalInvocationIndex, gl_WorkGroupID.x, globalStrandIndex, localStrandIndex, globalVertexIndex, localVertexIndex, numVerticesInTheStrand, indexForSharedMem, strandType);

  // Apply bone skinning to initial position
  // BoneSkinningData skinningData = g_BoneSkinningData[globalStrandIndex];
  vec4 bone_quat;
  vec4 initialPos = g_InitialHairPositions_[globalVertexIndex]; // rest position
  initialPos.xyz = ApplyVertexBoneSkinning(initialPos.xyz, /*skinningData,*/ bone_quat);
  // we temporarily use g_HairVertexTangents to hold bone quaternion data compute in ApplyVertexBoneSkinning.
  g_HairVertexTangents_[globalStrandIndex] = bone_quat;

  // position when this step starts. In other words, a position from the last step.
  vec4 currentPos = sharedPos[indexForSharedMem] = g_HairVertexPositions_[globalVertexIndex];

  groupMemoryBarrier(); // GroupMemoryBarrierWithGroupSync();

  // Integrate
  vec4 oldPos = g_HairVertexPositionsPrev_[globalVertexIndex];
  vec4 force = vec4(0, 0, 0, 0);
  if (IsMovable(currentPos)){
    float damping = GetDamping(strandType); // 1.0f;
    sharedPos[indexForSharedMem] = Integrate(
      currentPos, oldPos, initialPos,
      force, damping,
      globalVertexIndex, localVertexIndex, numVerticesInTheStrand);
  } else {
    sharedPos[indexForSharedMem] = initialPos;
  }

  // Global Shape Constraints
  // (Calc delta to initial position and move in that direction)
  float stiffnessForGlobalShapeMatching = GetGlobalStiffness(strandType);
  float globalShapeMatchingEffectiveRange = GetGlobalRange(strandType);
  bool hasStiffness = stiffnessForGlobalShapeMatching > 0;// && globalShapeMatchingEffectiveRange;
  bool isMovable_tmp = IsMovable(sharedPos[indexForSharedMem]);
  bool closeEnoughToRoot = float(localVertexIndex) < globalShapeMatchingEffectiveRange * float(numVerticesInTheStrand);

  if (hasStiffness && isMovable_tmp && closeEnoughToRoot) {
    float factor = stiffnessForGlobalShapeMatching;
    vec3 del = factor * (initialPos - sharedPos[indexForSharedMem]).xyz;
    sharedPos[indexForSharedMem].xyz += del;
  }

  // update global position buffers
  UpdateFinalVertexPositions(
    currentPos, sharedPos[indexForSharedMem],
    globalVertexIndex, localVertexIndex, numVerticesInTheStrand
  );
}

/*
vec3 ApplyVertexBoneSkinning(vec3 vertexPos, BoneSkinningData skinningData, inout vec4 bone_quat) {
  vec3 newVertexPos;

  {
    // Interpolate world space bone matrices using weights.
    row_major mat4 bone_matrix = g_BoneSkinningMatrix[skinningData.boneIndex[0]] * skinningData.boneWeight[0];
    float weight_sum = skinningData.boneWeight[0];

    for (int i = 1; i < 4; i++) {
      if (skinningData.boneWeight[i] > 0) {
        bone_matrix += g_BoneSkinningMatrix[skinningData.boneIndex[i]] * skinningData.boneWeight[i];
        weight_sum += skinningData.boneWeight[i];
      }
    }

    bone_matrix /= weight_sum;
    bone_quat = MakeQuaternion(bone_matrix);

    newVertexPos = mul(vec4(vertexPos, 1), bone_matrix).xyz;
  }

  return newVertexPos;
}
*/
