#pragma once

#include "include_TFx_pls_no_warnings.hpp"
#include "../../libs/amd_tressfx/include/AMD_TressFX.h"

// #include "SuString.h"
// #include "SuTypes.h"
// #include "SuObjectManager.h"

// class TressFXSimulation;

namespace glTFx {

  class TFxHairStrands {
    public:
    void initialize(
        const char*const modelName,
        const char*const hairObjectName,
        const char*const tfxFilePath,
        // const char*const tfxboneFilePath,
        int numFollowHairsPerGuideHair,
        float tipSeparationFactor);

    void destroy(EI_Device* pDevice);

    TressFXHairHandle get_AMDTressFXHandle() { return m_pStrands; }


    void TransitionSimToRendering(EI_CommandContextRef context);
    void TransitionRenderingToSim(EI_CommandContextRef context);
    // void UpdateBones(EI_CommandContextRef context);
    // void Simulate(EI_CommandContextRef context, TressFXSimulation* pSimulation);


    private:
    TressFXHairObject* m_pStrands = nullptr;
    // SuAnimatedModel* m_pSkeleton;
  };

} // namespace glTFx
