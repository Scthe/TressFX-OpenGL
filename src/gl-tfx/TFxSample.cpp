#include "../../include/pch.hpp"
#include "TFxSample.hpp"

#include "../../libs/amd_tressfx/include/AMD_TressFX.h"
#include "../../libs/amd_tressfx/include/TressFXLayouts.h"

#include "TFxPPLL.hpp"
// #include "TFxShortcut.hpp"
#include "TFxHairStrands.hpp"
#include "TFxSimulation.hpp"
#include "GpuInterface/TFxGpuInterface.hpp"
#include "../State.hpp"

// #include "SuLog.h"
// #include "SuString.h"
// #include "SuTypes.h"
// #include "SuEffectManager.h"
// #include "SuEffectTechnique.h"
// #include "SuGPUResourceManager.h"
// #include "SuObjectManager.h"

// #include "SushiGPUInterface.h"


static auto PPLL_BUILD_VS_PATH = "ppll_build.vert.glsl";
static auto PPLL_BUILD_FS_PATH = "ppll_build.frag.glsl";
static auto PPLL_RESOLVE_VS_PATH = "ppll_resolve.vert.glsl";
static auto PPLL_RESOLVE_FS_PATH = "ppll_resolve.frag.glsl";



// TODO move from here?
static const size_t AVE_FRAGS_PER_PIXEL = 4;
static const size_t PPLL_NODE_SIZE = 16;

// See TressFXLayouts.h
// By default, app allocates space for each of these, and TressFX uses it.
// These are globals, because there should really just be one instance.
// NOTE: it is used even in AMD::tfx library
TressFXLayouts* g_TressFXLayouts = nullptr;


namespace glTFx {

  TFxSample::TFxSample(GlobalState* st) : app_state(st){
    init_TFx_callbacks();
  }

  TFxSample::~TFxSample() {
    auto pDevice = GetDevice();

    for (size_t i = 0; i < m_hairStrands.size(); i++) {
      m_hairStrands[i]->destroy(pDevice);
    }

    if (m_pPPLL) {
      m_pPPLL->shutdown(pDevice);
      delete m_pPPLL;
    }
    // if (m_pShortCut) {
        // m_pShortCut->shutdown(pDevice);
        // delete m_pShortCut;
    // }
    // m_pShortCutBackBufferView = SuGPURenderableResourceViewPtr(0);
    // m_pShortCutDepthBufferView = SuGPUDepthStencilResourceViewPtr(0);

    if (m_pSimulation) {
      delete m_pSimulation;
    }

    this->destroy_layouts();
    this->unload_shaders();
  }

  void TFxSample::init() {
    GetContext().state = app_state;

    this->destroy_layouts();

    this->load_shaders();
    this->initialize_layouts();

    auto pDevice = GetDevice();
    const auto& tfx_settings = app_state->tfx_settings;

    if (tfx_settings.use_shortcut) {
      GFX_FAIL("Shortcut is not implemented yet");
      // m_pShortCut = new TFxShortCut();
      // m_pShortCut->shutdown(pDevice);
      // m_pShortCut->initialize(app_state->win_width, app_state->win_height/*,
          // m_pHairStrandsEffect.get(), m_pHairResolveEffect.get()*/);
    } else {
      m_pPPLL = new TFxPPLL();
      m_nPPLLNodes = app_state->win_width * app_state->win_height * AVE_FRAGS_PER_PIXEL;
      m_pPPLL->shutdown(pDevice);
      m_pPPLL->initialize(app_state->win_width, app_state->win_height,
          m_nPPLLNodes, PPLL_NODE_SIZE,
          &m_shaderPPLL_build, &m_shaderPPLL_resolve);
    }

    m_pSimulation = new TFxSimulation;

    // load hair strands
    m_hairStrands.push_back(new TFxHairStrands);
    m_hairStrands[0]->initialize(
      tfx_settings.object_name,
      tfx_settings.filepath,
      tfx_settings.follow_hairs_per_guide_hair,
      tfx_settings.simulation_settings.m_tipSeparation
    );
  }

  // <editor-fold draw>
  void TFxSample::draw_hair() {
    // Set shader constants for fragment buffer size
    // float4 vFragmentBufferSize;
    // vFragmentBufferSize.x = (float)app_state->win_width;
    // vFragmentBufferSize.y = (float)app_state->win_height;
    // vFragmentBufferSize.z = (float)vFragmentBufferSize.x * vFragmentBufferSize.y;
    // vFragmentBufferSize.w = 0;

    // LOGW << "Skipping setting shader parameters, cause shaders not ready";
    // m_pHairStrandsEffect->GetParameter("nNodePoolSize")->SetInt(m_nPPLLNodes);
    // m_pHairResolveEffect->GetParameter("nNodePoolSize")->SetInt(m_nPPLLNodes);
    // m_pHairStrandsEffect->GetParameter("vFragmentBufferSize")->SetFloatVector(&vFragmentBufferSize.x);
    // m_pHairResolveEffect->GetParameter("vFragmentBufferSize")->SetFloatVector(&vFragmentBufferSize.x);

    if (app_state->tfx_settings.use_shortcut) {
      GFX_FAIL("Shortcut is not implemented yet");
      // m_pShortCut->draw(m_hairStrands);
    } else {
      m_pPPLL->draw(m_hairStrands, m_nPPLLNodes);
    }
  }

  /*
  void TFxSample::draw_hair_shadows() {
    EI_CommandContextRef pRenderCommandList = (EI_CommandContextRef)SuRenderManager::GetRef().GetCurrentCommandList();
    EI_PSO* pso = GetPSOPtr(m_pHairStrandsEffect->GetTechnique("DepthOnly"));
    SU_ASSERT(pso != nullptr);

    for (size_t i = 0; i < m_hairStrands.size(); i++) {
      TressFXHairObject* pHair = m_hairStrands[i]->GetTressFXHandle();

      if (pHair) {
        pHair->DrawStrands(pRenderCommandList, *pso);
      }
    }
  }
  */
  // </editor-fold>


  // <editor-fold simulation>
  void TFxSample::simulate(double fTime) {
    // TODO provide sim_parameters?
    // hairMohawk->GetTressFXHandle()->UpdateSimulationParameters(mohawkParameters);
    // hairShort->GetTressFXHandle()->UpdateSimulationParameters(shortHairParameters);
    m_pSimulation->start_simulation(fTime, m_hairStrands);
  }

  void TFxSample::wait_simulate_done() {
    m_pSimulation->wait_on_simulation();
  }

  // void TressFXSample::draw_collision_mesh() {
    // Just draw collision spheres/meshes
  // }
  // </editor-fold>

  // <editor-fold layouts>
  void TFxSample::initialize_layouts() {
    // See TressFXLayouts.h
    // Global storage for layouts.

    if (g_TressFXLayouts == nullptr){
      g_TressFXLayouts = new TressFXLayouts;
    }

    EI_Device* pDevice = GetDevice(); //SuGPUResourceManager::GetPtr();

    // Each of these will call g_callback: CreateLayout (TressFXLayoutDescription)
    // EI_LayoutManagerRef renderStrandsLayoutManager = GetLayoutManagerRef(m_pHairStrandsEffect);
    EI_LayoutManager renderStrandsLayoutManager;
    renderStrandsLayoutManager.shaders.push_back({&this->m_shaderPPLL_build, PPLL_BUILD_VS_PATH, PPLL_BUILD_FS_PATH});
    CreateRenderPosTanLayout2(pDevice, renderStrandsLayoutManager);
    CreateRenderLayout2(pDevice, renderStrandsLayoutManager);
    CreatePPLLBuildLayout2(pDevice, renderStrandsLayoutManager);
    CreateShortCutDepthsAlphaLayout2(pDevice, renderStrandsLayoutManager);
    CreateShortCutFillColorsLayout2(pDevice, renderStrandsLayoutManager);

    // EI_LayoutManagerRef readLayoutManager = GetLayoutManagerRef(m_pHairResolveEffect);
    EI_LayoutManager readLayoutManager;
    readLayoutManager.shaders.push_back({&this->m_shaderPPLL_resolve, PPLL_RESOLVE_VS_PATH, PPLL_RESOLVE_FS_PATH});
    CreatePPLLReadLayout2(pDevice, readLayoutManager);
    CreateShortCutResolveDepthLayout2(pDevice, readLayoutManager);
    CreateShortCutResolveColorLayout2(pDevice, readLayoutManager);
  }

  void TFxSample::destroy_layouts() {
    if (g_TressFXLayouts != nullptr) {
      EI_Device* pDevice = GetDevice(); //(EI_Device*)SuGPUResourceManager::GetPtr();
      DestroyAllLayouts(pDevice); // AMD lib function

      delete g_TressFXLayouts;
    }
  }
  // </editor-fold>

  // <editor-fold effects>
  void TFxSample::load_shaders() {
    glUtils::ShaderTexts build_paths;
    build_paths.vertex = PPLL_BUILD_VS_PATH;
    build_paths.fragment = PPLL_BUILD_FS_PATH;
    load_tfx_shader(this->m_shaderPPLL_build, build_paths);

    glUtils::ShaderTexts resolve_paths;
    resolve_paths.vertex = PPLL_RESOLVE_VS_PATH;
    resolve_paths.fragment = PPLL_RESOLVE_FS_PATH;
    load_tfx_shader(this->m_shaderPPLL_resolve, resolve_paths);
  }

  void TFxSample::unload_shaders() {
    glUtils::destroy(this->m_shaderPPLL_build);
    glUtils::destroy(this->m_shaderPPLL_resolve);
  }
  // </editor-fold>


} // namespace glTFx
