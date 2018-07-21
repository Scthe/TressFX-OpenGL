EI_BindSet* TFx_cbCreateBindSet(EI_Device* commandContext, AMD::TressFXBindSet& bindSet) {
  LOGD << "[TFx_cbCreateBindSet] (nSRVs=" << bindSet.nSRVs
       << ", nUAVs="<<bindSet.nUAVs
       << ", has_values=" << (!!bindSet.values) << ")";
  glTFx::debug::debug_bindSet(bindSet);

  if (bindSet.values != 0) {
    LOGW << "[TFx_cbCreateBindSet] TressFXBindSet.values != 0, not sure what that means";
  }
  // GFX_FAIL_IF(bindSet.values != 0, "[TFx_cbCreateBindSet] TressFXBindSet.values is not 0."
    // "Not implemented!");

  EI_BindSet* pBindSet = new EI_BindSet;
  pBindSet->nBytes = bindSet.nBytes;
  pBindSet->values = bindSet.values;

  for (int i = 0; i < bindSet.nSRVs; ++i) {
    pBindSet->srvs.push_back(bindSet.srvs[i]);
  }

  for (int i = 0; i < bindSet.nUAVs; ++i) {
    pBindSet->uavs.push_back(bindSet.uavs[i]);
  }

  return pBindSet;
}

void TFx_cbDestroyBindSet(EI_Device* pDevice, EI_BindSet* pBindSet) {
  // NOP, we are using std::vector after all
}

static void bind_ClearableTexture2D (const EI_Resource& resource_instance, ShaderResource& resource_glsl) {
  // is load/store image, that offers random access texture
  // e.g. PPLLHeads(tRWFragmentListHead)
  // @see https://stackoverflow.com/questions/35716267/opengl-glactivetexture-vs-glbindimagetexture?rq=1
  GFX_FAIL_IF(resource_glsl.is_block, "Tried to bind ClearableTexture2D ",
    resource_instance.name, " to shader binding ", resource_glsl.name, ","
    " but in glsl this variable is a block (SSBO/UBO)");
  GFX_FAIL_IF(resource_glsl.variable->gl_type != GL_UNSIGNED_INT_IMAGE_2D,
    "Tried to bind ClearableTexture2D ", resource_instance.name,
    " to shader binding ", resource_glsl.name, ", but in glsl it has type ",
    resource_glsl.variable->gl_type, " instead of GL_UNSIGNED_INT_IMAGE_2D(",
    GL_UNSIGNED_INT_IMAGE_2D, ")");
  const auto& tex = resource_instance.texture.texture;
  glBindImageTexture(resource_glsl.variable->gl_location, tex.gl_id,
    0, GL_FALSE, 0, // GLint (mipmap)level​, GLboolean layered​, GLint layer
    GL_READ_WRITE, tex.sized_pixel_format);
}

static void bind_buffer_to_ssbo (const EI_StringHash& buffer_name, const glUtils::RawBuffer& buf,
  ShaderResource& resource_glsl)
{
  bool is_glsl_SSBO = resource_glsl.is_block
                   && resource_glsl.block->is_ubo == false;
  GFX_FAIL_IF(!is_glsl_SSBO, "Tried to bind StructuredBuffer[R,RW,RW_AtomicCounter] ",
    buffer_name, " to shader binding ", resource_glsl.name, ", but in glsl this variable is NOT SSBO");

  auto& shader = *resource_glsl.shader;
  auto& block = *resource_glsl.block;
  glUtils::set_shader_buffer(shader, block.gl_binding, block, buf, {0, buf.bytes});
}

static void bind (const EI_Resource& resource_instance, ShaderResource& resource_glsl) {
  // resource_instance := instance of the resource, that is opengl buffer/texture
  // resource_glsl := representation from glsl, e.g. buffer SSBO_A {...};
  // in this function we connnect these resources

  auto& shader = *resource_glsl.shader;

  switch (resource_instance.type) {
    case TFx_ResourceType::ClearableTexture2D:
      bind_ClearableTexture2D(resource_instance, resource_glsl);
      break;
    case TFx_ResourceType::StructuredBufferRW_WithAtomicCounter: {
      // bind SSBO
      bind_buffer_to_ssbo(resource_instance.name, resource_instance.buffer_with_counter.buffer, resource_glsl);
      // bind atomic
      // (will proceed assuming same gl_binding value as SSBO. This is.. sketchy,
      // but it's the only way to have reasonable API)
      auto gl_binding = resource_glsl.block->gl_binding;
      auto at = shader.get_atomic(gl_binding);
      if (at) {
        glUtils::set_shader_atomic(shader, gl_binding, resource_instance.buffer_with_counter.counter, 0);
      } else {
        LOGW << "No atomics to set were found on binding " << gl_binding <<
           ". Verify they are actually used. This is not an error when binding"
           " StructuredBufferRW_WithAtomicCounter, as there is not always need"
           " to use both buffer and atomic.";
      }
      break;
    }
    case TFx_ResourceType::StructuredBufferRW:
      bind_buffer_to_ssbo(resource_instance.name, resource_instance.buffer, resource_glsl);
      break;
    case TFx_ResourceType::StructuredBufferR:
      bind_buffer_to_ssbo(resource_instance.name, resource_instance.buffer, resource_glsl);
      break;
  }
}

static void update_constants(std::vector<const EffectParameter*>& cb, char* values, int nBytes) {
  LOGT << "[TFx_cbBind] update_constants(size=" << cb.size() << ", nBytes=" << nBytes << ")";

  u32 bytes_consumed = 0;
  for (u32 i = 0; i < cb.size(); ++i) {
    const glUtils::ShaderVariable& pParam = *cb[i];
    LOGT << pParam.name << "(gl_type=" << pParam.gl_type << ")";

    auto used_bytes = glUtils::set_uniform(pParam, (void*)(values + bytes_consumed));
    LOGT << "   used: " << used_bytes << " bytes, this should match sizeof provided type";
    bytes_consumed += used_bytes;

    // IF SEGFAULTED AROUND THIS FN READ THIS:
    GFX_FAIL_IF(bytes_consumed > (u32) nBytes, "Setting uniform values one by one"
      " resulted in ", bytes_consumed, " bytes consumed from void*. Meanwhile"
      " the void* has only ", nBytes, " bytes. This warning is useless, cause"
      " we SEGFAULTed before this assert could be reached.");
  }

  bool all_bytes_consumed = bytes_consumed == (u32) nBytes;
  if (!all_bytes_consumed) {
    LOGW << "When updating constants not all bytes from provided void* were used."
            " Provided " << nBytes << " bytes, while consumed " << bytes_consumed
            << " bytes. This may be due to the padding used at the end of structs."
            " See LogLevel::Trace for more details about which constants were involved.";
  }
  // GFX_FAIL_IF(!all_bytes_consumed, "Tried setting constants from void*."
      // " Consumed ", bytes_consumed, " bytes, but should have consumed ",
      // nBytes, " bytes. Check if all types match. If they do, there may be"
      // " an offset problem. bwahahahahahahahahahahahah You are F_cked");
}

static void verify_glsl_variable_exists (ShaderResource& resource_glsl, EI_StringHash& buffer_name) {
  GFX_FAIL_IF(!resource_glsl.was_found_in_glsl(), "Tried to bind resource ",
    buffer_name, " to shader binding for ", resource_glsl.name,
    " that was not found in shader code. It could be that glsl does not "
    "actually use this variable, or smth, anyway please investigate");
}

void TFx_cbBind(EI_CommandContextRef commandContext, EI_BindLayout* pLayout, EI_BindSet& set) {
  // I guess this constructs 2-way binding table: uniform_block -> TABLE <- shader_resource_id
  LOGD << "[TFx_cbBind] ";
  GFX_FAIL_IF(!pLayout, "pLayout provided to TFx_cbBind is null."
      " That would mean shader introspect failed?");

  for (u32 i = 0; i < set.srvs.size(); ++i) {
    auto& bind_item = *set.srvs[i];
    auto& layout_item = pLayout->srvs[i];
    verify_glsl_variable_exists(layout_item, bind_item.name);
    LOGT << "Bind SRV resource " << glTFx::debug::debug_EI_Resource(bind_item)
         << " to Shader variable " << glTFx::debug::debug_ShaderResource(layout_item);
    bind(bind_item, layout_item);
  }

  for (u32 i = 0; i < set.uavs.size(); ++i) {
    auto& bind_item = *set.uavs[i];
    auto& layout_item = pLayout->uavs[i];
    verify_glsl_variable_exists(layout_item, bind_item.name);
    LOGT << "Bind UAV resource " << glTFx::debug::debug_EI_Resource(bind_item)
         << " to Shader variable " << glTFx::debug::debug_ShaderResource(layout_item);
    bind(bind_item, layout_item);
  }

  LOGW << "Skip update_constants, cause we hardcode everything";
  // update_constants(pLayout->constants, (char*)set.values, set.nBytes);
}



/*
typedef SuGPUSamplingResourceViewPtr SuSRVPtr;
typedef SuGPUUnorderedAccessViewPtr SuUAVPtr;

class EI_BindSet {
  public:
  int     nSRVs;
  SuSRVPtr* srvs;
  int     nUAVs;
  SuUAVPtr* uavs;
  //SuArray<SuGPUSamplingResourceViewPtr> srvs;
  //SuArray<SuGPUUnorderedAccessViewPtr> uavs;

  void*   values;
  int     nBytes;
};

EI_BindSet* SuCreateBindSet(EI_Device* commandContext, AMD::TressFXBindSet& bindSet)
{
    EI_BindSet* pBindSet = new EI_BindSet;

    // should just replace with a SuArray.  Same as constant buffers.

    pBindSet->nBytes = bindSet.nBytes;
    pBindSet->values = bindSet.values;

    // Maybe replace this with SuArray usage.

    pBindSet->nSRVs = bindSet.nSRVs;
    if (bindSet.nSRVs > 0)
    {
        pBindSet->srvs = new SuSRVPtr[bindSet.nSRVs];
        for (int i = 0; i < bindSet.nSRVs; ++i)
        {
            pBindSet->srvs[i] = bindSet.srvs[i]->srv;
        }
    }
    else
    {
        pBindSet->srvs = nullptr;
    }

    pBindSet->nUAVs = bindSet.nUAVs;
    if (pBindSet->nUAVs > 0)
    {
        pBindSet->uavs = new SuUAVPtr[bindSet.nUAVs];
        for (int i = 0; i < bindSet.nUAVs; ++i)
        {
            pBindSet->uavs[i] = bindSet.uavs[i]->uav;
        }
    }
    else
    {
        pBindSet->uavs = nullptr;
    }

    return pBindSet;
}

void SuDestroyBindSet(EI_Device* pDevice, EI_BindSet* pBindSet)
{
    if (pBindSet->nSRVs > 0)
    {
        // should just call destructors,which should be setting to null.
        delete[] pBindSet->srvs;
    }

    if (pBindSet->nUAVs > 0)
    {
        delete[] pBindSet->uavs;
    }

    delete pBindSet;
}

static void UpdateConstants(SuArray<SuEffectParameter*>& cb, void* values, int nBytes) {
  uint8* pCurrent = (uint8*)values;

  // seems to be CPU-mapped-like buffer, where SetRawValue would trigger sync CPU-GPU
  for (AMD::int32 i = 0; i < cb.size(); ++i) {
    SuEffectParameter* pParam = cb[i];
    uint32 nParamBytes = pParam->GetParameterSize();
    uint32 nCummulativeBytes = static_cast<uint32>(pCurrent - (uint8*)values) + nParamBytes;
    SU_ASSERT(nBytes >= 0);

    if (nCummulativeBytes >(uint32)nBytes) {
      SuLogWarning("Layout looking for %ud bytes so far, but bindset only has %ud bytes.",
          (uint32)nCummulativeBytes,
          (uint32)nBytes);
    }

    pParam->SetRawValue(pCurrent, nParamBytes);
    pCurrent += nParamBytes;
  }
}

void SuBind(EI_CommandContextRef commandContext, EI_BindLayout* pLayout, EI_BindSet& set)
{
    (void)commandContext;
    SU_ASSERT(set.nSRVs == pLayout->srvs.size());
    for (AMD::int32 i = 0; i < set.nSRVs; ++i)
    {
        pLayout->srvs[i]->BindResource(set.srvs[i]);
    }

    SU_ASSERT(set.nUAVs == pLayout->uavs.size());
    for (AMD::int32 i = 0; i < set.nUAVs; ++i)
    {
        pLayout->uavs[i]->BindResource(set.uavs[i]);
    }

    UpdateConstants(pLayout->constants, set.values, set.nBytes);

}
*/
