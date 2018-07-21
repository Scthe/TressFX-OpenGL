// #include "SushiGPUInterface.h"
// #include "SuEffect.h"
// #include "SuEffectParameter.h"
// #include "SuEffectTechnique.h"
// #include "SuGPUIndexBuffer.h"
// #include "SuRenderManager.h"

// for fullscreen.
// #include "SuVector2.h"
// #include "SuGPUVertexBuffer.h"

// For engine hiding in sample code.
// #include "SuObjectManager.h"

#include "../../../include/pch.hpp"
#include "TFxGpuInterface.hpp"
#include "../print_me.hpp"
#include "../../State.hpp"

// needed to allocate clear buffer for PPLL heads buffer
#include "../../../libs/amd_tressfx/include/TressFXPPLL.h"
// needed to dispatch compute shader
#include "../../../libs/amd_tressfx/include/TressFXHairObject.h"
#include "../../../libs/amd_tressfx/include/TressFXLayouts.h"

// typedef SuCommandListPtr   EI_CommandContext;

static std::string SHADER_PATH = "src/shaders/generated/gl-tfx/";

// Callbacks impl:
EI_Resource::EI_Resource (){}

#pragma GCC diagnostic ignored "-Wunused-parameter"
extern "C" {

  #include "barriers.impl.hpp"
  #include "bindSet.impl.hpp"
  #include "buffer.impl.hpp"
  #include "compute.impl.hpp"
  #include "structuredBuffers.impl.hpp"
  #include "textures.impl.hpp"
  #include "draw.impl.hpp"
  #include "fileIO.impl.hpp"
  #include "layout.impl.hpp"

  void TFx_cbError(EI_StringHash message) {
      // SuLogWarning(message);
      LOGW << "[TFx_cbError] " << message;
  }

  void TFx_cbDestroy(EI_Device* pDevice, EI_Resource* pRW2D) {
    switch (pRW2D->type) {
      case TFx_ResourceType::ClearableTexture2D:
        LOGD << "[TFx_cbDestroy] " << pRW2D->name << "(type=ClearableTexture2D)";
        glUtils::destroy(pRW2D->texture.texture);
        glUtils::destroy(pRW2D->texture.clear_data_buffer);
        break;
      case TFx_ResourceType::StructuredBufferRW_WithAtomicCounter:
        LOGD << "[TFx_cbDestroy] " << pRW2D->name << "(type=StructuredBufferRW_WithAtomicCounter)";
        glUtils::destroy(pRW2D->buffer_with_counter.buffer);
        glUtils::destroy(pRW2D->buffer_with_counter.counter);
        break;
      case TFx_ResourceType::StructuredBufferRW:
        LOGD << "[TFx_cbDestroy] " << pRW2D->name << "(type=StructuredBufferRW)";
        glUtils::destroy(pRW2D->buffer);
        break;
      case TFx_ResourceType::StructuredBufferR:
        LOGD << "[TFx_cbDestroy] " << pRW2D->name << "(type=StructuredBufferR)";
        glUtils::destroy(pRW2D->buffer);
        break;
    }
    /*
    SuGPUResourceManager* pResourceManager = GetResourceManager(pDevice);

    pRW2D->uav = SuGPUUnorderedAccessViewPtr(0);
    pRW2D->srv = SuGPUSamplingResourceViewPtr(0);
    pRW2D->rtv = SuGPURenderableResourceViewPtr(0);
    pRW2D->resource = SuGPUResourcePtr(0);
    delete pRW2D;
    */
  }

} // extern "C"
#pragma GCC diagnostic warning "-Wunused-parameter"


static std::string get_file_contents(const char *filename);

namespace glTFx {

  EI_Device* GetDevice () {
    return nullptr;
  }

  EI_CommandContextRef GetContext() {
    static EI_CommandContext ctx;
    return ctx;
    // EI_CommandContext* p = nullptr;
    // return *p; // ugh
  }

  void init_TFx_callbacks () {
    // TFx_cb
    AMD::g_Callbacks.pfMalloc = malloc;
    AMD::g_Callbacks.pfFree = free;
    AMD::g_Callbacks.pfError = TFx_cbError;

    // fileIO
    AMD::g_Callbacks.pfRead = TressFX_DefaultRead;
    AMD::g_Callbacks.pfSeek = TressFX_DefaultSeek;

    // layout
    AMD::g_Callbacks.pfCreateLayout = TFx_cbCreateLayout; //SuCreateLayout;
    AMD::g_Callbacks.pfDestroyLayout = TFx_cbDestroyLayout; //SuDestroyLayout;

    // createStructuredBuffer
    AMD::g_Callbacks.pfCreateReadOnlySB = TFx_cbCreateReadOnlySB; //SuCreateReadOnlySB;
    AMD::g_Callbacks.pfCreateReadWriteSB = TFx_cbCreateReadWriteSB; //SuCreateReadWriteSB;
    AMD::g_Callbacks.pfCreateCountedSB = TFx_cbCreateCountedSB; //SuCreateCountedSB;

    // buffer-like
    AMD::g_Callbacks.pfCopy = TFx_cbCopy; //SuCopy;
    AMD::g_Callbacks.pfMap = TFx_cbMap; //SuMap;
    AMD::g_Callbacks.pfUnmap = TFx_cbUnmap; //SuUnmap;

    // resource mgmt
    AMD::g_Callbacks.pfDestroySB = TFx_cbDestroy;

    // createTexture
    AMD::g_Callbacks.pfCreateRT = TFx_cbCreateRT; //SuCreateRT;
    AMD::g_Callbacks.pfCreate2D = TFx_cbCreate2D; //SuCreate2D;

    // bindSet
    AMD::g_Callbacks.pfCreateBindSet = TFx_cbCreateBindSet; //SuCreateBindSet;
    AMD::g_Callbacks.pfDestroyBindSet = TFx_cbDestroyBindSet; //SuDestroyBindSet;
    AMD::g_Callbacks.pfBind = TFx_cbBind; //SuBind;

    // compute
    AMD::g_Callbacks.pfCreateComputeShaderPSO = TFx_cbCreateComputeShaderPSO; //SuCreateComputeShaderPSO;
    AMD::g_Callbacks.pfDestroyPSO = TFx_cbDestroyPSO; //SuDestroyPSO;
    AMD::g_Callbacks.pfDispatch = TFx_cbDispatch; //SuDispatch;

    // draw
    AMD::g_Callbacks.pfCreateIndexBuffer = TFx_cbCreateIndexBuffer; //SuCreateIndexBuffer;
    AMD::g_Callbacks.pfDestroyIB = TFx_cbDestroyIB; //SuDestroyIB;
    AMD::g_Callbacks.pfDraw = TFx_cbDrawIndexedInstanced; //SuDrawIndexedInstanced;

    // misc
    AMD::g_Callbacks.pfClearCounter = TFx_cbClearCounter;
    AMD::g_Callbacks.pfClear2D = TFx_cbClear2D;
    AMD::g_Callbacks.pfSubmitBarriers = TFx_cbSubmitBarriers; //SuSubmitBarriers;
  }

  void load_tfx_shader (glUtils::Shader& shader, const glUtils::ShaderTexts& filenames) {
    glTFx::debug::debug_shader_texts("Compiling shader from files", filenames, logger::LogLevel::Trace);

    std::string contents[6];
    glUtils::ShaderTexts shader_texts;

    #define SET_TEXT(IDX, NAME) \
      if (filenames.NAME != nullptr) { \
        contents[IDX] = get_file_contents(filenames.NAME); \
        shader_texts.NAME = contents[IDX].c_str(); \
      }

      SET_TEXT(0, vertex);
      SET_TEXT(1, fragment);
      SET_TEXT(2, geometry);
      SET_TEXT(3, tessellation_control);
      SET_TEXT(4, tessellation_evaluation);
      SET_TEXT(5, compute);
    #undef SET_TEXT

    glUtils::ShaderErrorsScratchpad es;
    shader = glUtils::create_shader(shader_texts, es);

    if (!shader.is_created()) {
      LOGE << "Shader create error:" << es.msg;
      glTFx::debug::debug_shader_texts("From files", filenames, logger::LogLevel::Error);
      GFX_FAIL("Shader compile/link failed, see above for more details");
    }
  }

}

// shader load libs:
#include <fstream>
#include <cerrno>

std::string get_file_contents(const char *filename) {
  std::string full_path = SHADER_PATH + filename;
  std::ifstream in(full_path, std::ios::in | std::ios::binary);
  if (in) {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return contents;
  } else {
    LOGE << "Error reading file content: '" << filename << "'";
    throw(errno);
  }
}
