#include "../libs/tiny_obj_loader/tiny_obj_loader.h"

struct Geometry {
  VAO vao;
  RawBuffer vertex_buffer;
  RawBuffer index_buffer;
  u32 triangles = 0;
};

struct SimpleVertex {
  glm::vec3 position;
  glm::vec3 normal;
};

static Geometry create_geometry (const char*const obj_path) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;

  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, nullptr, &err, obj_path);
  GFX_FAIL_IF(!ret || !err.empty(), err);
  GFX_FAIL_IF(shapes.size() != 1, ".obj contains != 1 shapes. Please join all models in the scene");

  const auto& shape = shapes[0];
  const auto& mesh = shape.mesh;
  LOGI << "Loaded obj '" << obj_path << "' :: '" << shape.name
       << " (vertices=" << attrib.vertices.size()
       << ", traingles=" << mesh.indices.size() / 3 <<  ")";

  std::vector<SimpleVertex> vertices;
  std::vector<u32> indices;

  for (size_t i = 0; i < mesh.indices.size(); i++) {
    const auto& idx = mesh.indices[i];
    indices.push_back(i);

    const u32 vert_idx = idx.vertex_index * 3;
    glm::vec3 pos = {
      attrib.vertices[vert_idx],
      attrib.vertices[vert_idx + 1],
      attrib.vertices[vert_idx + 2]
    };
    const u32 norm_idx = idx.normal_index * 3;
    glm::vec3 norm = {
      attrib.normals[norm_idx],
      attrib.normals[norm_idx + 1],
      attrib.normals[norm_idx + 2]
    };

    vertices.push_back({ pos, norm });
  }

  Geometry geo;

  // load to gpu
  u32 mem_verts = vertices.size() * sizeof(SimpleVertex);
  geo.vertex_buffer = glUtils::malloc(mem_verts, BufferUsagePattern::Default);
  glUtils::write(geo.vertex_buffer,  {0, mem_verts}, &vertices.at(0));

  // vao
  geo.vao = glUtils::create_vao({
    {&geo.vertex_buffer, {GL_FLOAT,3}, 0,                 sizeof(SimpleVertex)}, // position
    {&geo.vertex_buffer, {GL_FLOAT,3}, sizeof(glm::vec3), sizeof(SimpleVertex)} // normals
  });

  // index buffer
  geo.triangles = indices.size() / 3;
  u32 mem_idx = indices.size() * sizeof(int);
  geo.index_buffer = glUtils::malloc(mem_idx, BufferUsagePattern::Default);
  glUtils::write(geo.index_buffer, {0, mem_idx}, &indices.at(0));

  return geo;
}

static void draw_geometry (const GlobalState& state, const Shader& shader, const Geometry& geo) {
  glUseProgram(shader.gl_id);

  // uniforms
  const auto& camera = state.camera;
  const auto model_mat = state.tfx_settings.model_matrix;
  auto mvp = camera.projection * camera.view * model_mat;
  glUtils::set_uniform(shader, "g_MVP", mvp);

  // draw
  glBindVertexArray(geo.vao.gl_id);
  glBindBuffer(glUtils::BufferBindType::IndexBuffer, geo.index_buffer.gl_id);
  glDrawElements(GL_TRIANGLES, geo.triangles * 3, GL_UNSIGNED_INT, nullptr);
}

static void destroy_geometry(Geometry& geo) {
  destroy(geo.vao);
  destroy(geo.vertex_buffer);
  destroy(geo.index_buffer);
}
