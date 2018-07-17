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

#include <glm/gtc/matrix_transform.hpp> // TODO remove

namespace glTFx {

  void TFxPPLL::initialize(int width, int height,
    int nNodes, int nodeSize,
    glUtils::Shader* pStrandEffect, glUtils::Shader* pQuadEffect)
  {
    // see oHair.sufx
    m_pBuildPSO.shader = pStrandEffect;
    m_pBuildPSO.draw_params.depth.write = false;
    // m_pBuildPSO.draw_params.depth.test = glUtils::DepthTest::IfLess;
    m_pBuildPSO.draw_params.depth.test = glUtils::DepthTest::AlwaysPass;
    // m_pBuildPSO.draw_params.polygon_mode = glUtils::PolygonMode::Point;
    // m_pBuildPSO.draw_params.polygon_mode = glUtils::PolygonMode::Line;
    // m_pBuildPSO.draw_params.point_size = 3.0f;
    m_pBuildPSO.draw_params.backface_culling = glUtils::BackfaceCullingMode::CullingDisabled;
    // m_pBuildPSO.draw_params.color_write[0] = false;
    m_pBuildPSO.draw_params.color_write[1] = false;
    m_pBuildPSO.draw_params.color_write[2] = false;
    m_pBuildPSO.draw_params.color_write[3] = false;
    m_pBuildPSO.draw_params.stencil_counter_clockwise.op_pass = glUtils::StencilOperation::Increment; // front
    m_pBuildPSO.draw_params.stencil_clockwise.op_pass = glUtils::StencilOperation::Increment; // back

    // see qHair.sufx
    m_pReadPSO.shader = pQuadEffect;
    m_pReadPSO.draw_params.backface_culling = glUtils::BackfaceCullingMode::CullingDisabled;
    m_pReadPSO.draw_params.depth.write = false;
    m_pReadPSO.draw_params.depth.test = glUtils::DepthTest::AlwaysPass;
    m_pReadPSO.draw_params.color_write[0] = true;
    m_pReadPSO.draw_params.color_write[1] = true;
    m_pReadPSO.draw_params.color_write[2] = true;
    m_pReadPSO.draw_params.color_write[3] = true;
    m_pReadPSO.draw_params.stencil_counter_clockwise.write_bytes = 0; // front
    m_pReadPSO.draw_params.stencil_clockwise.write_bytes = 0; // back
    // we incr in 1st pass, so if ref is 0 then any pixel that was touched will have value >ref
    // OTOH any NOT touched in 1st pass will still have 0, which is equall to ref
    // m_pReadPSO.draw_params.stencil_counter_clockwise.test = glUtils::StencilTest::IfRefIsLessThenCurrent;
    // m_pReadPSO.draw_params.stencil_clockwise.test = glUtils::StencilTest::IfRefIsLessThenCurrent;
    // TODO blending

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

  void TFxPPLL::draw(std::vector<TFxHairStrands*>& hairStrands, u32 nodePoolSize) {
    // LOGE << "START draw_hair";
    EI_CommandContextRef commandContext = GetContext();

    // build ppll
    glUseProgram(m_pBuildPSO.shader->gl_id);
    commandContext.state->update_draw_params(m_pBuildPSO.draw_params);
    m_pPPLL->Clear(commandContext);
    m_pPPLL->BindForBuild(commandContext);  // put clear in here?
    {
      commandContext.state->update_draw_params(m_pBuildPSO.draw_params); // TODO remove from draw, and apply just here
      // uniform int nNodePoolSize; // width * height * AVE_FRAGS_PER_PIXEL(4)
      glUtils::set_uniform(*m_pBuildPSO.shader, "nNodePoolSize", (int)nodePoolSize, true);
      // uniform mat4 g_mVP;
      glm::mat4 m(1);
      float sc = 0.01;
      m = glm::scale(m, glm::vec3(sc,sc,sc));
      // auto v = glm::translate(view, {0, 0, -state.camera_distance});
      auto v = commandContext.state->camera.view;
      auto p = commandContext.state->camera.projection;
      glm::mat4 mvp = p * v * m;
      glUtils::set_uniform(*m_pBuildPSO.shader, "g_mVP", mvp, false, true);
      // uniform vec3 g_vEye;
      auto mv = v * m;
      glm::vec3 cam_pos = { mv[3][0], mv[3][1], mv[3][2]};
      glUtils::set_uniform(*m_pBuildPSO.shader, "g_vEye", cam_pos, true);
      // uniform float g_FiberRadius; // size of strand
      glUtils::set_uniform(*m_pBuildPSO.shader, "g_FiberRadius", 0.05f, true);
    }
    LOGD << "PPLL pass1 preparations complete, will draw " << hairStrands.size() << " strands";
    for (size_t i = 0; i < hairStrands.size(); i++) {
      TressFXHairObject* pHair = hairStrands[i]->get_AMDTressFXHandle();
      if (pHair) {
        pHair->DrawStrands(commandContext, m_pBuildPSO);
      }
    }
    m_pPPLL->DoneBuilding(commandContext);

    LOGI << "PPLL pass1 complete, will do pass2 (fullscreen pass)";

    // build ppll cleanup
    // TODO move this to a clear "after all pos and tan usage by rendering" place.
    // for (size_t i = 0; i < hairStrands.size(); i++){
      // hairStrands[i]->TransitionRenderingToSim(commandContext);
    // }
    // LOGW << "TransitionRenderingToSim executed, where it is reversed?";

    // draw ppll
    glUseProgram(m_pReadPSO.shader->gl_id);
    m_pPPLL->BindForRead(commandContext);
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
