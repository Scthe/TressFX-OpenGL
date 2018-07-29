#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <glm/gtc/matrix_transform.hpp>
#include "../include/gl-utils/main.hpp"

#include "gl-tfx/TFxSample.hpp"

using namespace glUtils;
#pragma GCC diagnostic ignored "-Wunused-function" // commented out fn

#include "State.hpp"
#include "utils.impl.hpp"
#include "gui.impl.hpp"
#include "scene.impl.hpp"
#include "camera_ortho.impl.hpp"


void create_dummy_vao (GLuint& vao) {
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glVertexAttribI1i(0, 0); // index, default_value
  glBindVertexArray(0);
}

void draw_bg (GlobalState& state, const Shader& shader) {
  // shader & PSO
  glUseProgram(shader.gl_id);
  DrawParameters params;
  params.depth.write = false;
  params.depth.test = DepthTest::AlwaysPass;
  state.update_draw_params(params);

  // uniforms
  glUtils::set_uniform(shader, "g_color_top", state.bg_grad_top);
  glUtils::set_uniform(shader, "g_color_bottom", state.bg_grad_bottom);
  glUtils::set_uniform(shader, "g_screenHeight", (f32)state.win_height, true);

  // geo
  glBindVertexArray(state.dummy_vao);

  // draw
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void draw_wind (GlobalState& state, const Shader& shader) {
  // shader & PSO
  glUseProgram(shader.gl_id);
  DrawParameters params;
  params.culling = glUtils::CullingMode::None;
  state.update_draw_params(params);

  // uniforms
  const auto& tfx_settings = state.tfx_settings;
  const auto& camera = state.camera;
  const auto& sim_settings = tfx_settings.simulation_settings;
  glm::vec3 wind_dir = { sim_settings.m_windDirection[0], sim_settings.m_windDirection[1], sim_settings.m_windDirection[2] };
  wind_dir = glm::normalize(wind_dir);
  glUtils::set_uniform(shader, "g_WindDirection", glm::vec4(wind_dir, 1.0), true);
  glUtils::set_uniform(shader, "g_Eye", state.camera.get_position(), true);
  glUtils::set_uniform(shader, "g_VP", camera.projection * camera.view);


  // geo
  glBindVertexArray(state.dummy_vao);

  // draw
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void init_tfx_settings(GlobalState& state, std::vector<Geometry>& scene_objects, bool use_sintel) {
  auto& settings = state.tfx_settings;
  auto& sim = settings.simulation_settings;

  settings.root_color = {0.03, 0.07, 0.25, 1.0};
  settings.tip_color = {0.16, 0.45, 0.64, 1.0};

  if (use_sintel) {
    // sintel
    const float model_scale = 0.01;
    settings.object_name = "sintel_hair";
    settings.filepath = "assets\\sintel_lite_v2_1\\GEO-sintel_hair_emit.001-sintel_hair.tfx";
    // settings.filepath = "assets\\sintel_lite_v2_1\\0TestHair-TestHair.tfx";
    settings.hair_thickness = 0.12;
    sim.m_gravityMagnitude = 0.0f;
    sim.m_damping = 1.0f;
    settings.follow_hair_root_offset_multiplier = 0.004 / model_scale;
    // model matrix
    glm::mat4 m(1);
    m = glm::scale(m, glm::vec3(model_scale, model_scale, model_scale));
    settings.model_matrix = glm::translate(m, {0,-0.35 / model_scale, 0});
    // objects
    // scene_objects.push_back(create_geometry("assets\\sintel_lite_v2_1\\cube.obj"));
    scene_objects.push_back(create_geometry("assets\\sintel_lite_v2_1\\sintel.obj"));
    scene_objects.push_back(create_geometry("assets\\sintel_lite_v2_1\\sintel_eyeballs.obj"));
    state.show_model = false;

  } else {
    // mohawk - AMD standard model
    const float model_scale = 0.01;
    settings.gravity_multipler = 30.0f;
    sim.m_gravityMagnitude = 1.0f;
    // model matrix
    glm::mat4 m(1);
    m = glm::scale(m, glm::vec3(model_scale, model_scale, model_scale));
    settings.model_matrix = glm::translate(m, {0,-60,0}); // scale is absurd
  }
}

///
/// main
///


int main(int argc, char *argv[]) {
  // logger::Log::ReportingLevel = logger::Trace;
  // logger::Log::ReportingLevel = logger::Warning;
  logger::Log::ReportingLevel = logger::Error;

  GlobalState state;

  // create window
  // (NO GL FUNCTION BEFORE THIS!)
  WindowInitOpts opts;
  opts.title = state.title;
  opts.w = state.win_width;
  opts.h = state.win_height;
  auto window = create_window(opts);
  LOGI << "Created window: " << window.screen_size[0] << "x" << window.screen_size[1];
  imgui_init(window);

  // init state
  std::vector<Geometry> scene_objects;
  init_tfx_settings(state, scene_objects, true);
  apply_draw_parameters(state.draw_params, nullptr);
  create_dummy_vao(state.dummy_vao);

  // set opengl debug print policy
  DebugBehaviourForType debug_beh;
  debug_beh.error.set_all(DebugBehaviour::AsError);
  debug_beh.undefined_behavior.set_all(DebugBehaviour::AsError);
  debug_beh.performance.high = DebugBehaviour::AsDebug;
  glUtils::init_debug_callback(debug_beh);

  // TFx
  glTFx::TFxSample tfx_sample(&state);
  tfx_sample.init();

  // scene geometry
  // Shader bg_shader;
  // create_shader(bg_shader, state.bg_vs, state.bg_fs);
  const std::string wind_vs = "src/shaders/wind.vert.glsl";
  const std::string wind_fs = "src/shaders/wind.frag.glsl";
  Shader wind_shader;
  create_shader(wind_shader, wind_vs, wind_fs);
  Shader scene_object_shader;
  create_shader(scene_object_shader, state.obj_vs, state.obj_fs);

  glClearColor(0.5, 0.5, 0.5, 0.5);
  glClearStencil(0);

  // run
  while(state.running) {
    // events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);

      if (!ImGui::GetIO().WantCaptureMouse) {
        update_camera(state, event);
      }

      if (event.type == SDL_QUIT) {
        state.running = false;
      }
    }

    // clear
    DrawParameters clean_params;
    state.update_draw_params(clean_params);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // scene
    // we could use stencils to make it more optimized, but ./shrug
    // draw_bg(state, bg_shader);
    draw_wind(state, wind_shader);
    if (state.show_model) {
      for (auto& geo : scene_objects) {
        draw_geometry(state, scene_object_shader, geo);
      }
    }

    // tfx
    tfx_sample.simulate(0.0);
    tfx_sample.wait_simulate_done();
    tfx_sample.draw_hair();

    // ui
    state.update_draw_params(clean_params);
    imgui_update(window, state); // prepare draw command list
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // draw

    // swap buffers
    SDL_GL_SwapWindow(window.sdl_window);
  }

  // Cleanup
  for (auto& scene_obj : scene_objects) {
    destroy_geometry(scene_obj);
  }
  imgui_destroy();
  destroy(window);

  LOGI << "--- FIN ---";
  return 0;
}
