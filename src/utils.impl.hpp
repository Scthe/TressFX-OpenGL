static std::string get_file_contents(const char *filename) {
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in) {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  } else {
    LOGE << "Error reading file content: '" << filename << "'";
    throw(errno);
  }
}

static void create_shader (Shader& shader, const std::string& p_vs, const std::string& f_vs) {
  ShaderTexts shader_texts;
  auto vs = get_file_contents(p_vs.c_str());
  auto fs = get_file_contents(f_vs.c_str());
  shader_texts.vertex = vs.c_str();
  shader_texts.fragment = fs.c_str();

  ShaderErrorsScratchpad es;
  shader = create_shader(shader_texts, es);
  if (!shader.is_created()) {
    LOGE << "Shader create error: " << es.msg;
    GFX_FAIL("Could not create scene shader [",
      p_vs.c_str(), ", ", f_vs.c_str(),
    "]");
  }
}
