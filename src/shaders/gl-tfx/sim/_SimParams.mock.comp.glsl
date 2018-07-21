// START _SimParams.mock.glsl

// lull
const vec4 g_Wind = vec4(0,0,0,0);
const vec4 g_Wind1 = vec4(0,0,0,0);
const vec4 g_Wind2 = vec4(0,0,0,0);
const vec4 g_Wind3 = vec4(0,0,0,0);

// @see libs\amd_tressfx\include\TressFXSettings.h
// @see tressfx-reference\TressFX-master\amd_tressfx_sample\bin\Scripts\GUI.lua

// TressFXAsset RatBoy_body.mohawk{
  // m_numTotalStrands: 6720
  // m_numTotalVertices: 215040
  // m_numVerticesPerStrand: 32
  // m_numGuideStrands: 2240
  // m_numGuideVertices: 71680
  // m_numFollowStrandsPerGuide: 2
  // has_skeleton: False
// };

#define g_NumOfStrandsPerThreadGroup      (2) //g_Counts.x := THREAD_GROUP_SIZE(64) / VERTEX_PER_STRAND(32) in kernel/vertex (should not be used in kernel/strand, cause bs)
#define g_NumFollowHairsPerGuideHair      (2) //g_Counts.y
// #define g_NumVerticesPerStrand            32 //g_Counts.z
// #define g_NumLocalShapeMatchingIterations 1 //g_SimInts.y
#define g_GravityMagnitude                (0.0)
#define g_TimeStep                        (1.0 / 60.0)
#define g_TipSeparationFactor             (0.0) // g_GravTimeTip.z

uint GetStrandType(uint globalThreadIndex) { return 0; }
float GetDamping(uint strandType) { return 1.0f; } // g_Shape.x
float GetLocalStiffness(uint strandType) { return 1.0; } // g_Shape.y; }
float GetGlobalStiffness(uint strandType) { return 0.0; } // g_Shape.z; }
float GetGlobalRange(uint strandType) { return 0.0; } // g_Shape.w; } // globalShapeMatchingEffectiveRange
// shock propagation weight, 0-none, 1-purely shock propagation
float GetVelocityShockPropogation() { return 0; } // g_VSP.x; }
// purely shock propagation driven threshold
float GetVSPAccelThreshold() { return 0; } // g_VSP.y; }
// int GetLocalConstraintIterations() { return int(g_SimInts.y); }
int GetLengthConstraintIterations() { return 0; } // int(g_SimInts.x); }

// END _SimParams.mock.glsl
