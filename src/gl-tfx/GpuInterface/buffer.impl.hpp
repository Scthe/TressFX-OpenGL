static void assert_is_memory_buffer(const EI_Resource& res) {
  bool is_counter_buf = res.type == TFx_ResourceType::StructuredBufferRW_WithAtomicCounter;
  bool is_buff = res.type == TFx_ResourceType::StructuredBufferRW
              || res.type == TFx_ResourceType::StructuredBufferR;
  GFX_FAIL_IF(is_counter_buf, "Memory mapping buffer with counter!"
      " This could get complicated, implement when know circumstances.");
  GFX_FAIL_IF(!is_buff, "Tried to map/unmap/copy resource that is not a buffer");
}

void TFx_cbCopy(EI_CommandContextRef pContext, EI_StructuredBuffer& from, EI_StructuredBuffer& to) {
  GFX_FAIL("TFx_cbCopy");
}

// Map gets a pointer to upload heap / mapped memory.
// Unmap issues the copy.
// This is only ever used for the initial upload.
//
void* TFx_cbMap(EI_CommandContextRef pContext, EI_StructuredBuffer& sb) {
  assert_is_memory_buffer(sb);
  LOGD << "[TFx_cbMap] mapping: '" << sb.name << "' (bytes=" << sb.buffer.bytes << ")";
  return glMapNamedBuffer(sb.buffer.gl_id, GL_READ_WRITE);
}

bool TFx_cbUnmap(EI_CommandContextRef pContext, EI_StructuredBuffer& sb) {
  assert_is_memory_buffer(sb);
  LOGD << "[TFx_cbMap] UN_mapping: '" << sb.name << "'";
  return glUnmapNamedBuffer(sb.buffer.gl_id);
}

/*
void SuCopy(EI_CommandContextRef pContext, EI_StructuredBuffer& from, EI_StructuredBuffer& to) {
  (void)pContext;
  to.resource->CopyResource(*from.resource);
}

// Map gets a pointer to upload heap / mapped memory.
// Unmap issues the copy.
// This is only ever used for the initial upload.
//
void* SuMap(EI_CommandContextRef pContext, EI_StructuredBuffer& sb) {
  SU_ASSERT(sb.resource->GetType() == SuGPUResource::GPU_RESOURCE_BUFFER);
  SuGPUBuffer* pBuffer = static_cast<SuGPUBuffer*>(sb.resource.get());
  SuHandle handle = pBuffer->Map(SuGPUResource::MAP_WRITE_DISCARD);
  return (void*)handle;
}

bool SuUnmap(EI_CommandContextRef pContext, EI_StructuredBuffer& sb) {
  return sb.resource->Unmap();
}
*/
