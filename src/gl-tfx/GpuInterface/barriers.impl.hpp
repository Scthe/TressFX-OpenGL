static const char* debug_EI_ResourceState(AMD::EI_ResourceState st) {
  switch (st) {
    case AMD::EI_STATE_NON_PS_SRV: return "EI_STATE_NON_PS_SRV";
    case AMD::EI_STATE_VS_SRV: return "EI_STATE_VS_SRV";
    case AMD::EI_STATE_CS_SRV: return "EI_STATE_CS_SRV";
    case AMD::EI_STATE_PS_SRV: return "EI_STATE_PS_SRV";
    case AMD::EI_STATE_UAV: return "EI_STATE_UAV";
    case AMD::EI_STATE_COPY_DEST: return "EI_STATE_COPY_DEST";
    case AMD::EI_STATE_COPY_SOURCE: return "EI_STATE_COPY_SOURCE";
    case AMD::EI_STATE_RENDER_TARGET: return "EI_STATE_RENDER_TARGET";
  }
}

static void debug_barrier (const AMD::EI_Barrier& bar) {
  LOGD << "[TFx_cbSubmitBarriers] " << bar.pResource->name
       << "(from=" << debug_EI_ResourceState(bar.from)
       << ", to=" << debug_EI_ResourceState(bar.to)
       << ")";
}

void TFx_cbSubmitBarriers(EI_CommandContextRef context,
  int numBarriers, AMD::EI_Barrier* barriers)
{
  for (int i = 0; i < numBarriers; ++i) {
    auto& bar = barriers[i];
    debug_barrier(bar);
  }

  // TODO optimize this, e.g. during load we can forgoe this,
  // as we have not executed any GPU code that would change resource memory
  if (numBarriers > 0) {
    glUtils::async_write_barrier(glUtils::AsyncWriteableResource::All);
  }
}

/*
static SuGPUResource::StateType TranslateState(AMD::EI_ResourceState inState) {
  switch (inState) {
    case AMD::EI_STATE_NON_PS_SRV:
      return SuGPUResource::STATE_NON_PIXEL_SHADER_RESOURCE;
    case AMD::EI_STATE_VS_SRV:
      return SuGPUResource::STATE_NON_PIXEL_SHADER_RESOURCE;
    case AMD::EI_STATE_CS_SRV:
      return SuGPUResource::STATE_NON_PIXEL_SHADER_RESOURCE;
    case AMD::EI_STATE_PS_SRV:
      return  SuGPUResource::STATE_PIXEL_SHADER_RESOURCE;
    case AMD::EI_STATE_UAV:
      return  SuGPUResource::STATE_UNORDERED_ACCESS;
    case AMD::EI_STATE_COPY_DEST:
      return  SuGPUResource::STATE_COPY_DEST;
    case AMD::EI_STATE_COPY_SOURCE:
      return  SuGPUResource::STATE_COPY_SOURCE;
    case AMD::EI_STATE_RENDER_TARGET:
      return  SuGPUResource::STATE_RENDER_TARGET;
  }

  // fallback.
  SuLogWarning("Unknown state.");
  return SuGPUResource::STATE_COMMON;
}

static void SuTransition(EI_Resource* pResource,
    AMD::EI_ResourceState from, AMD::EI_ResourceState to)
{
  if (from == to) {
    if (from == AMD::EI_STATE_UAV) {
      pResource->resource->UAVBarrier();
    } else {
      SuLogWarning("transition from %d to %d", (int)from, (int)to);
    }
  } else {
    pResource->resource->Transition(TranslateState(from), TranslateState(to));
  }
}

void SuSubmitBarriers(EI_CommandContextRef context, int numBarriers, AMD::EI_Barrier* barriers)
{
    (void)context;
    for (int i = 0; i < numBarriers; ++i)
    {
        SuTransition(barriers[i].pResource, barriers[i].from, barriers[i].to);
    }
}
*/
