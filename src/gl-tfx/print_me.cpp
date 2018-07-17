#include "../../../include/pch.hpp"
#include "print_me.hpp"

namespace glTFx::debug {

  void debug_bindSet (const AMD::TressFXBindSet& bindSet) {
    LOGT << "TressFXBindSet(bytes=" << bindSet.nBytes << ") {";

    for (int i = 0; i < bindSet.nSRVs; ++i) {
        LOGT << "  SRV " << bindSet.srvs[i]->name;
    }

    for (int i = 0; i < bindSet.nUAVs; ++i) {
      LOGT << "  UAV " << bindSet.uavs[i]->name;
    }

    LOGT << "  void* values; (has_values=" << (!!bindSet.values) << ")";
    LOGT << "};";
  }

  static void append_buffer_to_str (std::string& str, const glUtils::RawBuffer& buf) {
    str += "size=" + std::to_string(buf.bytes);
  }

  std::string debug_EI_Resource (const EI_Resource& res) {
    std::string s = std::string(res.name) + "(";
    switch (res.type) {
      case TFx_ResourceType::ClearableTexture2D: {
        const auto& tex = res.texture.texture;
        s += "ClearableTexture2D " + std::to_string(tex.dimensions[0])
            + "x" + std::to_string(tex.dimensions[1]);
        break;}
      case TFx_ResourceType::StructuredBufferRW_WithAtomicCounter:
        s += "BufferRW_withAtomic buffer(";
        append_buffer_to_str(s, res.buffer_with_counter.buffer);
        s += "), counter(";
        append_buffer_to_str(s, res.buffer_with_counter.counter);
        s += ")";
        break;
      case TFx_ResourceType::StructuredBufferRW:
        s += "BufferRW ";
        append_buffer_to_str(s, res.buffer);
        break;
      case TFx_ResourceType::StructuredBufferR:
        s += "BufferR ";
        append_buffer_to_str(s, res.buffer);
        break;
    }
    s += ")";
    return s;
  }

  void debug_StructuredBuffer(const char* fnName, const EI_StringHash resourceName,
    const AMD::uint32 structSize, const AMD::uint32 structCount)
  {
    LOGD  << "[" << fnName <<"] createSB " << resourceName
        << "(structSize=" << structSize << ", "
        << "structCount=" << structCount << ")";
  }

  void debug_DrawParams (const AMD::EI_IndexedDrawParams& drawParams) {
    LOGT << "drawIndexed(numIndices=" << drawParams.numIndices
         << ", numInstances=" << drawParams.numInstances << ") indices: {";
    LOGT << "  name=" << drawParams.pIndexBuffer->name;
    LOGT << "  buffer_el_type=" << drawParams.pIndexBuffer->buffer_el_type;
    LOGT << "  m_nIndexCount=" << drawParams.pIndexBuffer->m_nIndexCount;
    LOGT << "  buffer (size=" << drawParams.pIndexBuffer->buffer.bytes << " bytes)";
    LOGT << "}";
  }

  void debug_indexBuffer (AMD::uint32 indexCount, const EI_StringHash objectName) {
    LOGD << "[TFx_cbCreateIndexBuffer] IndexBuffer " << objectName
         << "(indexElSize=" << TRESSFX_INDEX_SIZE
         << ", m_nIndexCount=" << indexCount << ")";
  }

  void debug_layout (const AMD::TressFXLayoutDescription& description) {
    LOGT << "TressFXLayoutDescription {";
    for (int i = 0; i < description.nSRVs; ++i) {
        LOGT << "  SRV " << description.srvNames[i];
    }

    for (int i = 0; i < description.nUAVs; ++i) {
      LOGT << "  UAV " << description.uavNames[i];
    }

    auto& consts = description.constants;
    if (consts.nConstants > 0) {
      LOGT << "  CONST " << consts.constantBufferName <<  "{";
      for (int i = 0; i < consts.nConstants; i++) {
        LOGT << "    " << consts.parameterNames[i];
      }
      LOGT << "  }";
    }
    LOGT << "};";
  }

  std::string debug_ShaderResource (const ShaderResource& l) {
    std::string s = std::string(l.name) + "(";
    if (l.is_block) {
      s += (l.block->is_ubo ? "UBO" : "SSBO");
      s += "=" + l.block->name
        + ", gl_index=" + std::to_string(l.block->gl_index)
        + ", gl_binding=" + std::to_string(l.block->gl_binding);
    } else {
      s += "uniform=" + l.variable->name
        + ", gl_type=" + std::to_string(l.variable->gl_type)
        + ", location=" + std::to_string(l.variable->gl_location);
    }
    s += ")";
    return s;
  }

  void debug_shader(const glUtils::Shader& shader) {
    LOGT << "shader uniforms {";

    for (const auto& var : shader.used_variables) {
      LOGT << "  var " << var.name
           << "(loc=" << var.gl_location
           << ", in_block=" << (var.gl_index_of_parent_block != -1)
           << ", type=" << var.gl_type << ")";
    }

    for (const auto& var : shader.used_blocks) {
      LOGT << "  " << (var.is_ubo ? "UBO" : "SSBO") << " " << var.name
           << "(gl_index=" << var.gl_index
           << ", gl_binding=" << var.gl_binding
           << ", bytes=" << var.bytes << ")";
    }

    for (const auto& var : shader.used_atomics) {
      LOGT << "  atomic uint (binding=" << var.gl_binding
           << ", bytes=" << var.bytes << ")";
    }

    LOGT << "}";
  }

}
