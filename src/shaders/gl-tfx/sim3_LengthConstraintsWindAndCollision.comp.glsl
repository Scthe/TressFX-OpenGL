#version 450

#pragma include "_utils.glsl"
#pragma include "sim/_SimParams.comp.glsl"
#pragma include "sim/_SimBuffers.comp.glsl"
#pragma include "sim/_SimCommon.comp.glsl"
#pragma include "sim/_SimCapsuleCollision.comp.glsl"
// #pragma include "sim/_SimQuat.comp.glsl"

vec2 ConstraintMultiplier(vec4 particle0, vec4 particle1) {
  if (IsMovable(particle0)) {
    if (IsMovable(particle1)) {return vec2(0.5, 0.5);}
    else {return vec2(1, 0);}
  } else {
    if (IsMovable(particle1)) {return vec2(0, 1);}
    else {return vec2(0, 0);}
  }
}

void ApplyDistanceConstraint(inout vec4 pos0, inout vec4 pos1, float targetDistance, float stiffness = 1.0) {
  vec3 delta = pos1.xyz - pos0.xyz;
  float distance = max(length(delta), 1e-7);
  float stretching = 1 - targetDistance / distance;
  delta = stretching * delta;
  vec2 multiplier = ConstraintMultiplier(pos0, pos1);

  pos0.xyz += multiplier[0] * delta * stiffness;
  pos1.xyz -= multiplier[1] * delta * stiffness;
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
  uint globalStrandIndex, localStrandIndex, globalVertexIndex, localVertexIndex, numVerticesInTheStrand, indexForSharedMem, strandType;
  CalcIndicesInVertexLevelMaster(gl_LocalInvocationIndex, gl_WorkGroupID.x, globalStrandIndex, localStrandIndex, globalVertexIndex, localVertexIndex, numVerticesInTheStrand, indexForSharedMem, strandType);

  uint numOfStrandsPerThreadGroup = g_NumOfStrandsPerThreadGroup;

  // Copy data into shared memory
  sharedPos[indexForSharedMem] = g_HairVertexPositions_[globalVertexIndex];
  sharedLength[indexForSharedMem] = g_HairRestLengthSRV_[globalVertexIndex];

  groupMemoryBarrier(); // GroupMemoryBarrierWithGroupSync();

  // Wind
  if (g_Wind.x != 0 || g_Wind.y != 0 || g_Wind.z != 0) {
    vec4 force = vec4(0, 0, 0, 0);
    float frame = g_Wind.w;

    // only vertices 2nd and it's children, root and 1st ignore wind
    if (localVertexIndex >= 2 && localVertexIndex < numVerticesInTheStrand - 1) {
      // combining four winds.
      float a = (float(globalStrandIndex % 20))/20.0f; // rand(0:20) / 20
      vec3  windTotal = a * g_Wind.xyz
                      + (1.0f - a) * g_Wind1.xyz
                      + a * g_Wind2.xyz
                      + (1.0f - a) * g_Wind3.xyz;

      uint sharedIndex = localVertexIndex * numOfStrandsPerThreadGroup + localStrandIndex;

      // vector to next_vertex (?), indexing seems weird, prob. due too warp memory access
      vec3 v = sharedPos[sharedIndex].xyz - sharedPos[sharedIndex + numOfStrandsPerThreadGroup].xyz;
      vec3 force = -cross(cross(v, windTotal), v);
      sharedPos[sharedIndex].xyz += force * g_TimeStep * g_TimeStep;
    }
  }
  groupMemoryBarrier(); // GroupMemoryBarrierWithGroupSync();


  // Enforce length constraints
  uint a = uint(floor(numVerticesInTheStrand / 2.0f));
  uint b = uint(floor((numVerticesInTheStrand - 1) / 2.0f));
  int nLengthContraintIterations = GetLengthConstraintIterations();
  // we re-adjust positions several times, getting more accurate results with each iter.
  // In each iter we operate on 2 distances between 3 diffrent vertices
  for (int iterationE = 0; iterationE < nLengthContraintIterations; iterationE++) {
    uint sharedIndex = 2 * localVertexIndex * numOfStrandsPerThreadGroup + localStrandIndex;

    if (localVertexIndex < a){
      ApplyDistanceConstraint(
        sharedPos[sharedIndex], // this
        sharedPos[sharedIndex + numOfStrandsPerThreadGroup], // next
        sharedLength[sharedIndex].x); // expected len. (this->next)
    }
    groupMemoryBarrier(); // GroupMemoryBarrierWithGroupSync();

    if (localVertexIndex < b){
      ApplyDistanceConstraint(
        sharedPos[sharedIndex + numOfStrandsPerThreadGroup], // next
        sharedPos[sharedIndex + 2 * numOfStrandsPerThreadGroup], // next_next
        sharedLength[sharedIndex + numOfStrandsPerThreadGroup].x); // expected len (next->next_next)
    }
    groupMemoryBarrier(); // GroupMemoryBarrierWithGroupSync();
  }


  // Collision handling with capsule objects
  vec4 oldPos = g_HairVertexPositionsPrev_[globalVertexIndex];
  bool bAnyColDetected = ResolveCapsuleCollisions(sharedPos[indexForSharedMem], oldPos);
  groupMemoryBarrier(); // GroupMemoryBarrierWithGroupSync();


  // Compute tangent
  // tangent := normalize(vertex -> next_vertex)
  vec3 tangent = sharedPos[indexForSharedMem+numOfStrandsPerThreadGroup].xyz - sharedPos[indexForSharedMem].xyz;
  g_HairVertexTangents_[globalVertexIndex].xyz = normalize(tangent);

  // update global position buffers
  g_HairVertexPositions_[globalVertexIndex] = sharedPos[indexForSharedMem];

  if (bAnyColDetected) {
    g_HairVertexPositionsPrev_[globalVertexIndex] = sharedPos[indexForSharedMem];
  }

  return;
}
