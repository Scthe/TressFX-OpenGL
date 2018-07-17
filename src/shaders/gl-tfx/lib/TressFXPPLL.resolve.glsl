// START TressFXPPLL.resolve.glsl

#ifndef KBUFFER_SIZE
#define KBUFFER_SIZE 8
#endif


#ifndef TAIL_SHADING
  vec4 TressFXTailColor(vec2 pixelCoord, float depth, vec4 vTangentCoverage, vec3 baseColor) {
    return vec4(baseColor, vTangentCoverage.w);
  }
#define TAIL_SHADING TressFXTailColor
#endif

#ifndef HEAD_SHADING
  vec4 TressFXHeadColor(vec2 pixelCoord, float depth, vec4 vTangentCoverage, vec3 baseColor) {
    return vec4(baseColor, vTangentCoverage.w);
  }
#define HEAD_SHADING TressFXHeadColor
#endif


#define MAX_FRAGMENTS 512
#define TAIL_COMPRESS 0


#define NODE_DATA(x)  (LinkedListSRV_[x].data)
#define NODE_NEXT(x)  (LinkedListSRV_[x].uNext)
#define NODE_DEPTH(x) (LinkedListSRV_[x].depth)
#define NODE_COLOR(x) (LinkedListSRV_[x].color)

// 2D texture to store head pointers
// Texture2D<int>    tFragmentListHead;
layout(binding = 0, r32ui)
uniform uimage2D tFragmentListHead;

// SSBO to store nodes
// StructuredBuffer<PPLL_STRUCT> LinkedListSRV;
STRUCTURED_BUFFER(1, PPLL_STRUCT , LinkedListSRV)


vec4 UnpackUintIntoFloat4(uint uValue) {
  uvec4 v = uvec4(
    (uValue & 0xFF000000) >> 24,
    (uValue & 0x00FF0000) >> 16,
    (uValue & 0x0000FF00) >> 8,
    (uValue & 0x000000FF)
  );
  return vec4(v) / 255.0;
  // return vec4(
    // ((uValue & 0xFF000000) >> 24) / 255.0,
    // ((uValue & 0x00FF0000) >> 16) / 255.0,
    // ((uValue & 0x0000FF00) >> 8) / 255.0,
    // ((uValue & 0x000000FF)) / 255.0);
}

float mad1(float m, float a, float b) { return m * a + b; }
vec3  mad3(vec3 m,  float a, vec3 b ) { return m * a + b; }

// local tmp buffer
#define GET_DEPTH_AT_INDEX(uIndex) kBuffer[(uIndex)].x
#define GET_DATA_AT_INDEX(uIndex)  kBuffer[(uIndex)].y
#define GET_COLOR_AT_INDEX(uIndex) kBuffer[(uIndex)].z
#define STORE_DEPTH_AT_INDEX(uIndex, uValue) kBuffer[(uIndex)].x = (uValue)
#define STORE_DATA_AT_INDEX(uIndex, uValue)  kBuffer[(uIndex)].y = (uValue)
#define STORE_COLOR_AT_INDEX(uIndex, uValue) kBuffer[(uIndex)].z = (uValue)
#define KBUFFER_TYPE uvec4


vec4 GatherLinkedList(vec2 vfScreenAddress) {
  // uvec2 vScreenAddress = uvec2(vfScreenAddress);
  uint pointer = imageLoad(tFragmentListHead, ivec2(vfScreenAddress)).r; //tFragmentListHead[vScreenAddress]; or ivec2(gl_FragCoord.xy ?
  vec4 outColor = vec4(0, 0, 0, 1); // final color, defined here cause?

  if (pointer == FRAGMENT_LIST_NULL) {
    discard;
  }
  // ASSERT(pointer >= 0 && pointer < FRAGMENT_LIST_NULL)


  // create kBuffer to hold intermed. node values, zeroe it
  KBUFFER_TYPE kBuffer[KBUFFER_SIZE];
  for (int t = 0; t < KBUFFER_SIZE; ++t) {
    STORE_DEPTH_AT_INDEX(t, uint(100000.0));
    STORE_DATA_AT_INDEX(t, 0);
  }

  // Get first K elements.
  for (int p = 0; p < KBUFFER_SIZE; ++p) {
    if (pointer != FRAGMENT_LIST_NULL) {
      STORE_DEPTH_AT_INDEX(p, NODE_DEPTH(pointer));
      STORE_DATA_AT_INDEX(p, NODE_DATA(pointer));
      STORE_COLOR_AT_INDEX(p, NODE_COLOR(pointer));
      pointer = NODE_NEXT(pointer);
    }
  }

  vec4 fcolor = vec4(0, 0, 0, 1); // final fragment color, assigned to outColor later on

  // If the node in the linked list is nearer than the furthest one in the local array, exchange the node
  // in the local array for the one in the linked list.
  for (int iFragment = 0; iFragment < MAX_FRAGMENTS && pointer != FRAGMENT_LIST_NULL; ++iFragment) {
    if (pointer == FRAGMENT_LIST_NULL) break;

    // find the furthest node in array
    int id = 0; // id of node to be exchanged (one with max_depth)
    float max_depth = 0;
    for (int i = 0; i < KBUFFER_SIZE; i++) {
      float fDepth = float(GET_DEPTH_AT_INDEX(i));
      if (max_depth < fDepth) {
        max_depth = fDepth;
        id = i;
      }
    }

    // we fetch here to not cause warp split. Though it would be fetched anyway,
    // just You know, do not put more code in conditionals
    uint data = NODE_DATA(pointer);
    uint color = NODE_COLOR(pointer);
    uint nodeDepth = NODE_DEPTH(pointer);
    float fNodeDepth = float(nodeDepth);

    // do the exchange IF pointer is closer then one of members of KBUFFER
    if (max_depth > fNodeDepth) {
      uint tmp = GET_DEPTH_AT_INDEX(id);
      STORE_DEPTH_AT_INDEX(id, nodeDepth);
      fNodeDepth = float(tmp);

      tmp = GET_DATA_AT_INDEX(id);
      STORE_DATA_AT_INDEX(id, data);
      data = tmp;

      tmp = GET_COLOR_AT_INDEX(id);
      STORE_COLOR_AT_INDEX(id, color);
      color = tmp;
    }

    vec4 vData = UnpackUintIntoFloat4(data);
#if TAIL_COMPRESS
    vec4 vColor = UnpackUintIntoFloat4(color);
    fcolor.w = mad1(-fcolor.w, vColor.w, fcolor.w);
#else
    vec4 vColor = UnpackUintIntoFloat4(color);
    vec4 fragmentColor = TAIL_SHADING(vfScreenAddress, fNodeDepth, vData, vColor.rgb);
    //fragmentColor = vec4( max(float(iFragment)/255.0,255.0)/255.0, iFragment <= 255 ? float(iFragment%255) : 0, 0, 1);
    fcolor.xyz = mad3(-fcolor.xyz, fragmentColor.w, fcolor.xyz) + fragmentColor.xyz * fragmentColor.w;
    fcolor.w = mad1(-fcolor.w, fragmentColor.w, fcolor.w);
#endif

    pointer = NODE_NEXT(pointer);
  } // end of for loop


#if TAIL_COMPRESS
  float fTailAlphaInv = fcolor.w;
  fcolor.xyzw = vec4(0, 0, 0, 1);
#endif

  // Blend the k nearest layers of fragments from back to front, where k = MAX_TOP_LAYERS_EYE
  for (int j = 0; j < KBUFFER_SIZE; j++) {
    // find the furthest node in the array
    int id = 0; // id of node to be applied (one with max_depth)
    float max_depth = 0;
    for (int i = 0; i < KBUFFER_SIZE; i++) {
      float fDepth = float(GET_DEPTH_AT_INDEX(i));
      if (max_depth < fDepth) {
        max_depth = fDepth;
        id = i;
      }
    }

    // read node to be applied
    uint nodeDepth = GET_DEPTH_AT_INDEX(id);
    uint data = GET_DATA_AT_INDEX(id);
    uint color = GET_COLOR_AT_INDEX(id);

    // take this node out of the next search
    STORE_DEPTH_AT_INDEX(id, 0);

    // Use high quality shading for the nearest k fragments
    float fDepth = float(nodeDepth);
    vec4 vData = UnpackUintIntoFloat4(data);
    vec4 vColor = UnpackUintIntoFloat4(color);
    vec4 fragmentColor = HEAD_SHADING(vfScreenAddress, fDepth, vData, vColor.rgb);
#if TAIL_COMPRESS
    fragmentColor.w = 1 - (1 - fragmentColor.w)*fTailAlphaInv;
    //fTailAlphaInv = 1;
#endif
    // Blend the fragment color
    fcolor.xyz = mad3(-fcolor.xyz, fragmentColor.w, fcolor.xyz) + fragmentColor.xyz * fragmentColor.w;
    fcolor.w = fcolor.w* (1 - fragmentColor.w);//mad1(-fcolor.w, fragmentColor.w, fcolor.w);
  }

  outColor = fcolor;
  return outColor;
}


// END TressFXPPLL.resolve.glsl
