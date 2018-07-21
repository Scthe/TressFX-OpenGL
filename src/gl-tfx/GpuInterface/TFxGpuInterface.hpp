#pragma once

#include "../include_TFx_pls_no_warnings.hpp"
#include "../../../libs/amd_tressfx/include/AMD_TressFX.h"
#include "../../../libs/amd_tressfx/include/TressFXGPUInterface.h"

/*
#include "SuCommandList.h"
#include "SuGPUIndexBuffer.h"
#include "SuGPUResourceManager.h"
#include "SuGPUStructuredBuffer.h"
#include "SuGPUTexture2DArray.h"
#include "SuTextureSlot.h"
#include "SuTextureSlotBinder.h"
#include "SuUAVSlot.h"
#include "SuUAVSlotBinder.h"
#include "SuEffect.h"
// For timers
#include "SuRenderManager.h"
*/

enum class TFx_ResourceType {
  ClearableTexture2D, // TFx_cbCreate2D
  StructuredBufferRW_WithAtomicCounter, // read-write with additional space for uint
  StructuredBufferRW, // read-write
  StructuredBufferR // read
};

struct ClearableTexture {
  glUtils::RawBuffer clear_data_buffer;
  glUtils::Texture texture;
};

struct BufferWithCounter {
  glUtils::RawBuffer buffer;
  glUtils::RawBuffer counter;
};

class EI_Resource {
  public:
  EI_Resource();
  ~EI_Resource(){} // handled by pfDestroySB? anyway, not here

  TFx_ResourceType type;
  EI_StringHash name;

  union {
    ClearableTexture texture;
    glUtils::RawBuffer buffer;
    BufferWithCounter buffer_with_counter;
  };
  // public:
  // SuGPUResourcePtr resource;
  // SuGPUUnorderedAccessViewPtr    uav;
  // SuGPUSamplingResourceViewPtr   srv;
  // SuGPURenderableResourceViewPtr rtv;
};

// typedef glUtils::ShaderBlock SRVSlot; // SSBO (cause not known number of data)
// typedef glUtils::ShaderBlock UAVSlot; // SSBO
typedef glUtils::ShaderVariable EffectParameter; // TODO uniform not in UBO? or in UBO? check this!

struct ShaderResource {
  ShaderResource() : is_block(false), variable(nullptr){}
  inline bool was_found_in_glsl () const noexcept {return shader != nullptr;}

  bool is_block;
  std::string name;
  glUtils::Shader* shader = nullptr;

  union {
    glUtils::ShaderBlock* block;
    const glUtils::ShaderVariable* variable;
  };
};

struct EI_BindLayout {
  // std::vector<const SRVSlot*> srvs;
  // std::vector<const UAVSlot*> uavs;
  std::vector<ShaderResource> srvs;
  std::vector<ShaderResource> uavs;
  std::vector<const EffectParameter*> constants;
  // SuArray<const SuTextureSlot*> srvs;
  // SuArray<const SuUAVSlot*> uavs;
  // SuArray<SuEffectParameter*> constants;
};


struct GlobalState;
class EI_CommandContext { // ID3D11DeviceContext
  public:
  GlobalState* state = nullptr;
  TressFXHairObject* simulated_hair_object = nullptr;
};

class EI_Device {};

struct ShaderMetadata {
  glUtils::Shader* shader = nullptr;
  std::string path_1; // vertex/compute shader
  std::string path_2; // fragment shader
};

class EI_LayoutManager {
  public:
  std::vector<ShaderMetadata> shaders;
};

class EI_BindSet {
  public:
  // NOTE: EI_SRV & EI_UAV are EI_Resource*
  std::vector<EI_SRV> srvs; // raw pointers to resources
  std::vector<EI_UAV> uavs; // this could backfire easily..
  // int     nSRVs;
  // EI_SRV* srvs;
  // int     nUAVs;
  // EI_UAV* uavs;
  void*   values;
  int     nBytes;
};

class EI_PSO {
  public:
  glUtils::Shader* shader = nullptr;
  glUtils::DrawParameters draw_params;
};

class EI_IndexBuffer {
  public:
  std::string name;
  GLenum buffer_el_type; // GL_UNSIGNED_INT, GL_UNSIGNED_SHORT etc.
  // u32 m_nIndexSize;       ///< Individual index element size (element size)
  u32 m_nIndexCount;      ///< Number of indices in this index buffer (elements count)
  glUtils::RawBuffer buffer;
  GLuint vao_gl_id;
};

namespace glTFx {

  EI_Device* GetDevice();
  EI_CommandContextRef GetContext();
  void load_tfx_shader (glUtils::Shader&, const glUtils::ShaderTexts& filenames);

  /** Has to be called during init somehow */
  void init_TFx_callbacks();

}
