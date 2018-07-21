#pragma once

// #include "SuTypes.h"
// #include "SushiGPUInterface.h"
// #include "EngineTressFXObject.h"
// #include "SuAnimatedModel.h"
// #include "SuCommandQueue.h"
// #include "SuEffect.h"
// #include "SuEffectReloadListener.h"
// #include "SuGPUResource.h"

#include "include_TFx_pls_no_warnings.hpp"
#include "../../libs/amd_tressfx/include/TressFXAsset.h"
#include "../../libs/amd_tressfx/include/TressFXGPUInterface.h"
#include "../../libs/amd_tressfx/include/TressFXSettings.h"

#include <vector>

// lib forwards
// -

struct GlobalState; // global state from GUI

namespace glTFx {

  class TFxPPLL;
  class TFxShortCut;
  class TFxHairStrands;
  class TFxSimulation;

  /*
  class EI_Resource;
  class EI_Device;
  class EI_CommandContext;      // ID3D11DeviceContext
  class EI_LayoutManager;
  struct EI_BindLayout;
  class EI_BindSet;
  class EI_PSO;
  class EI_IndexBuffer;
  */

  // Main API:
  class TFxSample {
    public:
    TFxSample(GlobalState*);
    ~TFxSample();

    // has hardcoded list of resources. other name: load_tfx_assets
    void init();

    // render
    void draw_hair();
    // void draw_hair_shadows();

    // simulation
    void simulate(double fTime);
    void wait_simulate_done();
    // void draw_collision_mesh();


    private:

    // values from GUI etc.
    GlobalState* app_state = nullptr;

    // hair strands collection - important!
    std::vector<TFxHairStrands*> m_hairStrands;

    // OIT
    TFxPPLL* m_pPPLL = nullptr;
    // TFxShortCut* m_pShortCut = nullptr;
    // Views for back and depth buffer created in Normal.lua
    // SuGPURenderableResourceViewPtr   m_pShortCutBackBufferView;
    // SuGPUDepthStencilResourceViewPtr m_pShortCutDepthBufferView;

    // Sim
    TFxSimulation* m_pSimulation = nullptr;

    // size of PPLL SSBO: PPLLNode[m_nPPLLNodes]
    int m_nPPLLNodes = 0;

    // shaders
    // SuEffectPtr m_pHairStrandsEffect = nullptr; // oHair.sufx
    // (fill linked list)
    glUtils::Shader m_shaderPPLL_build;
    // SuEffectPtr m_pHairResolveEffect = nullptr; // qHair.sufx
    // (render fullscreen quad)
    glUtils::Shader m_shaderPPLL_resolve;

    // SuCountedPtr<SuGPUResource> m_pHairColorTexture;

    // Impl:
    void load_shaders(); // load_effects
    void unload_shaders(); // unload_effects
    void initialize_layouts();
    void destroy_layouts();
  };

} // namespace glTFx
