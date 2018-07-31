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

static void init_tfx_settings(GlobalState&, std::vector<Geometry>&, bool use_sintel);
static void draw_bg (GlobalState&, const Shader&);
static void draw_wind (GlobalState&, const Shader&, glm::vec4);
static void draw_debug_capsules(GlobalState&, const Shader&, const Geometry&);


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
  Shader wind_shader;
  create_shader(wind_shader, "src/shaders/wind.vert.glsl", "src/shaders/wind.frag.glsl");
  Shader scene_object_shader;
  create_shader(scene_object_shader, state.obj_vs, state.obj_fs);
  Shader capsule_debug_shader;
  create_shader(capsule_debug_shader, "src/shaders/capsule_debug.vert.glsl", "src/shaders/capsule_debug.frag.glsl");
  auto capsule_debug_geo = create_geometry("assets/sintel_lite_v2_1/icosphere.obj");

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


    // scene and debug draws
    if (state.tfx_settings.show_wind) {
      draw_wind(state, wind_shader, state.tfx_settings.wind0);
      draw_wind(state, wind_shader, state.tfx_settings.wind1);
    }
    // we could use stencils to make it more optimized, but ./shrug
    state.update_draw_params(clean_params);
    // draw_bg(state, bg_shader);
    if (state.show_model) {
      for (auto& geo : scene_objects) {
        draw_geometry(state, scene_object_shader, geo);
      }
    }
    state.update_draw_params(clean_params);
    if (state.show_collision_capsules) {
      draw_debug_capsules(state, capsule_debug_shader, capsule_debug_geo);
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
  destroy_geometry(capsule_debug_geo);
  imgui_destroy();
  destroy(window);

  LOGI << "--- FIN ---";
  return 0;
}


///
/// utils
///

void init_tfx_settings(GlobalState& state, std::vector<Geometry>& scene_objects, bool use_sintel) {
  auto& settings = state.tfx_settings;
  auto& sim = settings.simulation_settings;

  settings.root_color = {0.03, 0.07, 0.25, 1.0};
  settings.tip_color = {0.16, 0.45, 0.64, 0.2};
  settings.show_wind = true;

  if (use_sintel) {
    // sintel
    const float model_scale = 0.01;
    settings.object_name = "sintel_hair";
    settings.filepath = "assets\\sintel_lite_v2_1\\GEO-sintel_hair_emit.001-sintel_hair.tfx";
    // settings.filepath = "assets\\sintel_lite_v2_1\\0TestHair-TestHair.tfx";
    settings.hair_thickness = 0.28;
    sim.m_tipSeparation = 2.1;
    sim.m_gravityMagnitude = 0.0f;
    sim.m_damping = 0.0f;
    settings.follow_hair_root_offset_multiplier = 1.2;
    // model matrix
    glm::mat4 m(1);
    m = glm::scale(m, glm::vec3(model_scale, model_scale, model_scale));
    settings.model_matrix = glm::translate(m, {0,-0.35 / model_scale, 0});
    // objects
    scene_objects.push_back(create_geometry("assets\\sintel_lite_v2_1\\sintel.obj"));
    scene_objects.push_back(create_geometry("assets\\sintel_lite_v2_1\\sintel_eyeballs.obj"));

    // collision capsules
    settings.collision_capsule0 = glm::vec4(0, 36.65, -1.3,  9.27);
    settings.collision_capsule1 = glm::vec4(0, 35.81, 2.38,  8.97);
    settings.collision_capsule2 = glm::vec4(0, 26.56, 2.75,  7.89);

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

void draw_wind (GlobalState& state, const Shader& shader, glm::vec4 wind_dir4) {
  // shader & PSO
  glUseProgram(shader.gl_id);
  DrawParameters params;
  params.culling = glUtils::CullingMode::None;
  state.update_draw_params(params);

  // uniforms
  const auto& tfx_settings = state.tfx_settings;
  const auto& camera = state.camera;
  glm::vec3 wind_dir = { wind_dir4.x, wind_dir4.y, wind_dir4.z };
  wind_dir = glm::normalize(wind_dir);
  glUtils::set_uniform(shader, "g_WindDirection", glm::vec4(wind_dir, 1.0), true);
  glUtils::set_uniform(shader, "g_Eye", state.camera.get_position(), true);
  glUtils::set_uniform(shader, "g_VP", camera.projection * camera.view);


  // geo
  glBindVertexArray(state.dummy_vao);

  // draw
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void draw_debug_capsules(GlobalState& state, const Shader& shader, const Geometry& geo) {
  auto& settings = state.tfx_settings;
  glUseProgram(shader.gl_id);

  // uniforms
  glm::vec4 capsule_data[] = {
    settings.collision_capsule0,
    settings.collision_capsule1,
    settings.collision_capsule2,
    settings.collision_capsule3
  };
  auto variable = shader.get_uniform("g_Capsules[0]");
  GFX_FAIL_IF(!variable, "Could not set capsule data in shader");
  glUniform4fv(variable->gl_location, 4, (float*)&capsule_data);
  const auto& camera = state.camera;
  const auto model_mat = state.tfx_settings.model_matrix;
  auto mvp = camera.projection * camera.view * model_mat;
  glUtils::set_uniform(shader, "g_MVP", mvp, false, true);

  // draw
  glBindVertexArray(geo.vao.gl_id);
  glBindBuffer(glUtils::BufferBindType::IndexBuffer, geo.index_buffer.gl_id);
  glDrawElementsInstanced (GL_TRIANGLES, geo.triangles * 3, GL_UNSIGNED_INT, nullptr, 4);
}
