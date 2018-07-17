#include "GpuInterface/TFxGpuInterface.hpp"

namespace glTFx::debug {

  void debug_bindSet (const AMD::TressFXBindSet&);
  std::string debug_EI_Resource (const EI_Resource&);
  void debug_indexBuffer (AMD::uint32 indexCount, const EI_StringHash objectName);
  void debug_DrawParams (const AMD::EI_IndexedDrawParams&);
  void debug_layout (const AMD::TressFXLayoutDescription&);
  std::string debug_ShaderResource (const ShaderResource&);
  void debug_shader(const glUtils::Shader&);
  void debug_StructuredBuffer(const char* fnName, const EI_StringHash resourceName,
      const AMD::uint32 structSize, const AMD::uint32 structCount);

}