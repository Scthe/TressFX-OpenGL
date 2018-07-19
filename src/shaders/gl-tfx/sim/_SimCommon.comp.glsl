// START _SimCommon.glsl


// HLSL -> GLSL
// (from: https://anteru.net/blog/2016/mapping-between-hlsl-and-glsl/index.html)
//  * SV_DispatchThreadID gl_GlobalInvocationID
//  * SV_GroupID gl_WorkGroupID
//  * SV_GroupIndex gl_LocalInvocationIndex

void CalcIndicesInVertexLevelMaster(
  uint local_id, uint group_id,
  inout uint globalStrandIndex, inout uint localStrandIndex,
  inout uint globalVertexIndex, inout uint localVertexIndex,
  inout uint numVerticesInTheStrand,
  inout uint indexForSharedMem,
  inout uint strandType)
{
  indexForSharedMem = local_id;
  numVerticesInTheStrand = (THREAD_GROUP_SIZE / g_NumOfStrandsPerThreadGroup);

  localStrandIndex = local_id % g_NumOfStrandsPerThreadGroup;
  globalStrandIndex = group_id * g_NumOfStrandsPerThreadGroup + localStrandIndex;
  globalStrandIndex *= (g_NumFollowHairsPerGuideHair+1);
  localVertexIndex = (local_id - localStrandIndex) / g_NumOfStrandsPerThreadGroup;

  strandType = GetStrandType(globalStrandIndex);
  globalVertexIndex = globalStrandIndex * numVerticesInTheStrand + localVertexIndex;
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

// END _SimCommon.glsl
