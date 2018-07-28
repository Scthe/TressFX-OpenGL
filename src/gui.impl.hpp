#include "../libs/imgui/imgui.h"
#include "../libs/imgui/imgui_impl_sdl.h"
#include "../libs/imgui/imgui_impl_opengl3.h"
#include "gl-tfx/print_me.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/polar_coordinates.hpp"

void imgui_init (GlWindow& window) {
  // Setup Dear ImGui binding
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

  ImGui_ImplSDL2_InitForOpenGL(window.sdl_window, window.gl_context);
  ImGui_ImplOpenGL3_Init();

  // Setup style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();
}

void imgui_destroy() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

static void update_wind_direction(float* winDirs, glm::vec2 angles) {
  angles.x = -angles.x;
  angles.y += 90;
  auto cart = glm::euclidean(glm::radians(angles));
  winDirs[0] = cart.x;
  winDirs[1] = cart.y;
  winDirs[2] = cart.z;
}

void imgui_update (GlWindow& window, GlobalState& state) {
  auto& style = ImGui::GetStyle();
  style.ChildRounding = 0.2f;
  style.WindowRounding = 0.2f;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(window.sdl_window);
  ImGui::NewFrame();

  auto& tfx_settings = state.tfx_settings;
  auto& sim_settings = tfx_settings.simulation_settings;
  auto color_flags = ImGuiColorEditFlags_NoInputs
                   | ImGuiColorEditFlags_NoAlpha
                   | ImGuiColorEditFlags_HSV;
  auto slider_size = -150;
  auto window_width = 300;
  auto window_margin = 10;
  float section_indent = 10;

  #define INSERT_SPACING_HERE {ImGui::Spacing(); ImGui::Spacing();}
  #define ADD_TOOLTIP(TOOLTIP) if (ImGui::IsItemHovered()) { ImGui::SetTooltip(TOOLTIP); }

  {
    ImGui::SetNextWindowPos({(float)state.win_width - window_width - window_margin, (float)window_margin});
    ImGui::SetNextWindowSize(ImVec2(window_width, 0), ImGuiSetCond_Once);
    ImGui::Begin("TressFX OpenGL");
    ImGui::PushItemWidth(slider_size);

    // fps
    auto fps = ImGui::GetIO().Framerate;
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / fps, fps);
    INSERT_SPACING_HERE

    // Render subtree
    if (ImGui::CollapsingHeader("Render color", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Indent(section_indent);
      ImGui::ColorEdit3("Hair color", (float*)&tfx_settings.root_color, color_flags);
      ImGui::SliderFloat("Root opacity", (float*)&tfx_settings.root_color.a, 0.0f, 1.0f);
      ImGui::ColorEdit3("Hair tip color", (float*)&tfx_settings.tip_color, color_flags);
      ImGui::SliderFloat("Tip opacity", (float*)&tfx_settings.tip_color.a, 0.0f, 1.0f);
      ImGui::Checkbox("Use separate tip color", &tfx_settings.use_separate_tip_color);
      ImGui::SliderFloat("Color rand", &tfx_settings.strand_hue_rand_scale, 0.0f, 0.1f);
      ImGui::Unindent(section_indent);
    }
    INSERT_SPACING_HERE

    // Thickness subtree
    if (ImGui::CollapsingHeader("Render thickness", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Indent(section_indent);
      ImGui::SliderFloat("Hair thickness", &tfx_settings.hair_thickness, 0.1f, 1.0f);
      ImGui::Checkbox("Use thin tip", &tfx_settings.use_thin_tip);
      ImGui::SliderFloat("Tip thickness ratio", &tfx_settings.hair_thickness_at_tip_ratio, 0.01f, 1.0f);
      ADD_TOOLTIP("0 := tip is very small, 1 := tip is same size as parent. This settting is heavily affected by aliasing-preventing techniques");
      ImGui::SliderFloat("Hair tip split", &sim_settings.m_tipSeparation, 0.0f, 10.0f);
      ImGui::Unindent(section_indent);
    }
    INSERT_SPACING_HERE

    // Simulation subtree
    if (ImGui::CollapsingHeader("Simulation")) {
      ImGui::Indent(section_indent);
      ImGui::SliderFloat("Gravity", &sim_settings.m_gravityMagnitude, 0.0f, 10.0f);
      ImGui::SliderFloat("Damping", &sim_settings.m_damping, 0.0f, 1.0f);
      ADD_TOOLTIP("Inertia. 1 := slow down, 0 := run as normal");
      ImGui::SliderFloat("Global stiffness", &sim_settings.m_globalConstraintStiffness, 0.0f, 1.0f);
      ADD_TOOLTIP("(Global Shape Constraint) How much restore initial shape of the hair");
      ImGui::SliderFloat("Global range", &sim_settings.m_globalConstraintsRange, 0.0f, 1.0f);
      ADD_TOOLTIP("(Global Shape Constraint) 1 := affect from root to tip, 0.5 := affect from root to midway");
      ImGui::SliderFloat("VSP force", &sim_settings.m_vspCoeff, 0.0f, 1.0f);
      ADD_TOOLTIP("(Velocity Shock Propogation) 0 := no VSP, 1.0 := use VSP");
      ImGui::SliderFloat("VSP threshold", &sim_settings.m_vspAccelThreshold, 0.0f, 1.0f);
      ADD_TOOLTIP("(Velocity Shock Propogation) Acceleration above with ignore 'VSP force' and turn on VSP");
      ImGui::SliderFloat("Local stiffness", &sim_settings.m_localConstraintStiffness, 0.0f, 1.0f);
      ADD_TOOLTIP("(Local Shape Constraints) How much restore local shape (direction between vertices)");
      // ImGui::SliderFloat("Length stiffness", &sim_settings.hair_thickness, 0.0f, 1.0f);
      ImGui::SliderInt("Length iterations", &sim_settings.m_lengthConstraintsIterations, 0, 5);
      ADD_TOOLTIP("(Length Constraint) Fixing hairs that are too long/short");
      ImGui::Unindent(section_indent);
    }
    INSERT_SPACING_HERE

    // Simulation wind subtree
    if (ImGui::CollapsingHeader("Simulation wind")) {
      ImGui::Indent(section_indent);
      static glm::vec2 wind_angles = {0,0};
      ImGui::SliderFloat("Direction theta", &(wind_angles[1]), 0, 360);
      ImGui::SliderFloat("Direction fi", &(wind_angles[0]), -85, 85);
      update_wind_direction(sim_settings.m_windDirection, wind_angles);
      ImGui::PushItemWidth(slider_size);
      ImGui::SliderFloat("Force", &sim_settings.m_windMagnitude, 0.0f, 300.0f);
      ImGui::Unindent(section_indent);
    }
    INSERT_SPACING_HERE

    // debug subtree
    if (ImGui::CollapsingHeader("Debug"), ImGuiTreeNodeFlags_DefaultOpen) {
      ImGui::Indent(section_indent);
      const char* items[] = {"Normal", "PPLL Binary", "PPLL Overlap"};
      ImGui::Combo("Render mode", &tfx_settings.render_mode, items, IM_ARRAYSIZE(items));
      if (ImGui::Button("Print settings")) {
        glTFx::debug::debug_settings(tfx_settings);
      }

      const auto& cam = state.camera;
      auto p = cam.get_position();
      ImGui::Text("Camera (x=%.4f, y=%.4f, z=%.4f)", p.x, p.y, p.z);
      if (ImGui::Button("Print camera position")) {
        LOGE << "Camera (x=" << p.x << ", y=" << p.y << ", z=" << p.z << ")\n"
             << "view " << glm::to_string(cam.view) << "\n"
             << "projection " << glm::to_string(cam.projection);
      }
      ImGui::Unindent(section_indent);
    }
    INSERT_SPACING_HERE


    ImGui::End();
  }

  // create display list internally
  ImGui::Render();
  SDL_GL_MakeCurrent(window.sdl_window, window.gl_context);
}
