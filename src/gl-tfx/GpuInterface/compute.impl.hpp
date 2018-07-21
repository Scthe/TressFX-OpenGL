static bool match(const EI_StringHash& str, const char* pat) {
  auto l1 = strlen(str);
  auto l2 = strlen(pat);
  auto min_len = l1 < l2 ? l1 : l2;
  auto res = strncmp(str, pat, min_len);
  return res == 0;
}

static i32 get_index_for_layoutManager (const EI_StringHash& shaderName) {
  #define CHECK(IDX, NAME) \
    if (match(shaderName, NAME)) { return IDX; }

  CHECK(0, "IntegrationAndGlobalShapeConstraints")
  CHECK(1, "VelocityShockPropagation")
  CHECK(2, "LocalShapeConstraints")
  CHECK(3, "LengthConstriantsWindAndCollision")
  CHECK(4, "UpdateFollowHairVertices")

  return -1;
  #undef CHECK
}

EI_PSO* TFx_cbCreateComputeShaderPSO(EI_Device* pDevice,
    EI_LayoutManagerRef layoutManager,
    const EI_StringHash& shaderName)
{
  LOGD << "[TFx_cbCreateComputeShaderPSO] " << shaderName;
  for (const auto& l : layoutManager.shaders) {
    glTFx::debug::debug_shader(*l.shader);
  }

  EI_PSO* pso = new EI_PSO;
  auto idx = get_index_for_layoutManager(shaderName);
  GFX_FAIL_IF(idx == -1 || (idx > (i32)layoutManager.shaders.size() - 1),
    "[TFx_cbCreateComputeShaderPSO] could not find compute"
    " shader with name ", shaderName, ". See LogLevel::Trace to check available"
    " shaders");

  pso->shader = layoutManager.shaders[idx].shader;

  return pso;
}

void TFx_cbDestroyPSO(EI_Device* pDevice, EI_PSO* pso) {
  // shaders are allocated/managed in TFxSimulation, nop here
  delete pso;
}

// All our compute shaders have dimensions of (N,1,1)
void TFx_cbDispatch(EI_CommandContextRef commandContext, EI_PSO& pso, int nThreadGroups) {
  LOGD << "[TFx_cbDispatch]";

  GFX_FAIL_IF(!commandContext.simulated_hair_object,
      "[TFx_cbDispatch] commandContext.simulated_hair_object was not set, "
      "this would make binding not possible");
  GFX_FAIL_IF(!pso.shader, "[TFx_cbDispatch] Tried to dispatch compute shader,"
      " but shader reference does not exists");

  auto& shader = *pso.shader;
  glUseProgram(shader.gl_id);
  glTFx::debug::debug_shader(shader);

  auto& hairObject = *commandContext.simulated_hair_object;

  // EI_Bind(commandContext, GetSimPosTanLayout(), hairObject.mPosTanCollection.GetSimBindSet());
  // EI_Bind(commandContext, GetSimLayout(), *hairObject.m_pSimBindSet);
  EI_Bind(commandContext, GetSimPosTanLayout(), hairObject.GetPosTanCollection().GetSimBindSet());
  EI_Bind(commandContext, GetSimLayout(), hairObject.GetSimBindSet());

  // do the dispatch
  // NOTE: provides thread groups, not global_work_size
  glDispatchCompute(nThreadGroups, 1, 1);
}


/*
EI_PSO* SuCreateComputeShaderPSO(EI_Device*     pDevice,
    EI_LayoutManagerRef     layoutManager,
    const EI_StringHash& shaderName)
{
    (void)pDevice;
    // SuEffect& effect = (SuEffect&)layoutManager;
    SuEffectPtr pEffect = GetEffect(layoutManager);

    //return const_cast<SuEffectTechnique*>(
    const SuEffectTechnique* pTechnique = pEffect->GetTechnique(SuString("TressFXSimulation_") + shaderName);

    return GetPSOPtr(pTechnique);
}

void SuDestroyPSO(EI_Device* pDevice, EI_PSO* pso)
{
    (void)pDevice;
    (void)pso;
}

// All our compute shaders have dimensions of (N,1,1)
void SuDispatch(EI_CommandContextRef commandContext, EI_PSO& pso, int nThreadGroups)
{
    (void)commandContext;

    //SuEffectTechnique* pTechnique = pso;
    SuEffectTechnique& technique = GetTechniqueRef(pso);
    SuEffect*          pEffect = const_cast<SuEffect*>(technique.GetParent());

    uint32 numPasses;

    pEffect->Begin(&technique, &numPasses);
    SU_ASSERT(numPasses == 1);
    pEffect->BeginPass(0);
    SuRenderManager::GetRef().Dispatch(nThreadGroups, 1, 1);
    pEffect->EndPass();
    pEffect->End();
}
*/
