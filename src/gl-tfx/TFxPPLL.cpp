#include "../../include/pch.hpp"
#include "TFxPPLL.hpp"
#include "../../libs/amd_tressfx/include/TressFXLayouts.h"
#include "../../libs/amd_tressfx/include/TressFXPPLL.h"
#include "../../libs/amd_tressfx/include/TressFXHairObject.h"
#include "TFxHairStrands.hpp"

#include "GpuInterface/TFxGpuInterface.hpp"
#include "../State.hpp"
// #include "SushiGPUInterface.h"
// #include "SushiUtils.h"

namespace glTFx {

  void TFxPPLL::initialize(int width, int height,
    int nNodes, int nodeSize,
    glUtils::Shader* pStrandEffect, glUtils::Shader* pQuadEffect)
  {
    // see oHair.sufx
    m_pBuildPSO.shader = pStrandEffect;
    auto& dp0 = m_pBuildPSO.draw_params;
    dp0.depth.write = false;
    dp0.depth.test = glUtils::DepthTest::IfLess;
    // dp0.depth.test = glUtils::DepthTest::AlwaysPass;
    dp0.culling = glUtils::CullingMode::None;
    dp0.color_write[0] = false;
    dp0.color_write[1] = false;
    dp0.color_write[2] = false;
    dp0.color_write[3] = false;
    // dp0.polygon_mode = glUtils::PolygonMode::Line;
    dp0.stencil.front.op_pass = glUtils::StencilOperation::Increment;
    dp0.stencil.back.op_pass = glUtils::StencilOperation::Increment;

    // see qHair.sufx
    m_pReadPSO.shader = pQuadEffect;
    auto& dp1 = m_pReadPSO.draw_params;
    dp1.depth.write = false;
    dp1.depth.test = glUtils::DepthTest::AlwaysPass;
    dp1.culling = glUtils::CullingMode::None;
    dp1.blend.color.function = glUtils::BlendingFunction::Addition;
    dp1.blend.color.new_value_factor = glUtils::BlendingFactor::One; // src
    dp1.blend.color.current_value_factor = glUtils::BlendingFactor::SourceAlpha; // dest
    dp1.blend.alpha.function = glUtils::BlendingFunction::Addition;
    dp1.blend.alpha.new_value_factor = glUtils::BlendingFactor::Zero; // src
    dp1.blend.alpha.current_value_factor = glUtils::BlendingFactor::Zero; // dest
    // we incr in 1st pass, so if ref is 0 then any pixel that was touched will have value >ref
    // OTOH any NOT touched in 1st pass will still have 0, which is equall to ref
    dp1.stencil.front.test = glUtils::StencilTest::IfRefIsLessThenCurrent;
    dp1.stencil.back.test = glUtils::StencilTest::IfRefIsLessThenCurrent;

    // done
    m_pPPLL = new TressFXPPLL;
    m_pPPLL->Create(GetDevice(), width, height, nNodes, nodeSize);
  }

  void TFxPPLL::shutdown(EI_Device* pDevice) {
    if (m_pPPLL) {
      m_pPPLL->Destroy(pDevice);
      delete m_pPPLL;
      m_pPPLL = nullptr;
    }
  }

  static void update_uniforms_build (glUtils::Shader& shader, GlobalState& state, u32 nodePoolSize) {
    auto& settings = state.tfx_settings;
    auto& camera = state.camera;

    glUtils::set_uniform(shader, "nNodePoolSize", (int)nodePoolSize, true);
    glUtils::set_uniform(shader, "g_bThinTip", static_cast<i32>(settings.use_thin_tip), true);
    glUtils::set_uniform(shader, "g_Ratio", static_cast<f32>(settings.hair_thickness_at_tip_ratio), true);
    glUtils::set_uniform(shader, "g_FiberRadius", static_cast<f32>(settings.hair_thickness), true);
    glUtils::set_uniform(shader, "g_MatBaseColor", static_cast<glm::vec4>(settings.root_color), true);
    glm::vec4 tip_color = settings.use_separate_tip_color ? settings.tip_color : settings.root_color;
    glUtils::set_uniform(shader, "g_MatTipColor", static_cast<glm::vec4>(tip_color), true);
    glUtils::set_uniform(shader, "g_ColorMidAlpha", settings.mid_alpha, true);
    glUtils::set_uniform(shader, "g_ColorShiftScale", settings.strand_hue_rand_scale, true);
    glUtils::set_uniform(shader, "g_WinSize", glm::vec2(state.win_width, state.win_height), true);
    glUtils::set_uniform(shader, "g_vEye", state.camera.get_position(), true);
    glm::mat4 mvp = camera.projection * camera.view * settings.model_matrix;
    glUtils::set_uniform(shader, "g_mVP", mvp, false, true);
  }

  static void update_uniforms_resolve (glUtils::Shader& shader, GlobalState& state) {
    auto& settings = state.tfx_settings;
    // auto& camera = state.camera;
    glUtils::set_uniform(shader, "g_RenderMode", (int)settings.render_mode);
  }

  void TFxPPLL::draw(std::vector<TFxHairStrands*>& hairStrands, u32 nodePoolSize) {
    // LOGE << "START draw_hair";
    EI_CommandContextRef commandContext = GetContext();
    GFX_FAIL_IF(!commandContext.state, "EI_CommandContext should have been initialized"
        "with ptr to GlobalState before TFxPPLL::draw was called");
    GFX_FAIL_IF(!m_pBuildPSO.shader, "m_pBuildPSO should have been initialized"
        "with ptr to shader before TFxPPLL::draw was called");
    auto& shader0 = *m_pBuildPSO.shader;

    // build ppll
    glUseProgram(shader0.gl_id);
    commandContext.state->update_draw_params(m_pBuildPSO.draw_params);
    m_pPPLL->Clear(commandContext);
    m_pPPLL->BindForBuild(commandContext);
    update_uniforms_build(shader0, *commandContext.state, nodePoolSize);

    LOGD << "PPLL pass1 preparations complete, will draw " << hairStrands.size() << " strands";
    for (size_t i = 0; i < hairStrands.size(); i++) {
      TressFXHairObject* pHair = hairStrands[i]->get_AMDTressFXHandle();
      if (pHair) {
        glUtils::set_uniform(shader0, "g_NumVerticesPerStrand", pHair->GetNumVerticesPerStrand());
        pHair->DrawStrands(commandContext, m_pBuildPSO);
      }
    }
    m_pPPLL->DoneBuilding(commandContext);

    LOGD << "PPLL pass1 complete, will do pass2 (fullscreen pass)";

    // build ppll cleanup
    for (size_t i = 0; i < hairStrands.size(); i++){
      hairStrands[i]->TransitionRenderingToSim(commandContext);
    }

    // draw ppll
    auto& shader1 = *m_pReadPSO.shader;
    glUseProgram(shader1.gl_id);
    m_pPPLL->BindForRead(commandContext);
    update_uniforms_resolve(shader1, *commandContext.state);

    if (hairStrands.size() > 0) {
      commandContext.state->update_draw_params(m_pReadPSO.draw_params);
      // glBindVertexArray(0); // WHATEVER, WE WILL NOT USE IT ANYWAY!
      LOGI << "[drawFullscreenQuad]";
      glDrawArrays(GL_TRIANGLES, 0, 6); // m_pFullscreenPass->Draw(m_pReadPSO);
    }
    m_pPPLL->DoneReading(commandContext);

    // LOGE << "END draw_hair";
  }

} // namespace glTFx
