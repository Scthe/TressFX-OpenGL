#include "../libs/imgui/imgui.h"
#include "../libs/imgui/imgui_impl_sdl.h"
#include "../libs/imgui/imgui_impl_opengl3.h"

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

void imgui_update (GlWindow& window, GlobalState& state) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(window.sdl_window);
  ImGui::NewFrame();

  // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
  {
    ImGui::Text("Hello, world!");
    ImGui::SliderFloat("Test float", &state.test_float, 0.0f, 1.0f);
    ImGui::ColorEdit3("Test vec4 color", (float*)&state.test_vec4);

    ImGui::Checkbox("Test bool", &state.test_bool);

    if (ImGui::Button("Test counter btn")) {
      state.test_counter++;
    }
    ImGui::SameLine();
    ImGui::Text("counter = %d", state.test_counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  }

  // create display list internally
  ImGui::Render();
  SDL_GL_MakeCurrent(window.sdl_window, window.gl_context);
}
