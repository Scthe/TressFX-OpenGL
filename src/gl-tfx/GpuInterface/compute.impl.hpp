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

static void update_uniforms_simulation (glUtils::Shader& shader, GlobalState& state, TressFXHairObject& hair_obj) {
  auto& settings = state.tfx_settings;
  auto& sim_s = settings.simulation_settings;
  f32 UNUSED = 0.0; i32 UNUSED_I = 0;

  // GFX_FAIL("swap include in shaders! Modify mock step by step and see what comes. Start with gravity");
  glm::vec3 wind_dir = { sim_s.m_windDirection[0], sim_s.m_windDirection[1], sim_s.m_windDirection[2] };
  wind_dir = glm::normalize(wind_dir) * sim_s.m_windMagnitude;
  glm::vec4 g_Wind = { wind_dir,  UNUSED };
  glm::vec4 g_Wind1 = {0,0,0,0};
  glm::vec4 g_Wind2 = {0,0,0,0};
  glm::vec4 g_Wind3 = {0,0,0,0};
  glm::vec4 g_Shape = {
    sim_s.m_damping,
    sim_s.m_localConstraintStiffness,
    sim_s.m_globalConstraintStiffness,
    sim_s.m_globalConstraintsRange };
  glm::vec4 g_GravTimeTip = {
    sim_s.m_gravityMagnitude * settings.gravity_multipler, // g_GravityMagnitude
    (1.0 / 60.0), // g_TimeStep TODO hardcoded here?
    sim_s.m_tipSeparation, // g_TipSeparationFactor
    UNUSED };
  glm::ivec4 g_SimInts = {
    sim_s.m_lengthConstraintsIterations,
    sim_s.m_localConstraintsIterations, // ? g_NumLocalShapeMatchingIterations AMD has collision? Either way unused in shader code..
    UNUSED_I,
    UNUSED_I };
  glm::ivec4 g_Counts = {
    TRESSFX_SIM_THREAD_GROUP_SIZE / hair_obj.GetNumVerticesPerStrand(), // g_NumOfStrandsPerThreadGroup (2)
    settings.follow_hairs_per_guide_hair, // g_NumFollowHairsPerGuideHair
    hair_obj.GetNumVerticesPerStrand(), // g_NumVerticesPerStrand, unused in shader code
    UNUSED_I };
  glm::vec4 g_VSP = {
    sim_s.m_vspCoeff,
    sim_s.m_vspAccelThreshold,
    UNUSED,
    UNUSED };

    glUtils::set_uniform(shader, "g_Wind", g_Wind);
    glUtils::set_uniform(shader, "g_Wind1", g_Wind1);
    glUtils::set_uniform(shader, "g_Wind2", g_Wind2);
    glUtils::set_uniform(shader, "g_Wind3", g_Wind3);
    glUtils::set_uniform(shader, "g_Shape", g_Shape);
    glUtils::set_uniform(shader, "g_GravTimeTip", g_GravTimeTip);
    glUtils::set_uniform(shader, "g_SimInts", g_SimInts);
    glUtils::set_uniform(shader, "g_Counts", g_Counts);
    glUtils::set_uniform(shader, "g_VSP", g_VSP);
}

// All our compute shaders have dimensions of (N,1,1)
void TFx_cbDispatch(EI_CommandContextRef commandContext, EI_PSO& pso, int nThreadGroups) {
  LOGD << "[TFx_cbDispatch]";

  GFX_FAIL_IF(!commandContext.simulated_hair_object,
      "[TFx_cbDispatch] commandContext.simulated_hair_object was not set, "
      "this would make binding not possible");
  GFX_FAIL_IF(!commandContext.state,
      "[TFx_cbDispatch] commandContext.state was not set, "
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
  update_uniforms_simulation(shader, *commandContext.state, hairObject);

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
