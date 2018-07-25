glm::vec3 Camera::get_position () const noexcept {
  glm::mat4 view(1);
  view = glm::rotate(view, this->angles[0], {1, 0, 0});
  view = glm::rotate(view, this->angles[1], {0, 1, 0});

  glm::vec4 translate = {0, 0, -this->camera_distance, 1};
  auto res = view * translate;
  if (res.z > 0) { res.y = -res.y; } // ?!
  return glm::vec3(res.x, res.y, -res.z);
}

struct CameraOrthoInputState {
  bool is_clicked = false;
  i32 last_x = 0, last_y = 0;
};
static CameraOrthoInputState camera_ois;

static glm::mat4 get_view_matrix (const Camera& state) {
  glm::mat4 view(1);

  // Opengl matrices are 'applied' in reverse, read from last to first:
  // 3. move back
  view = glm::translate(view, {0, 0, -state.camera_distance});
  // 2. rotate up-down
  view = glm::rotate(view, state.angles[0], {1, 0, 0});
  // 1. rotate left-right
  view = glm::rotate(view, state.angles[1], {0, 1, 0});

  return view;
}

static glm::mat4 get_proj_matrix (const GlobalState& state) {
  const auto& camera = state.camera;
  return glm::perspective(
    glm::radians(camera.fov_dgr),
    state.aspect_ratio(),
    camera.zNear, camera.zFar);
}

void update_camera (GlobalState& state, const SDL_Event& ev) {
  auto& camera = state.camera;

  if (ev.type == SDL_MOUSEBUTTONUP || ev.type == SDL_MOUSEBUTTONDOWN) {
    if (ev.button.button == SDL_BUTTON_LEFT) {
      camera_ois.is_clicked = ev.button.type == SDL_MOUSEBUTTONDOWN;
      camera_ois.last_x = ev.button.x;
      camera_ois.last_y = ev.button.y;
    }
  } else if (ev.type == SDL_MOUSEMOTION) {
    if (camera_ois.is_clicked) {
      auto dx = ev.motion.x  - camera_ois.last_x;
      auto dy = ev.motion.y  - camera_ois.last_y;
      camera_ois.last_x = ev.motion.x;
      camera_ois.last_y = ev.motion.y;

      camera.angles.y += dx * camera.camera_sensitivity;
      camera.angles.x += dy * camera.camera_sensitivity;
      while (camera.angles.y < 0)                   {camera.angles.y +=  glm::radians(360.);}
      while (camera.angles.y >= glm::radians(360.)) {camera.angles.y -=  glm::radians(360.);}
      if (camera.angles.x < -glm::radians(89.))     {camera.angles.x  = -glm::radians(89.);}
      if (camera.angles.x > glm::radians(89.))      {camera.angles.x  =  glm::radians(89.);}
    }
  } else if (ev.type == SDL_MOUSEWHEEL) {
    camera.camera_distance += -ev.wheel.y * camera.camera_distance_sensitivity;
  }

  state.camera.view = get_view_matrix(state.camera);
  state.camera.projection = get_proj_matrix(state);
}
