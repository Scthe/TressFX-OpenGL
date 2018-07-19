// START _SimParams.glsl

// If you change the value below, you must change it in TressFXAsset.h as well.
#ifndef THREAD_GROUP_SIZE
#define THREAD_GROUP_SIZE 64
#endif

shared vec4 sharedPos[THREAD_GROUP_SIZE];
shared vec4 sharedTangent[THREAD_GROUP_SIZE];
shared float sharedLength[THREAD_GROUP_SIZE];

// ?Old skinning method?
#define USE_MESH_BASED_HAIR_TRANSFORM 0

// Toggle capsule collisions
#define TRESSFX_COLLISION_CAPSULES 0

//constants that change frame to frame
// cbuffer tressfxSimParameters {//: register( b0 )
  uniform vec4 g_Wind;
  uniform vec4 g_Wind1;
  uniform vec4 g_Wind2;
  uniform vec4 g_Wind3;

  uniform vec4 g_Shape; // damping, local stiffness, global stiffness, global range.
  uniform vec4 g_GravTimeTip; // gravity maginitude (assumed to be in negative y direction.)
  uniform ivec4 g_SimInts; // Length iterations, local iterations, collision flag.
  uniform ivec4 g_Counts; // num strands per thread group, num follow hairs per guid hair, num verts per strand.
  uniform vec4 g_VSP; // VSP parmeters

  // uniform /*row_major*/ mat4 g_BoneSkinningMatrix[AMD_TRESSFX_MAX_NUM_BONES];

#if TRESSFX_COLLISION_CAPSULES
  uniform vec4 g_centerAndRadius0[TRESSFX_MAX_NUM_COLLISION_CAPSULES];
  uniform vec4 g_centerAndRadius1[TRESSFX_MAX_NUM_COLLISION_CAPSULES];
  uniform ivec4 g_numCollisionCapsules;
#endif
// }

#define g_NumOfStrandsPerThreadGroup      g_Counts.x
#define g_NumFollowHairsPerGuideHair      g_Counts.y
#define g_NumVerticesPerStrand            g_Counts.z

#define g_NumLocalShapeMatchingIterations g_SimInts.y

#define g_GravityMagnitude                g_GravTimeTip.x
#define g_TimeStep                        g_GravTimeTip.y
#define g_TipSeparationFactor             g_GravTimeTip.z

// We no longer support groups (indirection).
uint GetStrandType(uint globalThreadIndex) {
  return 0;
}

float GetDamping(uint strandType) {
  // strand type unused.
  // In the future, we may create an array and use indirection.
  return g_Shape.x;
}

float GetLocalStiffness(uint strandType) {
  // strand type unused.
  // In the future, we may create an array and use indirection.
  return g_Shape.y;
}

float GetGlobalStiffness(uint strandType) {
  // strand type unused.
  // In the future, we may create an array and use indirection.
  return g_Shape.z;
}

float GetGlobalRange(uint strandType) {
  // strand type unused.
  // In the future, we may create an array and use indirection.
  return g_Shape.w;
}

// shock propagation weight, 0-none, 1-purely shock propagation
float GetVelocityShockPropogation() {
  return g_VSP.x;
}

// purely shock propagation driven threshold
float GetVSPAccelThreshold() {
  return g_VSP.y;
}

int GetLocalConstraintIterations() {
  return int(g_SimInts.y);
}

int GetLengthConstraintIterations() {
  return int(g_SimInts.x);
}

// END _SimParams.glsl
