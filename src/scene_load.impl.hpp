#include "../libs/tiny_obj_loader/tiny_obj_loader.h"

#include <fstream>
#include <string>
#include <cerrno>

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


struct Geometry {
  VAO vao;
  RawBuffer vertex_buffer;
  RawBuffer normal_buffer;
  RawBuffer index_buffer;
  Shader shader;
  u32 triangles = 0;
};

/*struct SimpleVertex {
  glm::vec3 position;
  glm::vec3 normal;
};*/

static void load_obj (const GlobalState& state, Geometry& geo) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;

  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, nullptr, &err, state.obj_path.c_str());

  if (!ret || !err.empty()) {
    LOGE << err;
    exit(1);
  }
  GFX_FAIL_IF(shapes.size() != 1, ".obj contains != 1 shapes. Please join all models in the scene");

  const auto& shape = shapes[0];
  const auto& mesh = shape.mesh;
  LOGI << "Loaded obj '" << state.obj_path << "'. Mesh '" << shape.name
       << "', vertices: " << attrib.vertices.size()
       << ", triangles: " << mesh.indices.size() / 3;

  // pack positions and normals (see definition of index_t)
  // SimpleVertex* verts = (SimpleVertex*)malloc(sizeof(SimpleVertex) * mesh.indices.size());
  std::vector<unsigned int> indices;
  for (size_t i = 0; i < mesh.indices.size(); i++) {
    const auto& idx = mesh.indices[i];
    indices.push_back((unsigned int) idx.vertex_index);

    /*SimpleVertex vv = {
      {
        attrib.vertices[idx.vertex_index*3 + 0],
        attrib.vertices[idx.vertex_index*3 + 1],
        attrib.vertices[idx.vertex_index*3 + 2]
      }, {
        attrib.normals[idx.normal_index*3 + 0],
        attrib.normals[idx.normal_index*3 + 1],
        attrib.normals[idx.normal_index*3 + 2]
      }
    };
    verts[idx.vertex_index] = vv;*/
  }

  u32 mem_idx = indices.size() * sizeof(int);

  // load to gpu
  u32 mem_verts = attrib.vertices.size() * sizeof(float);
  u32 mem_norm = attrib.normals.size() * sizeof(float);
  geo.vertex_buffer  = glUtils::malloc(mem_verts, BufferUsagePattern::Default);
  geo.normal_buffer  = glUtils::malloc(mem_norm, BufferUsagePattern::Default);
  glUtils::write(geo.vertex_buffer,  {0, mem_verts}, &attrib.vertices.at(0));
  glUtils::write(geo.normal_buffer,  {0, mem_norm}, &attrib.normals.at(0));
  // SimpleVertex
  // u32 mem_simple_vert = sizeof(SimpleVertex) * mesh.indices.size();
  // geo.vertex_buffer  = glUtils::malloc(mem_simple_vert, BufferUsagePattern::Default);
  // glUtils::write(geo.vertex_buffer,  {0, mem_simple_vert}, verts);
  // index buffer
  geo.triangles = indices.size() / 3;
  geo.index_buffer = glUtils::malloc(mem_idx, BufferUsagePattern::Default);
  glUtils::write(geo.index_buffer, {0, mem_idx},   &indices.at(0));
  // vao
  geo.vao = glUtils::create_vao({
    {&geo.vertex_buffer, {GL_FLOAT,3}, 0, 3*sizeof(float)}, // position
    {&geo.normal_buffer, {GL_FLOAT,3}, 0, 3*sizeof(float)} // normals
    // {&geo.vertex_buffer, {GL_FLOAT,3}, 0, 6*sizeof(float)}, // position
    // {&geo.vertex_buffer, {GL_FLOAT,3}, 3*sizeof(float), 6*sizeof(float)} // normals
  });

  // free(verts);
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

static Geometry load_scene (const GlobalState& state) {
  Geometry head_model;
  load_obj(state, head_model);
  create_shader(head_model.shader, state.obj_vs, state.obj_fs);
  return head_model;
}

static void draw_scene (GlobalState& state, const Geometry& geo) {
  // shader & PSO
  glUseProgram(geo.shader.gl_id); // TODO move glUseProgram into set_uniform?
  glUtils::DrawParameters params;
  // params.backface_culling = BackfaceCullingMode::CullingDisabled;
  // params.polygon_mode = PolygonMode::Edge;
  state.update_draw_params(params);

  // uniforms
  // glm::mat4 matProj = glm::perspective(glm::radians(state.fov_dgr), state.aspect_ratio(), state.zNear, state.zFar);
  // glm::mat4 matView = glm::lookAt(state.cam_pos, state.cam_target, {0,1,0});
  glm::mat4 matProj = state.camera.projection;
  glm::mat4 matView = state.camera.view;
  glUtils::set_uniform(geo.shader, "g_matProj", matProj);
  glUtils::set_uniform(geo.shader, "g_matView", matView);

  // geo
  glBindVertexArray(geo.vao.gl_id);
  glBindBuffer(glUtils::BufferBindType::IndexBuffer, geo.index_buffer.gl_id);


  // draw
  glDrawElements(GL_TRIANGLES, geo.triangles * 3, GL_UNSIGNED_INT, nullptr);
}

static void destroy_scene(Geometry& geo) {
  destroy(geo.vao);
  destroy(geo.vertex_buffer);
  destroy(geo.normal_buffer);
  destroy(geo.index_buffer);
}
