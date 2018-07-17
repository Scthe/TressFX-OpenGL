#pragma once

#include "include_TFx_pls_no_warnings.hpp"
#include "../../libs/amd_tressfx/include/AMD_TressFX.h"

#include <vector>
#include "GpuInterface/TFxGpuInterface.hpp"

// #include "SuTypes.h"
// #include "SuArray.h"

// lib forwards
class TressFXPPLL;
// class SuEffect;
// class EI_PSO;


namespace glTFx {

  class TFxHairStrands;

  class TFxPPLL {
    public:
    void initialize(int width, int height,
      int nNodes, int nodeSize,
      glUtils::Shader* pStrandEffect, glUtils::Shader* pQuadEffect);
    void shutdown(EI_Device* pDevice);

    void draw(std::vector<TFxHairStrands*>& hairStrands, u32 nodePoolSize);

    private:
    TressFXPPLL* m_pPPLL = nullptr; // AMD lib

    /** shader + params used for building  PPLL (pass 1)*/
    EI_PSO m_pBuildPSO;
    /** shader + params for fullscreen pass (pass 2)*/
    EI_PSO m_pReadPSO;
  };

} // namespace glTFx
