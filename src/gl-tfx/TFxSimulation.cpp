#include "../../include/pch.hpp"
#include "TFxSimulation.hpp"

#include "../../libs/amd_tressfx/include/TressFXLayouts.h"
#include "TFxHairStrands.hpp"
#include "GpuInterface/TFxGpuInterface.hpp"
// #include "SDF.h"
#include "../State.hpp"

// #include "SushiGPUInterface.h"
// #include "SuEffectManager.h"


static auto SIM_0_SHADER_PATH = "sim0_IntegrationAndGlobalShapeConstraints.comp.glsl";
static auto SIM_1_SHADER_PATH = "sim1_VelocityShockPropagation.comp.glsl";
static auto SIM_2_SHADER_PATH = "sim2_LocalShapeConstraints.comp.glsl";
static auto SIM_3_SHADER_PATH = "sim3_LengthConstraintsWindAndCollision.comp.glsl";
static auto SIM_4_SHADER_PATH = "sim4_UpdateFollowHairVertices.comp.glsl";

static void load_compute_shader (EI_LayoutManager& slm, const char*const path, glUtils::Shader* shader) {
  glUtils::ShaderTexts st;
  st.compute = path;
  glTFx::load_tfx_shader(*shader, st);
  slm.shaders.push_back({shader, path, ""});
}

namespace glTFx {

  TFxSimulation::TFxSimulation() {
    EI_Device* pDevice = (EI_Device*)GetDevice();

    // load shaders
    EI_LayoutManager slm;
    load_compute_shader(slm, SIM_0_SHADER_PATH, &shader_sim0_IntegrationAndGlobalShapeConstraints);
    load_compute_shader(slm, SIM_1_SHADER_PATH, &shader_sim1_VelocityShockPropagation);
    load_compute_shader(slm, SIM_2_SHADER_PATH, &shader_sim2_LocalShapeConstraints);
    load_compute_shader(slm, SIM_3_SHADER_PATH, &shader_sim3_LengthConstraintsWindAndCollision);
    load_compute_shader(slm, SIM_4_SHADER_PATH, &shader_sim4_UpdateFollowHairVertices);

    // EI_LayoutManagerRef simLayoutManager = GetLayoutManagerRef(m_pTressFXSimEffect);
    // EI_LayoutManager simLayoutManager = {&this->m_shaderPPLL_resolve, PPLL_RESOLVE_VS_PATH, PPLL_RESOLVE_FS_PATH};
    CreateSimPosTanLayout2(pDevice, slm);
    CreateSimLayout2(pDevice, slm);

    // init AMD lib
    mSimulation.Initialize(pDevice, slm);
  }

  TFxSimulation::~TFxSimulation() {
    EI_Device* pDevice = GetDevice();

    mSimulation.Shutdown(pDevice);

    glUtils::destroy(shader_sim0_IntegrationAndGlobalShapeConstraints);
    glUtils::destroy(shader_sim1_VelocityShockPropagation);
    glUtils::destroy(shader_sim2_LocalShapeConstraints);
    glUtils::destroy(shader_sim3_LengthConstraintsWindAndCollision);
    glUtils::destroy(shader_sim4_UpdateFollowHairVertices);
  }

  void TFxSimulation::start_simulation(double fTime, std::vector<TFxHairStrands*>& hairStrands) {
    // SuCommandListPtr pSimCommandList = m_ComputeQueue.GetComputeCommandList();
    // EI_CommandContextRef simContext = (EI_CommandContextRef)pSimCommandList;
    EI_CommandContextRef simContext = GetContext();

    // run
    for (size_t i = 0; i < hairStrands.size(); i++) {
      simContext.simulated_hair_object = hairStrands[i]->get_AMDTressFXHandle();

      // update bone matrices for bone skinning of first two vertices of hair strands
      // hairStrands[i]->UpdateBones(simContext);

      // Run simulation
      hairStrands[i]->simulate(simContext, &mSimulation);
    }

    // finalize
    for (size_t i = 0; i < hairStrands.size(); i++){
      hairStrands[i]->TransitionSimToRendering(simContext);
    }

    // Have compute work wait for signal from graphics queue that we can start
    // issuing the sim commands.
    // m_ComputeQueue.WaitForLastFrameHairRenders();
    // m_ComputeQueue.SubmitSimCommandList(pSimCommandList);
    // m_ComputeQueue.RestoreRenderAsDefaultQueue(pRenderCommandList);
  }

  void TFxSimulation::wait_on_simulation() {
    // m_ComputeQueue.WaitForCompute();
  }

} // namespace glTFx
