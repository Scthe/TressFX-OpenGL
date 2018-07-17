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
#include "../../libs/amd_tressfx/include/TressFXSDFCollision.h"
#include "../../libs/amd_tressfx/include/TressFXSettings.h"

#include <vector>

// lib forwards
// -

struct GlobalState; // global state from GUI

namespace glTFx {

  class TFxPPLL;
  class TFxShortCut;
  class TFxHairStrands;
  // class Simulation;

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

    void draw_hair();
    // void draw_hair_shadows();

    // int       GetNumTressFXObjects() { return static_cast<int>(m_hairStrands.size()); }
    // SuObject* GetTressFXObject(int index);


    private:
    // values from GUI etc.
    GlobalState* app_state = nullptr;

    // hair strands collection - important!
    std::vector<TFxHairStrands*> m_hairStrands;

    TFxPPLL* m_pPPLL = nullptr;
    TFxShortCut* m_pShortCut = nullptr;
    // Simulation* m_pSimulation;

    // Views for back and depth buffer created in Normal.lua
    // SuGPURenderableResourceViewPtr   m_pShortCutBackBufferView;
    // SuGPUDepthStencilResourceViewPtr m_pShortCutDepthBufferView;

    int m_nPPLLNodes = 0;

    // Need to handle these better.
    void load_shaders(); // load_effects
    void unload_shaders(); // unload_effects
    // SuEffectPtr m_pHairStrandsEffect = nullptr; // oHair.sufx
    // (fill linked list)
    glUtils::Shader m_shaderPPLL_build;
    // SuEffectPtr m_pHairResolveEffect = nullptr; // qHair.sufx
    // (render fullscreen quad)
    glUtils::Shader m_shaderPPLL_resolve;


    // SuCountedPtr<SuGPUResource> m_pHairColorTexture;

    void initialize_layouts();
    void destroy_layouts();

  };

} // namespace glTfx
