#pragma once

#include "include_TFx_pls_no_warnings.hpp"
#include "../../libs/amd_tressfx/include/AMD_TressFX.h"

// #include "SuString.h"
// #include "SuTypes.h"
// #include "SuObjectManager.h"

class TressFXSimulation;

namespace glTFx {

  class TFxHairStrands {
    public:

    void initialize(
        const char*const hairObjectName,
        const char*const tfxFilePath,
        // const char*const tfxboneFilePath,
        int numFollowHairsPerGuideHair,
        float tipSeparationFactor);
    void destroy(EI_Device* pDevice);

    void TransitionSimToRendering(EI_CommandContextRef);
    void TransitionRenderingToSim(EI_CommandContextRef);
    // void update_bones(EI_CommandContextRef);
    void simulate(EI_CommandContextRef, TressFXSimulation*);

    TressFXHairHandle get_AMDTressFXHandle() { return m_pStrands; }

    private:
    TressFXHairObject* m_pStrands = nullptr;
  };

} // namespace glTFx
