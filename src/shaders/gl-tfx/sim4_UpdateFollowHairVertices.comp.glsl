#version 450

#pragma include "_utils.glsl"
#pragma include "sim/_SimParams.mock.comp.glsl"
// #pragma include "sim/_SimParams.comp.glsl"
#pragma include "sim/_SimCommon.comp.glsl"
#pragma include "sim/_SimBuffers.comp.glsl"

shared vec4 sharedPos[THREAD_GROUP_SIZE];
shared vec4 sharedTangent[THREAD_GROUP_SIZE];
shared float sharedLength[THREAD_GROUP_SIZE];

// Generate follow (child) hair strands from guide (master) strands.
//
// One thread computes one vertex (of the guide-hair).
//
layout (local_size_x = THREAD_GROUP_SIZE) in; // [numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main() {
  uint globalStrandIndex, localStrandIndex, globalVertexIndex, localVertexIndex, numVerticesInTheStrand, indexForSharedMem, strandType;
  CalcIndicesInVertexLevelMaster(gl_LocalInvocationIndex, gl_WorkGroupID.x, globalStrandIndex, localStrandIndex, globalVertexIndex, localVertexIndex, numVerticesInTheStrand, indexForSharedMem, strandType);

  // copy before we modify it
  sharedPos[indexForSharedMem] = g_HairVertexPositions_[globalVertexIndex];
  sharedTangent[indexForSharedMem] = g_HairVertexTangents_[globalVertexIndex];
  groupMemoryBarrier(); // GroupMemoryBarrierWithGroupSync();

  for (uint i = 0; i < g_NumFollowHairsPerGuideHair; i++) {
      uint globalFollowVertexIndex = globalVertexIndex + numVerticesInTheStrand * (i + 1);
	    uint globalFollowStrandIndex = globalStrandIndex + i + 1;

      // we can spread follow-hairs more using g_TipSeparationFactor simulation param
      float lenFromRoot = float(localVertexIndex) / float(numVerticesInTheStrand); // in [0,1]
      float factor = g_TipSeparationFactor * lenFromRoot + 1.0f;
      // delta from guide-hair
      vec3 offset = factor * g_FollowHairRootOffset_[globalFollowStrandIndex].xyz;
	    vec3 followPos = sharedPos[indexForSharedMem].xyz + offset;

      // write back
      g_HairVertexPositions_[globalFollowVertexIndex].xyz = followPos;
      g_HairVertexTangents_[globalFollowVertexIndex] = sharedTangent[indexForSharedMem];
  }

  return;
}
