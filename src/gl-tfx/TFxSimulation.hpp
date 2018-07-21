#pragma once

#include <vector>
#include "include_TFx_pls_no_warnings.hpp"
#include "../../libs/amd_tressfx/include/AMD_TressFX.h" // AMD forgot to import
// #include "../../libs/amd_tressfx/include/TressFXSDFCollision.h"
#include "../../libs/amd_tressfx/include/TressFXSimulation.h"

// lib forwards
// -

struct GlobalState; // global state from GUI

namespace glTFx {

  class TFxHairStrands;

  class TFxSimulation {
    public:
    TFxSimulation();
    ~TFxSimulation();

    void start_simulation(double fTime, std::vector<TFxHairStrands*>&);
    void wait_on_simulation();


    private:

    // original AMD lib
    TressFXSimulation mSimulation;

    // TODO pack in struct SimulationShaders
    glUtils::Shader shader_sim0_IntegrationAndGlobalShapeConstraints;
    glUtils::Shader shader_sim1_VelocityShockPropagation;
    glUtils::Shader shader_sim2_LocalShapeConstraints;
    glUtils::Shader shader_sim3_LengthConstraintsWindAndCollision;
    glUtils::Shader shader_sim4_UpdateFollowHairVertices;

  };

} // namespace glTFx
