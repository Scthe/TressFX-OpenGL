static bool get_resource (glUtils::Shader* shader,
  const glUtils::ShaderResourceName name, ShaderResource& out)
{
  bool found = false;
  bool is_ubo = false; // all are ssbo

  auto block_p = shader->get_block(name, is_ubo);
  if (block_p) {
    out.is_block = true;
    out.block = block_p;
    out.shader = shader;
    found = true;
  } else {
    auto var_p = shader->get_uniform(name);
    if (var_p) {
      out.is_block = false;
      out.variable = var_p;
      out.shader = shader;
      found = true;
    }
  }
  return found;
}

static ShaderResource get_resource_from_one_of_shaders (
  const std::vector<ShaderMetadata>& shaders,
  const glUtils::ShaderResourceName& name)
{
  ShaderResource res;
  res.name = name;

  for (size_t i = 0; i < shaders.size(); i++) {
    if (get_resource(shaders[i].shader, name, res)) {
      break;
    }
  }

  return res;
}

static const glUtils::ShaderVariable* get_uniform_from_one_of_shaders (
  const std::vector<ShaderMetadata>& shaders,
  const glUtils::ShaderResourceName& name)
{
  for (size_t i = 0; i < shaders.size(); i++) {
    auto uni = shaders[i].shader->get_uniform(name);
    if (uni) {
      return uni;
    }
  }

  return nullptr;
}

static void validate_shader_metadata (const ShaderMetadata& smd) {
  GFX_FAIL_IF(!smd.shader, "Layout manager should contain pointer to shader,"
    " was nullptr for shaders program that uses", smd.path_1);
}

EI_BindLayout* TFx_cbCreateLayout(EI_Device* pDevice, EI_LayoutManagerRef layoutManager,
  const AMD::TressFXLayoutDescription& description)
{
  LOGD << "[TFx_cbCreateLayout] (nSRVs=" << description.nSRVs
       << ", nUAVs=" << description.nUAVs
       << ", nConstants=" << description.constants.nConstants << ")";
  glTFx::debug::debug_layout(description);
  for (const auto& l : layoutManager.shaders) {
    validate_shader_metadata(l);
    glTFx::debug::debug_shader(*l.shader);
  }

  const ShaderMetadata& shader_to_print_debug = layoutManager.shaders[0];

  // NOTE: we use SSBO everywhere
  EI_BindLayout* pLayout = new EI_BindLayout;


  for (int i = 0; i < description.nSRVs; ++i) {
    const auto& name = description.srvNames[i];
    ShaderResource res = get_resource_from_one_of_shaders(layoutManager.shaders, name);
    // we have to push_back, as during bind we do not connect based on names,
    // but indices. Skipping push_back would shift indices.
    // Fortunatelly, ShaderResource#was_found_in_glsl exists
    pLayout->srvs.push_back(res);
    if (!res.shader) {
      LOGW << "Could not find SRV(SSBO): " << name
           << " during evaluation of shaders: [" << shader_to_print_debug.path_1 << ", ... ]";
    }
  }

  for (int i = 0; i < description.nUAVs; ++i) {
    const auto& name = description.uavNames[i];
    ShaderResource res = get_resource_from_one_of_shaders(layoutManager.shaders, name);
    // we have to push_back, as during bind we do not connect based on names,
    // but indices. Skipping push_back would shift indices.
    // Fortunatelly, ShaderResource#was_found_in_glsl exists
    pLayout->uavs.push_back(res);
    if (!res.shader) {
      LOGW << "Could not find UAV(SSBO): " << name
           << " during evaluation of shaders: [" << shader_to_print_debug.path_1 << ", ... ]";
    }
  }

  const auto& consts = description.constants;
  for (int i = 0; i < consts.nConstants; i++) {
    const auto& name = consts.parameterNames[i];
    auto var_p = get_uniform_from_one_of_shaders(layoutManager.shaders, name);
    if (var_p) {
      pLayout->constants.push_back(var_p);
    } else {
      LOGW << "Could not find Uniform: " << name
           << " during evaluation of shaders: [" << shader_to_print_debug.path_1 << ", ... ]";
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
