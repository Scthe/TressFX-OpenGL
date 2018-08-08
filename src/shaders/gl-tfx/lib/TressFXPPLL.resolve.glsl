// START TressFXPPLL.resolve.glsl

#define KBUFFER_SIZE 8
#define MAX_FRAGMENTS 512

// simplified shading for strands that are not in KBUFFER_SIZE of closest strands
#ifndef TAIL_COMPRESS
#define TAIL_COMPRESS 0
// #define TAIL_COMPRESS 1
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

#define DEPTH_IS_FURTHER(LAST_USED_DEPTH, DEPTH_TO_TEST) ((LAST_USED_DEPTH) < (DEPTH_TO_TEST))
#define DEPTH_RESET_TO_CLOSE (0)


// 2D texture to store head pointers
// Texture2D<int>    tFragmentListHead;
layout(location=0, r32ui)
uniform uimage2D tFragmentListHead;

// SSBO to store nodes
// StructuredBuffer<PPLL_STRUCT> LinkedListSRV;
STRUCTURED_BUFFER(1, PPLL_STRUCT, LinkedListSRV)
#define NODE_DATA(x)  (LinkedListSRV_[x].data)
#define NODE_NEXT(x)  (LinkedListSRV_[x].uNext)
#define NODE_DEPTH(x) (LinkedListSRV_[x].depth) // was multiplied by 255 in build stage
#define NODE_COLOR(x) (LinkedListSRV_[x].color)


// kBuffer - local tmp buffer for first K values from PPLL
#define KBUFFER_TYPE uvec4
#define GET_KBUFFER_DEPTH(uIndex) kBuffer[(uIndex)].x
#define GET_KBUFFER_DATA(uIndex)  kBuffer[(uIndex)].y
#define GET_KBUFFER_COLOR(uIndex) kBuffer[(uIndex)].z
#define STORE_KBUFFER_DEPTH(uIndex, uValue) kBuffer[(uIndex)].x = (uValue)
#define STORE_KBUFFER_DATA(uIndex, uValue)  kBuffer[(uIndex)].y = (uValue)
#define STORE_KBUFFER_COLOR(uIndex, uValue) kBuffer[(uIndex)].z = (uValue)

void ClearKBuffer(inout KBUFFER_TYPE kBuffer[KBUFFER_SIZE]) {
  // [unroll]
  for (int t = 0; t < KBUFFER_SIZE; ++t) {
    STORE_KBUFFER_DEPTH(t, uint(100000.0));
    STORE_KBUFFER_DATA(t, 0);
  }
}

uint FillFirstKBuffferElements (inout KBUFFER_TYPE kBuffer[KBUFFER_SIZE], uint pointer) {
  for (int p = 0; p < KBUFFER_SIZE; ++p) {
    if (pointer != FRAGMENT_LIST_NULL) {
      STORE_KBUFFER_DEPTH(p, NODE_DEPTH(pointer));
      STORE_KBUFFER_DATA(p, NODE_DATA(pointer));
      STORE_KBUFFER_COLOR(p, NODE_COLOR(pointer));
      pointer = NODE_NEXT(pointer);
    }
  }
  return pointer;
}

int FindFurthestKBufferEl (const KBUFFER_TYPE kBuffer[KBUFFER_SIZE], inout float max_depth) {
  int id = 0;
  for (int i = 0; i < KBUFFER_SIZE; i++) {
    float fDepth = float(GET_KBUFFER_DEPTH(i));
    if (DEPTH_IS_FURTHER(max_depth, fDepth)) { // max_depth < fDepth
      max_depth = fDepth;
      id = i;
    }
  }
  return id;
}

void TailShading (vec2 vfScreenAddress, float fNodeDepth, vec4 vData, vec4 vColor, inout vec4 fcolor) {
  #if TAIL_COMPRESS
    fcolor.a = mix(fcolor.a, 0.0, vColor.a);
  #else
    vec4 fragmentColor = TAIL_SHADING(vfScreenAddress, fNodeDepth, vData, vColor.rgb);
    fcolor = mix(fcolor, vec4(0), fragmentColor.a); // previous color contrib
    fcolor.rgb += fragmentColor.rgb * fragmentColor.a; // add current color
  #endif
}

vec4 GatherLinkedList(vec2 vfScreenAddress) {
  uint pointer = imageLoad(tFragmentListHead, ivec2(vfScreenAddress)).r;
  if (pointer == FRAGMENT_LIST_NULL) {
    discard;
  }

  // create kBuffer to hold intermediary values. We are going to fill it with
  // KBUFFER_SIZE of PPLL_STRUCTs that are closest to the camera. The closest
  // linked list elements have special treatment in blending
  KBUFFER_TYPE kBuffer[KBUFFER_SIZE];
  ClearKBuffer(kBuffer); // actually makes me wonder how providing arrays to a function works
  pointer = FillFirstKBuffferElements(kBuffer, pointer);

  vec4 fcolor = vec4(0, 0, 0, 1); // final fragment color

  // TAIL := all vertices that are not in kBuffer
  // If the node in the linked list is nearer than the furthest one in the local array,
  // exchange the node in the local array for the one in the linked list.
  for (int iFragment = 0; iFragment < MAX_FRAGMENTS; ++iFragment) {
    if (pointer == FRAGMENT_LIST_NULL) break;

    // find id of node to be exchanged (one with kbufferFurthestDepth)
    float kbufferFurthestDepth = 0;
    int id = FindFurthestKBufferEl(kBuffer, kbufferFurthestDepth);

    // fetch data for this iteration of linked list elements
    uint data = NODE_DATA(pointer);
    uint color = NODE_COLOR(pointer);
    uint nodeDepth = NODE_DEPTH(pointer);
    float fNodeDepth = float(nodeDepth);

    // kBuffer collects linked list elements closest to the eye. If element
    // under pointer is closer then furthest kBuffer element, then exchange
    if (DEPTH_IS_FURTHER(fNodeDepth, kbufferFurthestDepth)) { // kbufferFurthestDepth > fNodeDepth
      uint tmp = GET_KBUFFER_DEPTH(id);
      STORE_KBUFFER_DEPTH(id, nodeDepth);
      fNodeDepth = float(tmp);

      tmp = GET_KBUFFER_DATA(id);
      STORE_KBUFFER_DATA(id, data);
      data = tmp;

      tmp = GET_KBUFFER_COLOR(id);
      STORE_KBUFFER_COLOR(id, color);
      color = tmp;
    }

    // add the element to accumulating value
    // (just do the shading, ok?)
    vec4 vData = UnpackUintIntoFloat4(data);
    vec4 vColor = UnpackUintIntoFloat4(color);
    TailShading(vfScreenAddress, fNodeDepth, vData, vColor, fcolor);

    pointer = NODE_NEXT(pointer);
  }


  #if TAIL_COMPRESS
    float fTailAlphaInv = fcolor.w;
    fcolor.rgba = vec4(0, 0, 0, 1);
  #endif

  // Blend the k nearest layers of fragments from back to front, where k = MAX_TOP_LAYERS_EYE
  for (int j = 0; j < KBUFFER_SIZE; j++) {
    float kbufferFurthestDepth = 0;
    int id = FindFurthestKBufferEl(kBuffer, kbufferFurthestDepth);

    // read node to be applied
    uint nodeDepth = GET_KBUFFER_DEPTH(id);
    uint data = GET_KBUFFER_DATA(id);
    uint color = GET_KBUFFER_COLOR(id);

    // take this node out of the next search
    STORE_KBUFFER_DEPTH(id, DEPTH_RESET_TO_CLOSE);

    // Use high quality shading for the nearest k fragments
    float fDepth = float(nodeDepth);
    vec4 vData = UnpackUintIntoFloat4(data);
    vec4 vColor = UnpackUintIntoFloat4(color);
    vec4 fragmentColor = HEAD_SHADING(vfScreenAddress, fDepth, vData, vColor.rgb);

    #if TAIL_COMPRESS
      fragmentColor.a = 1 - (1 - fragmentColor.a) * fTailAlphaInv;
      //fTailAlphaInv = 1;
    #endif

    // Blend the fragment color
    fcolor = mix(fcolor, vec4(0), fragmentColor.a); // previous color contrib
    fcolor.rgb += fragmentColor.rgb * fragmentColor.a; // add current color
  }

  return fcolor;
}


// END TressFXPPLL.resolve.glsl
