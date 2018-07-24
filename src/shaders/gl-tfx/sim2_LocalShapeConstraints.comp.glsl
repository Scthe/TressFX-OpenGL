#version 450

#pragma include "_utils.glsl"
#pragma include "sim/_SimParams.mock.comp.glsl"
// #pragma include "sim/_SimParams.comp.glsl"
#pragma include "sim/_SimCommon.comp.glsl"
#pragma include "sim/_SimBuffers.comp.glsl"
#pragma include "sim/_SimQuat.comp.glsl"

// Compute shader to maintain the local shape constraints.
// for each vertex in strand (excluding root vertex):
//   1) get initial (frame 0) vector: (vertex -> next_vertex) from g_HairRefVecsInLocalFrame_
//   2) calculate where, according to this vector, would next_vertex lie
//   3) compare this with current next_vertex position after gravity, shock propagation etc.
//   4) adjust g_HairVertexPositions_[i], g_HairVertexPositions_[i+1] based on
//      local shape constraint param from simulation params
//
// One thread computes one strand.
//
layout (local_size_x = THREAD_GROUP_SIZE) in; // [numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main() {
  uint local_id, group_id, globalStrandIndex, numVerticesInTheStrand, globalRootVertexIndex, strandType;
  CalcIndicesInStrandLevelMaster(gl_LocalInvocationIndex, gl_WorkGroupID.x, globalStrandIndex, numVerticesInTheStrand, globalRootVertexIndex, strandType);

  // stiffness for local shape constraints
  float stiffnessForLocalShapeMatching = GetLocalStiffness(strandType);
  //1.0 for stiffness makes things unstable sometimes.
  stiffnessForLocalShapeMatching = 0.5f * min(stiffnessForLocalShapeMatching, 0.95f);

  // Local shape constraint for bending/twisting
  {
    // pos of currently processed vertex in global space, after sim0, sim1
    vec4 pos = g_HairVertexPositions_[globalRootVertexIndex + 1];
    vec4 rotGlobal = g_GlobalRotations_[globalRootVertexIndex];

    // we temporarily use g_HairVertexTangents to hold bone quaternion data
    // computed in ApplyVertexBoneSkinning (from sim0_IntegrationAnd...).
    vec4 bone_quat = g_HairVertexTangents_[globalStrandIndex];

    // iterate starting from child vertex 1 (which means not-root)
    // in strand all the way to the tip
    for ( uint i = 1; i < numVerticesInTheStrand - 1; i++ ) {
      uint globalVertexIndex = globalRootVertexIndex + i;
      // pos of next vertex in strand, unless i==tip, then garbage
      vec4 pos_plus_one = g_HairVertexPositions_[globalVertexIndex + 1];

      // Update position i and i_plus_1
      // 1) bone rotation in global space
      vec4 rotGlobalWorld = MultQuaternionAndQuaternion(bone_quat, rotGlobal);
      // 2) ? probably possition of NEXT vertex relative to CURRENT vertex
      //    (in other words: next vertex position in 'local' space)?
      vec3 orgPos_i_plus_1_InLocalFrame_i = g_HairRefVecsInLocalFrame_[globalVertexIndex + 1].xyz;
      // 3) NEXT vertex position in global space
      vec3 orgPos_i_plus_1_InGlobalFrame = MultQuaternionAndVector(rotGlobalWorld, orgPos_i_plus_1_InLocalFrame_i)
                + pos.xyz;
      // 4) delta where it would be if followed RefVecs (so very hard constraint
      //    to follow shape of the strand) and actual position after
      //    gravity, shock propagation etc.
      vec3 del = stiffnessForLocalShapeMatching * (orgPos_i_plus_1_InGlobalFrame - pos_plus_one.xyz).xyz;
      // 5) apply to go back to local shape
      if (IsMovable(pos)){ pos.xyz -= del.xyz; }
      if (IsMovable(pos_plus_one)){ pos_plus_one.xyz += del.xyz; }

      // Update local/global frames
      // 1) ? global -> local
      vec4 invRotGlobalWorld = InverseQuaternion(rotGlobalWorld);
      // 2) delta: vertex -> next_vertex (in global space)
      vec3 vec = normalize(pos_plus_one.xyz - pos.xyz);
      // 3) delta in local space
      vec3 x_i_plus_1_frame_i = normalize(MultQuaternionAndVector(invRotGlobalWorld, vec));
      // 4) create local space rotation between delta and X-axis
      vec3 e = vec3(1.0f, 0, 0);
      vec3 rotAxis = cross(e, x_i_plus_1_frame_i);
      if (length(rotAxis) > 0.001) {
        float angle_radian = acos(dot(e, x_i_plus_1_frame_i));
        rotAxis = normalize(rotAxis);
        vec4 localRot = MakeQuaternion(angle_radian, rotAxis);
        // move down the strand to next vertex with updated global rotation
        // (similar to bone child calculations)
        rotGlobal = MultQuaternionAndQuaternion(rotGlobal, localRot);
      }

      // write back
      g_HairVertexPositions_[globalVertexIndex].xyz = pos.xyz;
      g_HairVertexPositions_[globalVertexIndex + 1].xyz = pos_plus_one.xyz;

      // move forward with iteration
      pos = pos_plus_one;
    }
  }

  return;
}
