// START TressFXStrands.glsl

#define TRESSFX_FLOAT_EPSILON 1e-7

// static const bool g_bExpandPixels = false; // yep, pls just keep 0/false

// if use g_Ratio
// uniform bool g_bThinTip;
// scale down at the tip.
// e.g. 0.5 means that hair is half as thick at tip when compared to root
// uniform float g_Ratio;

uniform float g_FiberRadius;

// No colors for now
// texture2D<vec3> g_txHairColor;
// sampler g_samLinearWrap;

// Structured buffers with hair vertex info
// Accessors to allow changes to how they are accessed.
STRUCTURED_BUFFER(1, vec4 , g_GuideHairVertexPositions) //StructuredBuffer<vec4> g_GuideHairVertexPositions;
STRUCTURED_BUFFER(2, vec4 , g_GuideHairVertexTangents) //StructuredBuffer<vec4> g_GuideHairVertexTangents;
STRUCTURED_BUFFER(3, float, g_HairThicknessCoeffs) //StructuredBuffer<float> g_HairThicknessCoeffs;
STRUCTURED_BUFFER(4, vec2 , g_HairStrandTexCd) //StructuredBuffer<vec2> g_HairStrandTexCd;
vec4  GetPosition (uint index) { return g_GuideHairVertexPositions_[index]; }
vec4  GetTangent  (uint index) { return g_GuideHairVertexTangents_[index]; }
float GetThickness(uint index) { return g_HairThicknessCoeffs_[index]; }

// Controls whether you do mul(M,v) or mul(v,M) -> (row major vs column major)
vec4 MatrixMult(mat4 m, vec4 v) {
  return m * v;
}

// Safe_normalize-vec2
vec2 Safe_normalize2(vec2 vec) {
  float len = length(vec);
  return len >= TRESSFX_FLOAT_EPSILON ? normalize(vec) : vec2(0, 0);
}

// Safe_normalize-vec3
vec3 Safe_normalize3(vec3 vec) {
  float len = length(vec);
  return len >= TRESSFX_FLOAT_EPSILON ? normalize(vec) : vec3(0, 0, 0);
}

vec3 GetStrandColor(uint index) {
  // vec2 texCd = g_HairStrandTexCd_[(uint)index / (uint)g_NumVerticesPerStrand];
  // vec3 color = g_txHairColor.SampleLevel(g_samLinearWrap, texCd, 0).xyz;// * g_MatBaseColor.xyz;
  // return (color);
  return vec3(0.7);
}

struct TressFXVertex {
  vec4 Position;
  vec4 Tangent;
  vec4 p0p1;
  vec3 strandColor;
};


TressFXVertex GetExpandedTressFXVert(uint vertexId, vec3 eye, /*vec2 winSize,*/ mat4 viewProj) {
  // Access the current line segment
  // (We will move vertices left or right by hair thickness, while odd vertices are moved left, even are moved right.
  // And by 'left' and 'right' we mean according to normal&tangent.
  // And by nromal we mean hair_pos - camera_pos)
  uint index = vertexId / 2;  // vertexId is actually the indexed vertex id when indexed triangles are used

  // Get updated positions and tangents from simulation result
  vec3 v = GetPosition(index).xyz;
  vec3 t = GetTangent(index).xyz;

  // Get hair strand thickness
  // float ratio = (g_bThinTip > 0) ? g_Ratio : 1.0;
  float ratio = 1.0;

  // Calculate right and projected right vectors
  vec3 towardsCamera = Safe_normalize3(v - eye);
  vec3 right = Safe_normalize3(cross(t, towardsCamera));
  // vec2 proj_right = Safe_normalize(MatrixMult(viewProj, vec4(right, 0)).xy);

  // g_bExpandPixels should be set to 0 at minimum from the CPU side; this would avoid the below test
  // float expandPixels = (g_bExpandPixels < 0) ? 0.0 : 0.71;

  // Calculate the negative and positive offset screenspace positions
  vec4 hairEdgePositions[2]; // 0 is for odd vertexId, 1 is positive even vertexId
  vec3 thicknessVector = right * ratio * g_FiberRadius;
  hairEdgePositions[0] = vec4(v - thicknessVector, 1.0); // position 'left'
  hairEdgePositions[1] = vec4(v + thicknessVector, 1.0); // position 'right'
  hairEdgePositions[0] = MatrixMult(viewProj, hairEdgePositions[0]);
  hairEdgePositions[1] = MatrixMult(viewProj, hairEdgePositions[1]);

  // Write output data
  TressFXVertex Output;
	bool isOdd = (vertexId & 0x01) > 0; // just check where it is used..
  // float fDirIndex = (vertexId & 0x01) ? -1.0 : 1.0;
  Output.Position = (isOdd ? hairEdgePositions[0] : hairEdgePositions[1]);
                // + fDirIndex * vec4(proj_right * expandPixels / winSize.y, 0.0f, 0.0f) * ((vertexId & 0x01) ? hairEdgePositions[0].w : hairEdgePositions[1].w);
  Output.Tangent = vec4(t, ratio); // pack tangent + ThinTipRatio
  Output.p0p1 = vec4(
    hairEdgePositions[0].xy / max(hairEdgePositions[0].w, TRESSFX_FLOAT_EPSILON),
    hairEdgePositions[1].xy / max(hairEdgePositions[1].w, TRESSFX_FLOAT_EPSILON)
  );
  Output.strandColor = GetStrandColor(index);
  //Output.PosCheck = MatrixMult(g_mView, vec4(v,1));

  return Output;
}

// END TressFXStrands.glsl
