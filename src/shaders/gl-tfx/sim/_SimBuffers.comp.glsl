// START _SimBuffers.glsl

// struct Transforms {
  // /*row_major*/ mat4 tfm;
  // vec4 quat;
// };

// struct BoneSkinningData {
	// vec4 boneIndex;  // x, y, z and w component are four bone indices per strand
	// vec4 boneWeight; // x, y, z and w component are four bone weights per strand
// };

//
// UAVs (read-write resources)
//

// RWStructuredBuffer<vec4> g_HairVertexPositions;
STRUCTURED_BUFFER(0, vec4, g_HairVertexPositions)
// RWStructuredBuffer<vec4> g_HairVertexPositionsPrev;
STRUCTURED_BUFFER(1, vec4, g_HairVertexPositionsPrev)
// RWStructuredBuffer<vec4> g_HairVertexPositionsPrevPrev;
STRUCTURED_BUFFER(2, vec4, g_HairVertexPositionsPrevPrev)
// RWStructuredBuffer<vec4> g_HairVertexTangents;
STRUCTURED_BUFFER(3, vec4, g_HairVertexTangents)

// #if USE_MESH_BASED_HAIR_TRANSFORM == 1
  // RWStructuredBuffer<Transforms>  g_Transforms;
// #endif


//
// SRVs (read resources)
//

// StructuredBuffer<vec4> g_InitialHairPositions;
STRUCTURED_BUFFER(4, vec4, g_InitialHairPositions)

// probably strand's vertex rotation, but in global space
// StructuredBuffer<vec4> g_GlobalRotations;
STRUCTURED_BUFFER(5, vec4, g_GlobalRotations)

// lengths between vertices in strand
// StructuredBuffer<float> g_HairRestLengthSRV;
STRUCTURED_BUFFER(6, float, g_HairRestLengthSRV)

// used in local shape constraints, vector (vertex) -> (next_vertex)
// StructuredBuffer<vec4> g_HairRefVecsInLocalFrame;
STRUCTURED_BUFFER(7, vec4, g_HairRefVecsInLocalFrame)

// each follow hair is <offset> from guide/master root
// StructuredBuffer<vec4> g_FollowHairRootOffset;
STRUCTURED_BUFFER(8, vec4, g_FollowHairRootOffset)

// StructuredBuffer<vec4> g_MeshVertices;
// STRUCTURED_BUFFER(9, vec4, g_MeshVertices)

// StructuredBuffer<vec4> g_TransformedVerts;
// STRUCTURED_BUFFER(10, vec4, g_TransformedVerts)

// StructuredBuffer<BoneSkinningData> g_BoneSkinningData;
// STRUCTURED_BUFFER(11, BoneSkinningData, g_BoneSkinningData)

// END _SimBuffers.glsl
