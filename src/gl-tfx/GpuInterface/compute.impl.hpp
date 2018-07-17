EI_PSO* TFx_cbCreateComputeShaderPSO(EI_Device* pDevice,
    EI_LayoutManagerRef layoutManager,
    const EI_StringHash& shaderName)
{
  GFX_FAIL("TFx_cbCreateComputeShaderPSO");
  return nullptr;
}

void TFx_cbDestroyPSO(EI_Device* pDevice, EI_PSO* pso) {
  GFX_FAIL("TFx_cbDestroyPSO");
}

// All our compute shaders have dimensions of (N,1,1)
void TFx_cbDispatch(EI_CommandContextRef commandContext, EI_PSO& pso, int nThreadGroups) {
  GFX_FAIL("TFx_cbDispatch");
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
