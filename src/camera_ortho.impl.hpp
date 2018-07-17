struct CameraOrthoInputState {
  bool is_clicked = false;
  i32 last_x = 0, last_y = 0;
  glm::vec2 angles = {0,0};
};

static CameraOrthoInputState camera_ois;

static glm::mat4 get_view_matrix (const GlobalState& state, const CameraOrthoInputState& cois) {
  glm::mat4 view(1);

  // Opengl matrices are 'applied' in reverse, read from last to first:
  // 3. move back
  view = glm::translate(view, {0, 0, -state.camera_distance});
  // 2. rotate up-down
  view = glm::rotate(view, cois.angles[0], {1, 0, 0});
  // 1. rotate left-right
  view = glm::rotate(view, cois.angles[1], {0, 1, 0});

  // move camera up/down on global Y axis
  // (helps to center camera on hair, not waist)
  view[3][1] -= state.camera_magic_offset_y;

  return view;
}

static glm::mat4 get_proj_matrix (const GlobalState& state) {
  return glm::perspective(
    glm::radians(state.fov_dgr),
    state.aspect_ratio(),
    state.zNear, state.zFar);
}

void update_camera (GlobalState& state, const SDL_Event& ev) {
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

      camera_ois.angles.y += dx * state.camera_sensitivity;
      camera_ois.angles.x += dy * state.camera_sensitivity;
      while (camera_ois.angles.y < 0)                   {camera_ois.angles.y +=  glm::radians(360.);}
      while (camera_ois.angles.y >= glm::radians(360.)) {camera_ois.angles.y -=  glm::radians(360.);}
      if (camera_ois.angles.x < -glm::radians(89.))     {camera_ois.angles.x  = -glm::radians(89.);}
      if (camera_ois.angles.x > glm::radians(89.))      {camera_ois.angles.x  =  glm::radians(89.);}
    }
  } else if (ev.type == SDL_MOUSEWHEEL) {
    state.camera_distance += -ev.wheel.y * state.camera_distance_sensitivity;
  }

  state.camera.view = get_view_matrix(state, camera_ois);
  state.camera.projection = get_proj_matrix(state);
}
