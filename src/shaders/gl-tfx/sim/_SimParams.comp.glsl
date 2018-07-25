// START _SimParams.glsl

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


uniform vec4 g_Wind ; //= vec4(0,0,0,0); //vec4(150,0,0,0);
uniform vec4 g_Wind1; //= vec4(0,0,0,0);
uniform vec4 g_Wind2; //= vec4(0,0,0,0);
uniform vec4 g_Wind3; //= vec4(0,0,0,0);

uniform vec4 g_Shape; // damping, local stiffness, global stiffness, global range.
uniform vec4 g_GravTimeTip; // gravity maginitude (assumed to be in negative y direction.)
uniform ivec4 g_SimInts; // Length iterations, local iterations, collision flag.
uniform ivec4 g_Counts; // num strands per thread group, num follow hairs per guid hair, num verts per strand.
// uniform vec4 g_VSP; // VSP parmeters

#define g_NumOfStrandsPerThreadGroup      (g_Counts.x) // (2) THREAD_GROUP_SIZE(64) / VERTEX_PER_STRAND(32) in kernel/vertex (should not be used in kernel/strand, cause bs)
#define g_NumFollowHairsPerGuideHair      (g_Counts.y) // (2)
// #define g_NumVerticesPerStrand            (g_Counts.z) // 32
// #define g_NumLocalShapeMatchingIterations 1 //g_SimInts.y
#define g_GravityMagnitude                (g_GravTimeTip.x) //(200.0)
#define g_TimeStep                        (g_GravTimeTip.y) //(1.0 / 60.0)
#define g_TipSeparationFactor             (g_GravTimeTip.z) //(0.0)

const float LENGTH_STIFFNESS = 1.0; // TODO make uniform?

// unused
uint GetStrandType(uint globalThreadIndex) { return 0; }

// Used durring Verlet integration. Can be though of as inertia.
// We calculate `delta = position_now - position_prev` and then
// multiply by damping:
//   * if damping == 1, then delta affects outcome (seemingly another acceleration)
//   * if damping == 0, then delta is nullified and verlet only calculates
//       basing on forces/gravity
float GetDamping(uint strandType) {
  return g_Shape.x; // 1.0f;
}


//
// Global Shape Constraints (GSC)

// If the forces/gravity are not strong enough to overcome
// this, the strands will not move
float GetGlobalStiffness(uint strandType) {
  return g_Shape.z; //0.05;
}

// By default, Global Shape Constraints affect only `global_range * vertices_in_strand`
// vertices:
//   * globalRange == 1.0, then strand tips will be affected
//       by GSC, which wolud negate forces/gravity etc.
//   * globalRange == 0.0, then whole strand is affected by forces/gravity
//   * globalRange == 0.5, only half of strand (the one closer to root)
//       is affected by forces/gravity
// Also known as 'globalShapeMatchingEffectiveRange'
float GetGlobalRange(uint strandType) {
  return g_Shape.w; // 0.3;
}


//
// Velocity Shock Propogation (VSP)

// shock propagation weight:
//   * value == 0, then no VSP
//   * value == 1, then purely driven by shock propagation
float GetVelocityShockPropogation() { return 0; } // g_VSP.x; }

// purely shock propagation driven threshold
float GetVSPAccelThreshold() { return 0; } // g_VSP.y; }


//
// Local Shape Constraints

// * stiffness == 0, then no local shape preservation
// * stiffness == 1, then ignore forces/gravity VSP etc.
// NOTE: value gets halfed in shader code, so when You set it to 1,
//       it will actually still be 0.5. No, setting the value to
//       2 will not help either (taken min(stiffness, 0.95))
float GetLocalStiffness(uint strandType) {
  return g_Shape.y; // 0.9;
}




int GetLengthConstraintIterations() {
  return int(g_SimInts.x); //1;
}

// int GetLocalConstraintIterations() { return int(g_SimInts.y); }

// END _SimParams.glsl
