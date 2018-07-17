#include <cstring>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "../include/gl-utils/main.hpp"

#include "gl-tfx/TFxSample.hpp"

using namespace glUtils;
#pragma GCC diagnostic ignored "-Wunused-function" // commented out fn

#include "State.hpp"
#include "gui.impl.hpp"
#include "scene_load.impl.hpp"
#include "camera_ortho.impl.hpp"



///
/// main
///

void draw_bg (GlobalState& state, const Shader& shader, GLuint whatever_vao_gl_id) {
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
  glBindVertexArray(whatever_vao_gl_id);

  // draw
  glDrawArrays(GL_TRIANGLES, 0, 6);
}



static GLuint tmp_vao;
static Shader perf_test_shader;
void perf_test_init () {
  glGenVertexArrays(1, &tmp_vao);
  glBindVertexArray(tmp_vao);
  glVertexAttribI1i(0, 0); // index, default_value
  glBindVertexArray(0);

  create_shader(perf_test_shader,
    "src/shaders/perf_test/vert.glsl",
    "src/shaders/perf_test/frag.glsl"
  );
}

void perf_test_draw () {
  glDisable(GL_CULL_FACE);
  glUseProgram(perf_test_shader.gl_id);
  glBindVertexArray(tmp_vao);

  // auto verices_count = 6;
  GLsizei verices_count = 1249920;// / 1000;
  // TODO set index buffer?
  // glDrawElementsInstanced(GL_TRIANGLES, verices_count,
      // idx_buffer.buffer_el_type, 0, drawParams.numInstances);
  // glDrawElements(GL_TRIANGLES, verices_count, idx_buffer.buffer_el_type, 0);
  // glDrawElements(GL_TRIANGLES, verices_count, idx_buffer.buffer_el_type, 0);
  glDrawArrays(GL_TRIANGLES, 0, verices_count);
}

int main(int argc, char *argv[]) {
  // logger::Log::ReportingLevel = logger::Trace;
  // logger::Log::ReportingLevel = logger::Warning;
  logger::Log::ReportingLevel = logger::Error;

  GlobalState state;

  WindowInitOpts opts;
  opts.title = state.title;
  opts.w = state.win_width;
  opts.h = state.win_height;
  auto window = create_window(opts);
  LOGI << "Created window: " << window.screen_size[0] << "x" << window.screen_size[1];
  apply_draw_parameters(state.draw_params, nullptr);
  DrawParameters clean_params;

  DebugBehaviourForType debug_beh;
  debug_beh.error.set_all(DebugBehaviour::AsError);
  debug_beh.undefined_behavior.set_all(DebugBehaviour::AsError);
  debug_beh.performance.high = DebugBehaviour::AsDebug;
  glUtils::init_debug_callback(debug_beh);

  // TFx test:
  glTFx::TFxSample tfx_sample(&state);
  tfx_sample.init();
  // LOGI << "TFxSample::init finished!";
  // LOGI << "TFxSample::draw starts now";
  glClearColor(0.5, 0.5, 0.5, 0.5);
  glClearStencil(0);
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  // tfx_sample.draw_hair();
  // SDL_GL_SwapWindow(window.sdl_window);
  // LOGI << "TFxSample::draw finished!";
  // system("pause"); state.running = false;

  // perf_test_init();


  // Geometry scene_geometry = load_scene(state);
  // Shader bg_shader;
  // create_shader(bg_shader, state.bg_vs, state.bg_fs);

  imgui_init(window);

  // TODO check write mask?


  while(state.running) {
    // events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);

      update_camera(state, event);

      if (event.type == SDL_QUIT) {
        state.running = false;
      }
    }

    // clear
    state.update_draw_params(clean_params);
    glStencilMask(~0); // TODO reorganize stencil in DrawParameters - in reality it is simpler then current, with less front/back-separable features
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // we could use stencils to make it more optimized, but ./shrug
    // draw_bg(state, bg_shader, scene_geometry.vao.gl_id);
    // draw_scene(state, scene_geometry);

    // perf_test_draw();

    tfx_sample.draw_hair();
    // glFlush();
    // glFinish();

    // ui
    state.update_draw_params(clean_params);
    imgui_update(window, state); // prepare draw command list
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // draw

    // swap
    SDL_GL_SwapWindow(window.sdl_window);
  }

  // Cleanup
  // destroy_scene(scene_geometry);
  imgui_destroy();
  destroy(window);

  LOGI << "--- FIN ---";
  return 0;
}
