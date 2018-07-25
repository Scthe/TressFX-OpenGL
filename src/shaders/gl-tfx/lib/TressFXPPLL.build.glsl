// START TressFXPPLL.build.glsl

// 2D texture to store head pointers
// RWTexture2D<int> tRWFragmentListHead;
layout(location=1, r32ui) // why location, not binding? everyone uses binding, but location seems to be correct
uniform uimage2D tRWFragmentListHead;

// SSBO to store nodes
// RWStructuredBuffer<PPLL_STRUCT> LinkedListUAV;
STRUCTURED_BUFFER(0, PPLL_STRUCT , LinkedListUAV)
layout(binding=0, offset=0) uniform atomic_uint LinkedListUAV_counter;

// size of LinkedListUAV
// width * height * AVE_FRAGS_PER_PIXEL(4)
uniform int nNodePoolSize;

uint PackFloat4IntoUint(vec4 vValue) {
  return ((uint(vValue.x * 255)) << 24) |
         ((uint(vValue.y * 255)) << 16) |
         ((uint(vValue.z * 255)) << 8) |
           uint(vValue.w * 255);
}

// Allocate a new fragment location in fragment color, depth, and link buffers
uint AllocateFragment(ivec2 vScreenAddress) {
  uint newAddress = atomicCounterIncrement(LinkedListUAV_counter); //LinkedListUAV.IncrementCounter();
  if (newAddress <= 0 || newAddress > nNodePoolSize){
    newAddress = FRAGMENT_LIST_NULL;
  }
  return newAddress;
}

// Insert a new fragment at the head of the list. The old list head becomes the
// the second fragment in the list and so on. Return the address of the *old* head.
uint MakeFragmentLink(ivec2 vScreenAddress, uint nNewHeadAddress) {
    // int nOldHeadAddress;
    // InterlockedExchange(tRWFragmentListHead[vScreenAddress], nNewHeadAddress, nOldHeadAddress);
    uint nOldHeadAddress = imageAtomicExchange(tRWFragmentListHead, vScreenAddress, nNewHeadAddress);
    return nOldHeadAddress;
}


// Write fragment attributes to list location.
void WriteFragmentAttributes(uint nAddress, uint nPreviousLink, vec4 vData, vec3 vColor3, float fDepth) {
    LinkedListUAV_[nAddress].data  = PackFloat4IntoUint(vData);
    LinkedListUAV_[nAddress].color = PackFloat4IntoUint(vec4(vColor3, 0));
    LinkedListUAV_[nAddress].depth = uint(fDepth * 255.0); //uint(saturate(fDepth)); or gl_FragCoord.z; ?
    LinkedListUAV_[nAddress].uNext = nPreviousLink;
}

// END TressFXPPLL.build.glsl
