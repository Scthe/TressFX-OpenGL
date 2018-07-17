#pragma once

struct Camera {
    glm::mat4 view;
    glm::mat4 projection;
};

/**
 * Overall state of the program. Mainly used with imgui for debug
 */
struct GlobalState {
  // init
  u32 win_width = 1280;
  u32 win_height = 720;
  inline float aspect_ratio () const noexcept {return this->win_width * 1.0f / this->win_height; }
  std::string title = "TressFX";

  // TFx
  bool use_shortcut = false;

  // scene
  std::string obj_path = "assets/PonytailModel/ponytail.obj";
  std::string obj_vs = "src/shaders/simple_obj.vert.glsl";
  std::string obj_fs = "src/shaders/simple_obj.frag.glsl";
  std::string bg_vs = "src/shaders/bg.vert.glsl";
  std::string bg_fs = "src/shaders/bg.frag.glsl";
  glm::vec4 bg_grad_top    = {0.49, 0.57, 0.65, 1.0}; // some nice gray-blue color
  glm::vec4 bg_grad_bottom = {0.01, 0.01, 0.01, 1.0};

  // camera
  Camera camera;
  float camera_distance = 2.0f;
  float camera_distance_sensitivity = 0.1f;
  float camera_sensitivity = 0.005f;
  float camera_magic_offset_y = 0.5;
  float zNear = 0.1f;
  float zFar = 50.0f;
  float fov_dgr = 45.0f;
  // glm::vec3 cam_pos = {0,0,2};
  // glm::vec3 cam_target = {0,0,0};

  // runtime
  bool running = true;
  glUtils::DrawParameters draw_params;
  inline void update_draw_params (const glUtils::DrawParameters& new_parms) noexcept {
    glUtils::apply_draw_parameters(new_parms, &this->draw_params);
    this->draw_params = new_parms;
  }

  // ui tests
  bool test_bool = false;
  float test_float = 0.45f;
  int test_counter = 0;
  glm::vec4 test_vec4 = {0.45f, 0.55f, 0.60f, 1.00f};
};
