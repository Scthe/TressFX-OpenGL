#pragma once

#include "include_TFx_pls_no_warnings.hpp"
#include "../../libs/amd_tressfx/include/TressFXSettings.h"


namespace glTFx {

  struct TFxRenderingMode {
    static const i32 Normal = 0;
    static const i32 BinaryPPLL = 1; // fill single color
    static const i32 PPLLOverlap = 2; // blue - 1 overlap, red - 3 overlaps etc.
  };

  struct TFxSettings {

    //
    // model
    const char*const object_name = "mohawk";
    const char*const filepath = "assets\\Ratboy\\Ratboy_mohawk.tfx";
    const i32 follow_hairs_per_guide_hair = 2;
    glm::mat4 model_matrix;


    //
    // rendering
    i32 render_mode = TFxRenderingMode::Normal;
    const bool use_shortcut = false;
    // color
    glm::vec4 root_color = {0.93, 0.11, 0.11, 1.0};
    glm::vec4 tip_color = {0.09, 0.09, 0.14, 1.0};
    bool use_separate_tip_color = true;
    // thickness
    f32 hair_thickness = 0.1;
    f32 hair_thickness_at_tip_ratio = 0.5; // thick hair root, thin tip
    bool use_thin_tip = true;


    //
    // simulation
    TressFXSimulationSettings simulation_settings;
    float gravity_multipler = 1.0f; // depends on scale of the model

  };

} // namespace glTFx
