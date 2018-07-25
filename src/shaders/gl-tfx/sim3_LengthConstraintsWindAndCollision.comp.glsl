#version 450

#pragma include "_utils.glsl"
#pragma include "sim/_SimParams.comp.glsl"
#pragma include "sim/_SimCommon.comp.glsl"
#pragma include "sim/_SimBuffers.comp.glsl"
#pragma include "sim/_SimCapsuleCollision.comp.glsl"
// #pragma include "sim/_SimQuat.comp.glsl"

// THREAD_GROUP_SIZE <- 64 (2 strands, 32 vertices each)
// shared mem indexing: v{strand}_{vertex_index}
// shared mem preview: [v0_0, v1_0, v0_1, v1_1, ... v0_31, v1_31]
shared vec4 sharedPos[THREAD_GROUP_SIZE];
shared float sharedLength[THREAD_GROUP_SIZE];

vec2 ConstraintMultiplier(vec4 particle0, vec4 particle1) {
  bool can_move0 = IsMovable(particle0);
  bool can_move1 = IsMovable(particle1);
  if ( can_move0 &&  can_move1) { return vec2(0.5, 0.5); } // move equally
  if ( can_move0 && !can_move1) { return vec2(1.0, 0.0); } // move only 1st
  if (!can_move0 &&  can_move1) { return vec2(0.0, 1.0); } // move only 2nd
  return vec2(0, 0); // can't move either
}

void ApplyDistanceConstraint(uint idx0, uint idx1, float targetDistance) {
  vec4 pos0 = sharedPos[idx0];
  vec4 pos1 = sharedPos[idx1];

  vec3 delta = pos1.xyz - pos0.xyz;
  float distance = max(length(delta), 1e-7);
  // targetDistance / distance:
  //  * > 1 if real distance if TOO SHORT and we need to ELONGATE
  //  * < 1 if real distance is BIGGER than expected and we need to SHORTEN
  //  stretching:
  //  * < 0 if we need to ELONGATE
  //  * > 0 if we need to SHORTEN
  float stretching = 1 - targetDistance / distance;
  // float stretching = 0.0005;
  delta = stretching * delta;

  // how much we scale movement of each vertex
  vec2 multiplier = ConstraintMultiplier(pos0, pos1);

  sharedPos[idx0].xyz += multiplier[0] * delta * LENGTH_STIFFNESS;
  sharedPos[idx1].xyz -= multiplier[1] * delta * LENGTH_STIFFNESS;
}

//
// 1) wind
// 2) length constraints
// 3) capsule collisions
// 4) update tangents
// 5) write back to g_HairVertexPositions_
//
// One thread computes one vertex.
//
layout (local_size_x = THREAD_GROUP_SIZE) in; // [numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main() {
  const uint numOfStrandsPerThreadGroup = g_NumOfStrandsPerThreadGroup;
  uint numVerticesInTheStrand; // 32
  const PerVertexData vertData = GetPerVertexData(gl_LocalInvocationIndex, gl_WorkGroupID.x, numVerticesInTheStrand);

  #define get_shared_index(VERTEX_ID) ((VERTEX_ID) * numOfStrandsPerThreadGroup + vertData.strandId)

  // Copy data into shared memory
  sharedPos[vertData.localId] = g_HairVertexPositions_[vertData.vertexId_global];
  sharedLength[vertData.localId] = g_HairRestLengthSRV_[vertData.vertexId_global];
  GroupMemoryBarrierWithGroupSync();


  // Wind
  if (g_Wind.x != 0 || g_Wind.y != 0 || g_Wind.z != 0) { // compare on floats?!
    // only vertices 2nd and it's children, root and 1st ignore wind
    if (vertData.vertexId >= 2 && vertData.vertexId < numVerticesInTheStrand - 1) {
      // combining four winds.
      float w1 = (float(vertData.strandId_global % 20))/20.0f; // rand(0:20) / 20
      float w2 = 1.0f - w1;
      vec3  windTotal = w1 * g_Wind.xyz
                      + w2 * g_Wind1.xyz
                      + w1 * g_Wind2.xyz
                      + w2 * g_Wind3.xyz;

      uint sharedIndex      = get_shared_index(vertData.vertexId    );
      uint sharedIndex_next = get_shared_index(vertData.vertexId + 1);
      // vector(next_vertex -> this_vertex), NOT NORMALIZED
      vec3 from_next_vert = sharedPos[sharedIndex].xyz - sharedPos[sharedIndex_next].xyz;
      // NOTE: had some weird stability issues when did cross on unnormalized
      // vectors. Not sure wtf, but ain't gonna bother investigating
      float force_mul = length(from_next_vert) * length(windTotal);
      from_next_vert = normalize(from_next_vert);
      windTotal = normalize(windTotal);
      vec3 force = cross(cross(from_next_vert, windTotal), from_next_vert);
      force *= force_mul;
      // original code:
      // vec3 v = sharedPos[sharedIndex].xyz - sharedPos[sharedIndex + numOfStrandsPerThreadGroup].xyz;
      // vec3 force = -cross(cross(v, windTotal), v);

      // apply wind
      sharedPos[sharedIndex].xyz += force * g_TimeStep * g_TimeStep;
    }
  }
  GroupMemoryBarrierWithGroupSync();


  // Enforce length constraints
  uint a = uint(floor(float(numVerticesInTheStrand) / 2.0f));
  uint b = uint(floor(float(numVerticesInTheStrand - 1) / 2.0f));
  int nLengthContraintIterations = GetLengthConstraintIterations();
  // we re-adjust positions several times, getting more accurate results with each iter.
  // In each iter. we operate on 2 distances between 3 diffrent consecutive vertices
  for (int jitteration = 0; jitteration < nLengthContraintIterations; jitteration++) {
    uint sharedIndex    = get_shared_index(2*vertData.vertexId    );
    uint sharedIndex_n1 = get_shared_index(2*vertData.vertexId + 1); // next vertex
    uint sharedIndex_n2 = get_shared_index(2*vertData.vertexId + 2); // next next vertex

    if (vertData.vertexId < a) {
      float expected_len = sharedLength[sharedIndex].x;
      ApplyDistanceConstraint(sharedIndex, sharedIndex_n1, expected_len);
    }
    GroupMemoryBarrierWithGroupSync();

    if (vertData.vertexId < b) {
      float expected_len = sharedLength[sharedIndex_n1].x;
      ApplyDistanceConstraint(sharedIndex_n1, sharedIndex_n2, expected_len);
    }
    GroupMemoryBarrierWithGroupSync();
  }


  // Collision handling with capsule objects
  // vec4 oldPos = g_HairVertexPositionsPrev_[vertData.vertexId_global];
  // bool bAnyColDetected = ResolveCapsuleCollisions(sharedPos[vertData.localId], oldPos, 0.4);
  // GroupMemoryBarrierWithGroupSync();
  bool bAnyColDetected = false; // just to remember to uncomment when testing col. capsules


  // Compute tangent
  // tangent := normalize(vertex -> next_vertex)
  vec3 tangent = sharedPos[vertData.localId + numOfStrandsPerThreadGroup].xyz
               - sharedPos[vertData.localId].xyz;
  g_HairVertexTangents_[vertData.vertexId_global].xyz = normalize(tangent);

  // update global position buffers
  g_HairVertexPositions_[vertData.vertexId_global] = sharedPos[vertData.localId];

  if (bAnyColDetected) {
    g_HairVertexPositionsPrev_[vertData.vertexId_global] = sharedPos[vertData.localId];
  }

  return;
}
