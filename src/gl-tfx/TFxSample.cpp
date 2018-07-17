#include "../../include/pch.hpp"
#include "TFxSample.hpp"

#include "../../libs/amd_tressfx/include/AMD_TressFX.h"
#include "../../libs/amd_tressfx/include/TressFXLayouts.h"

#include "TFxPPLL.hpp"
// #include "TFxShortcut.hpp"
#include "TFxHairStrands.hpp"
#include "GpuInterface/TFxGpuInterface.hpp"
#include "../State.hpp"

// shader load libs:
#include <fstream>
#include <cerrno>

// #include "SuLog.h"
// #include "SuString.h"
// #include "SuTypes.h"
// #include "SuEffectManager.h"
// #include "SuEffectTechnique.h"
// #include "SuGPUResourceManager.h"
// #include "SuObjectManager.h"

// #include "SushiGPUInterface.h"


static std::string SHADER_PATH = "src/shaders/generated/gl-tfx/";
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

    this->destroy_layouts();
    this->unload_shaders();
  }

  void TFxSample::init() {
    GetContext().state = app_state;

    this->destroy_layouts();

    this->load_shaders();
    this->initialize_layouts();

    auto pDevice = GetDevice();

    if (app_state->use_shortcut) {
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

    // load hair strands
    m_hairStrands.push_back(new TFxHairStrands);
    m_hairStrands[0]->initialize(
      "RatBoy_body", "mohawk",
      "assets\\Ratboy\\Ratboy_mohawk.tfx",
      // "Objects\\HairAsset\\Ratboy\\Ratboy_mohawk.tfxbone",
      2, // This is number of follow hairs per one guide hair. It could be zero if there is no follow hair at all.
      2.0f
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

    if (app_state->use_shortcut) {
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
    EI_LayoutManager renderStrandsLayoutManager = {&this->m_shaderPPLL_build, PPLL_BUILD_VS_PATH, PPLL_BUILD_FS_PATH};
    CreateRenderPosTanLayout2(pDevice, renderStrandsLayoutManager);
    CreateRenderLayout2(pDevice, renderStrandsLayoutManager);
    CreatePPLLBuildLayout2(pDevice, renderStrandsLayoutManager);
    CreateShortCutDepthsAlphaLayout2(pDevice, renderStrandsLayoutManager);
    CreateShortCutFillColorsLayout2(pDevice, renderStrandsLayoutManager);

    // EI_LayoutManagerRef readLayoutManager = GetLayoutManagerRef(m_pHairResolveEffect);
    EI_LayoutManager readLayoutManager = {&this->m_shaderPPLL_resolve, PPLL_RESOLVE_VS_PATH, PPLL_RESOLVE_FS_PATH};
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
  static std::string get_file_contents(const char *filename) {
    std::string full_path = SHADER_PATH + filename;
    std::ifstream in(full_path, std::ios::in | std::ios::binary);
    if (in) {
      std::string contents;
      in.seekg(0, std::ios::end);
      contents.resize(in.tellg());
      in.seekg(0, std::ios::beg);
      in.read(&contents[0], contents.size());
      in.close();
      return contents;
    } else {
      LOGE << "Error reading file content: '" << filename << "'";
      throw(errno);
    }
  }

  static void create_shader (glUtils::Shader& shader, const std::string& f_vs, const std::string& f_ps) {
    glUtils::ShaderTexts shader_texts;
    auto vs = get_file_contents(f_vs.c_str());
    auto fs = get_file_contents(f_ps.c_str());
    shader_texts.vertex = vs.c_str();
    shader_texts.fragment = fs.c_str();

    LOGT << "Compiling shader from files: vs='" << f_vs << "', ps='" << f_ps << "'";

    glUtils::ShaderErrorsScratchpad es;
    shader = glUtils::create_shader(shader_texts, es);
    if (!shader.is_created()) {
      LOGE << "Shader create error: " << es.msg;
      GFX_FAIL("Could not create TFx shader [",
        f_vs.c_str(), ", ", f_ps.c_str(),
      "]");
    }
  }

  void TFxSample::load_shaders() {
    create_shader(this->m_shaderPPLL_build, PPLL_BUILD_VS_PATH, PPLL_BUILD_FS_PATH);
    create_shader(this->m_shaderPPLL_resolve, PPLL_RESOLVE_VS_PATH, PPLL_RESOLVE_FS_PATH);
    // m_pHairStrandsEffect = SuEffectManager::GetRef().LoadEffect("oHair.sufx");
    // m_pHairResolveEffect = SuEffectManager::GetRef().LoadEffect("qHair.sufx");
  }

  void TFxSample::unload_shaders() {
    glUtils::destroy(this->m_shaderPPLL_build);
    glUtils::destroy(this->m_shaderPPLL_resolve);
    // m_pHairStrandsEffect = SuEffectPtr(0);
    // m_pHairResolveEffect = SuEffectPtr(0);
  }
  // </editor-fold>


} // namespace glTFx
