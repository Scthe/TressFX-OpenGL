// START _SimCommon.glsl

// HLSL -> GLSL notes
// (from: https://anteru.net/blog/2016/mapping-between-hlsl-and-glsl/index.html)
//  * SV_DispatchThreadID gl_GlobalInvocationID
//  * SV_GroupID gl_WorkGroupID
//  * SV_GroupIndex gl_LocalInvocationIndex


// If you change the value below, you must change it in TressFXAsset.h as well.
#define THREAD_GROUP_SIZE 64

// ?Old skinning method?
#define USE_MESH_BASED_HAIR_TRANSFORM 0

// Toggle capsule collisions
#ifndef TRESSFX_COLLISION_CAPSULES
#define TRESSFX_COLLISION_CAPSULES 0
#endif

struct PerVertexData {
  uint localId; // [0-64] unique indexForSharedMem
  uint strandId; // {0,1}, localStrandIndex (each workgroup operates on 2 strands)
  uint strandId_global; // globalStrandIndex
  uint vertexId; // [0-32], localVertexIndex
  uint vertexId_global; // globalVertexIndex
  uint strandType; // const 0
};

PerVertexData GetPerVertexData(uint local_id, uint group_id, inout uint numVerticesInTheStrand) {
  numVerticesInTheStrand = (THREAD_GROUP_SIZE / g_NumOfStrandsPerThreadGroup);

  PerVertexData d;
  d.localId = local_id;
  d.strandId = local_id % g_NumOfStrandsPerThreadGroup;
  d.strandId_global = group_id * g_NumOfStrandsPerThreadGroup + d.strandId;
  d.strandId_global *= (g_NumFollowHairsPerGuideHair+1);
  d.vertexId = (local_id - d.strandId) / g_NumOfStrandsPerThreadGroup;
  d.vertexId_global = d.strandId_global * numVerticesInTheStrand + d.vertexId;
  d.strandType = GetStrandType(d.strandId_global);
  return d;
}

bool IsMovable(vec4 particle) {
  if ( particle.w > 0 ){ return true;}
  return false;
}

void CalcIndicesInStrandLevelMaster(
  uint local_id, uint group_id,
  inout uint globalStrandIndex, inout uint numVerticesInTheStrand,
  inout uint globalRootVertexIndex,
  inout uint strandType)
{
  globalStrandIndex = THREAD_GROUP_SIZE*group_id + local_id;
  globalStrandIndex *= (g_NumFollowHairsPerGuideHair+1);
  numVerticesInTheStrand = (THREAD_GROUP_SIZE / g_NumOfStrandsPerThreadGroup);
  strandType = GetStrandType(globalStrandIndex);
  globalRootVertexIndex = globalStrandIndex * numVerticesInTheStrand;
}

void GroupMemoryBarrierWithGroupSync () {
  memoryBarrierShared();
  barrier();
}

// END _SimCommon.glsl
