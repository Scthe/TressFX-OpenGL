static ShaderResource get_resource (glUtils::Shader* shader, glUtils::ShaderResourceName name, bool is_ubo) {
  ShaderResource res;
  res.name = name;

  auto block_p = shader->get_block(name, is_ubo);
  if (block_p) {
    res.is_block = true;
    res.block = block_p;
    res.shader = shader;
  } else {
    auto var_p = shader->get_uniform(name);
    if (var_p) {
      res.is_block = false;
      res.variable = var_p;
      res.shader = shader;
    }
  }
  return res;
}

EI_BindLayout* TFx_cbCreateLayout(EI_Device* pDevice, EI_LayoutManagerRef layoutManager,
  const AMD::TressFXLayoutDescription& description)
{
  LOGD << "[TFx_cbCreateLayout] (nSRVs=" << description.nSRVs
       << ", nUAVs=" << description.nUAVs
       << ", nConstants=" << description.constants.nConstants << ")";
  glTFx::debug::debug_layout(description);
  glTFx::debug::debug_shader(*layoutManager.shader);

  // NOTE: we use SSBO everywhere
  EI_BindLayout* pLayout = new EI_BindLayout;
  auto pEffect = layoutManager.shader;
  GFX_FAIL_IF(!pEffect, "Layout manager should contain pointer to shader, was nullptr");
  bool allow_ubo = false;

  for (int i = 0; i < description.nSRVs; ++i) {
    const auto& name = description.srvNames[i];
    ShaderResource res = get_resource(pEffect, name, allow_ubo);
    // we have to push_back, as during bind we do not connect based on names,
    // but indices. Skipping push_back would shift indices.
    // Fortunatelly, ShaderResource#was_found_in_glsl exists
    pLayout->srvs.push_back(res);
    if (!res.shader) {
      LOGW << "Could not find SRV(SSBO): " << name
      << " in Shader(vs=" << layoutManager.vs_path << ", ps=" << layoutManager.fs_path << ")";
    }
  }

  for (int i = 0; i < description.nUAVs; ++i) {
    const auto& name = description.uavNames[i];
    ShaderResource res = get_resource(pEffect, name, allow_ubo);
    // we have to push_back, as during bind we do not connect based on names,
    // but indices. Skipping push_back would shift indices.
    // Fortunatelly, ShaderResource#was_found_in_glsl exists
    pLayout->uavs.push_back(res);
    if (!res.shader) {
      LOGW << "Could not find UAV(SSBO): " << name
           << " in Shader(vs=" << layoutManager.vs_path << ", ps=" << layoutManager.fs_path << ")";
    }
  }

  const auto& consts = description.constants;
  for (int i = 0; i < consts.nConstants; i++) {
    const auto& name = consts.parameterNames[i];
    auto var_p = pEffect->get_uniform(name);
    if (var_p) {
      pLayout->constants.push_back(var_p);
    } else {
      LOGW << "Could not find Uniform: " << name
           << " in Shader(vs=" << layoutManager.vs_path << ", ps=" << layoutManager.fs_path << ")";
    }
  }

  return pLayout;
}

void TFx_cbDestroyLayout(EI_Device* pDevice, EI_BindLayout* pLayout)
{
  AMD_SAFE_DELETE(pLayout);
}


/*
EI_BindLayout* SuCreateLayout(EI_Device* pDevice, EI_LayoutManagerRef layoutManager, const AMD::TressFXLayoutDescription& description)
{
    // unused
    (void)pDevice;
    (void)description.constants.constantBufferName; // Sushi doesn't use constant buffer names.  It sets individually.

    // main:
    EI_BindLayout* pLayout = new EI_BindLayout;
    SuEffectPtr pEffect = GetEffect(layoutManager);

    for (int i = 0; i < description.nSRVs; ++i) {
        pLayout->srvs.push_back(pEffect->GetTextureSlot(description.srvNames[i]));
    }

    for (int i = 0; i < description.nUAVs; ++i) {
        const SuUAVSlot* pSlot = pEffect->GetUAVSlot(description.uavNames[i]);
        SU_ASSERT(pSlot != nullptr);
        pLayout->uavs.push_back(pSlot);
    }

    for (int i = 0; i < description.constants.nConstants; i++) {
        pLayout->constants.push_back(pEffect->GetParameter(description.constants.parameterNames[i]));
    }

    return pLayout;
}

void SuDestroyLayout(EI_Device* pDevice, EI_BindLayout* pLayout)
{
    (void)pDevice;
    AMD_SAFE_DELETE(pLayout);
}
*/
