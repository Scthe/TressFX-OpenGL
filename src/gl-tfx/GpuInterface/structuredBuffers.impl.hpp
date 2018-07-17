#define PRINT_DEBUG_SB() glTFx::debug::debug_StructuredBuffer( \
  __FUNCTION__, resourceName, structSize, structCount);

static EI_Resource* createSB_helper(TFx_ResourceType type, u32 size,
  EI_StringHash objectName, EI_StringHash resourceName)
{
  EI_Resource* pRW2D = new EI_Resource;
  pRW2D->type = type;
  pRW2D->name = resourceName; // TODO should be "${objectName}.${resourceName}"
  pRW2D->buffer = glUtils::malloc(size, glUtils::BufferUsagePattern::Dynamic);
  return pRW2D;
}

// Creates structured buffer. SRV only.  Begin state should be Upload.
// Creates a structured buffer and srv (StructuredBuffer). It necessarily needs data to start, so
// begin state should be EI_STATE_COPY_DEST.
EI_Resource* TFx_cbCreateReadOnlySB(EI_Device*  pContext,
    const AMD::uint32 structSize, const AMD::uint32 structCount,
    EI_StringHash resourceName, EI_StringHash objectName)
{
  PRINT_DEBUG_SB();
  u32 mem = structSize * structCount;
  return createSB_helper(TFx_ResourceType::StructuredBufferR, mem, objectName, resourceName);
}


// Creates read/write structured buffer. has UAV so begin state should be UAV.
// Creates a structured buffer and default UAV/SRV (StructuredBuffer and RWStructuredBuffer in
// HLSL).  It will be used as UAV, so begin state should be EI_STATE_UAV.
EI_Resource* TFx_cbCreateReadWriteSB(EI_Device*  pContext,
    const AMD::uint32 structSize, const AMD::uint32 structCount,
    EI_StringHash resourceName, EI_StringHash objectName)
{
  PRINT_DEBUG_SB();
  u32 mem = structSize * structCount;
  return createSB_helper(TFx_ResourceType::StructuredBufferRW, mem, objectName, resourceName);
}

// Creates read/write structured buffer with a counter. has UAV so begin state should be UAV.
// Same as EI_CreateReadWriteSB, but also includes an atomic counter.  Atomic counter is cleared to
// zero each frame (EI_SB_ClearCounter)
EI_Resource* TFx_cbCreateCountedSB(EI_Device*  pContext,
    const AMD::uint32 structSize, const AMD::uint32 structCount,
    EI_StringHash resourceName, EI_StringHash objectName)
{
  PRINT_DEBUG_SB();

  // this will allocate SSBO and space for atomic counter
  u32 mem = structSize * structCount;
  EI_Resource* pRW2D = new EI_Resource;
  pRW2D->type = TFx_ResourceType::StructuredBufferRW_WithAtomicCounter;
  pRW2D->name = resourceName; // TODO should be "${objectName}.${resourceName}"
  pRW2D->buffer_with_counter.buffer = glUtils::malloc(mem, glUtils::BufferUsagePattern::Dynamic);
  pRW2D->buffer_with_counter.counter = glUtils::malloc(sizeof(GLuint), glUtils::BufferUsagePattern::Dynamic);
  return pRW2D;
}

// NOTE: we are going to use this only with StructuredBufferRW_WithAtomicCounter,
// so might as well put into this file (to keep in same file all counter things)
void TFx_cbClearCounter(EI_CommandContextRef pContext, EI_StructuredBufferRef sb, AMD::uint32 clearValue) {
  LOGD << "[TFx_cbClearCounter] clearing_counter for " << sb.name << " with value " << clearValue;

  GFX_FAIL_IF(sb.type != TFx_ResourceType::StructuredBufferRW_WithAtomicCounter,
      "Tried to clear atomic counter on resource that is not"
      " StructuredBufferRW_WithAtomicCounter. How?");

  glUtils::RawBuffer& buf = sb.buffer_with_counter.counter;
  u32 val = clearValue;
  glUtils::write(buf, {0, sizeof(GLuint)}, &val);

  // (void)pContext;
  // sb.uav->SetInitialCount(clearValue);
}

/*
// Generates a name from object and resource name to create a unique, single string.
// For example, "ruby.positions" or just "positions".
static void GenerateCompositeName(SuString& fullName, const char* resourceName, const char* objectName) {
  SU_ASSERT(resourceName != nullptr && objectName != nullptr);

  if (strlen(objectName) > 0){
    fullName.Format("%s.%s", resourceName, objectName);
  } else {
    fullName.Format("%s", resourceName);
  }
}

// This is just a helper function right now.
EI_Resource* SuCreateSB(EI_Device*  pContext,
    const AMD::uint32 structSize, const AMD::uint32 structCount,
    EI_StringHash resourceName, EI_StringHash objectName,
    bool bUAV, bool bCounter)
{
  // you can't have a counter if you don't also have a UAV.
  SU_ASSERT(!bCounter || (bCounter && bUAV));

  EI_Resource* resource_p = new EI_Resource;
  // EI_Resource& r = *resource_p;

  //    r.resource = SuGPUStructuredBufferPtr(0);
  //    r.uav      = SuGPUUnorderedAccessViewPtr(0);
  //    r.srv      = SuGPUSamplingResourceViewPtr(0);

  SuString strHash;
  GenerateCompositeName(strHash, resourceName, objectName);

  SuGPUResourceManager* pManager = (SuGPUResourceManager*)pContext;

  SuGPUStructuredBufferPtr sbPtr = pManager->CreateStructuredBuffer(
      (uint32_t)structSize, (uint32_t)structCount,
      (bUAV ? SuGPUResource::BIND_SHADER_RESOURCE | SuGPUResource::BIND_UNORDERED_ACCESS
            : SuGPUResource::BIND_SHADER_RESOURCE),
      SuMemoryBufferPtr(0),  // initial data.  We will map and upload explicitly.
      strHash, 0, 0);

  v->resource = sbPtr.cast<SuGPUResource>();
  SU_ASSERT(resource_p->resource);

  SuGPUResourceDescription resourceDesc;
  resourceDesc.nFlags                = 0;
  resourceDesc.Buffer.nElementOffset = 0;
  resourceDesc.Buffer.nElementWidth  = (uint32_t)structCount;

  SU_ASSERT(SuGPUResource::GPU_RESOURCE_BUFFER == resource_p->resource->GetType());

  SuGPUResourceViewDescription srvDesc(
      SU_SAMPLING_VIEW, SU_FORMAT_UNKNOWN, SuGPUResource::GPU_RESOURCE_BUFFER, resourceDesc);
  resource_p->srv = resource_p->resource->GetSamplingView(srvDesc);
  SU_ASSERT(resource_p->srv);

  if (bCounter) {
    resourceDesc.nFlags = SU_COUNTER_VIEW; // adding counter view for UAV
  }

  if (bUAV) {
    SuGPUResourceViewDescription uavDesc = SuGPUResourceViewDescription(
        SU_UNORDERED_ACCESS_VIEW,
        SU_FORMAT_UNKNOWN,
        SuGPUResource::GPU_RESOURCE_BUFFER,
        resourceDesc);
    resource_p->uav = resource_p->resource->GetUnorderedAccessView(uavDesc);

    SU_ASSERT(resource_p->uav);
  }

  return resource_p;
}

// Note from me: Concated docs from AMD (header+impl). [sic]


// Creates structured buffer. SRV only.  Begin state should be Upload.
// Creates a structured buffer and srv (StructuredBuffer). It necessarily needs data to start, so
// begin state should be EI_STATE_COPY_DEST.
EI_Resource* SuCreateReadOnlySB(EI_Device*  pContext,
    const AMD::uint32 structSize,
    const AMD::uint32 structCount,
    EI_StringHash     resourceName, EI_StringHash objectName)
{
    return SuCreateSB(pContext, structSize, structCount, resourceName, objectName, false, false);
}


// Creates read/write structured buffer. has UAV so begin state should be UAV.
// Creates a structured buffer and default UAV/SRV (StructuredBuffer and RWStructuredBuffer in
// HLSL).  It will be used as UAV, so begin state should be EI_STATE_UAV.
EI_Resource* SuCreateReadWriteSB(EI_Device*  pContext,
    const AMD::uint32 structSize,
    const AMD::uint32 structCount,
    EI_StringHash     resourceName, EI_StringHash objectName)
{
    return SuCreateSB(pContext, structSize, structCount, resourceName, objectName, true, false);
}

// Creates read/write structured buffer with a counter. has UAV so begin state should be UAV.
// Same as EI_CreateReadWriteSB, but also includes an atomic counter.  Atomic counter is cleared to
// zero each frame (EI_SB_ClearCounter)
EI_Resource* SuCreateCountedSB(EI_Device*  pContext,
    const AMD::uint32 structSize,
    const AMD::uint32 structCount,
    EI_StringHash     resourceName, EI_StringHash objectName)
{
    return SuCreateSB(pContext, structSize, structCount, resourceName, objectName, true, true);
}
*/
