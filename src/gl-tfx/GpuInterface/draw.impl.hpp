// Initialize and leave in state for use as index buffer.
// Index buffers are for either triangle-strip hair, or line segments.
EI_IndexBuffer* TFx_cbCreateIndexBuffer(EI_Device* context,
    AMD::uint32 indexCount, void* pInitialData, EI_StringHash objectName)
{
  glTFx::debug::debug_indexBuffer(indexCount, objectName);
  GFX_FAIL_IF(TRESSFX_INDEX_SIZE != 4, "Assumed that provided index data is in GL_UNSIGNED_INT. It is not..");

  EI_IndexBuffer* ib = new EI_IndexBuffer;
  ib->name = objectName;
  ib->buffer_el_type = GL_UNSIGNED_INT;
  ib->m_nIndexCount = indexCount;

  u32 data_size = TRESSFX_INDEX_SIZE * indexCount;
  ib->buffer = glUtils::malloc(data_size, glUtils::BufferUsagePattern::Default);
  glUtils::write(ib->buffer, {0, data_size}, pInitialData);

  // (there is no need for such 'complicated' VAO, but might as well,
  // cause weirder things happend)
  glGenVertexArrays(1, &ib->vao_gl_id);
  glBindVertexArray(ib->vao_gl_id);
  glVertexAttribI1i(0, 0); // index, default_value
	glBindVertexArray(0);

  return ib;
}

void TFx_cbDestroyIB(EI_Device* pDevice, EI_IndexBuffer* pBuffer) {
  LOGD << "[TFx_cbDestroyIB] IndexBuffer " << pBuffer->name;
  glUtils::destroy(pBuffer->buffer);
  delete pBuffer;
}

void TFx_cbDrawIndexedInstanced(EI_CommandContextRef commandContext,
    EI_PSO& pso, AMD::EI_IndexedDrawParams& drawParams)
{
  LOGI << "[TFx_cbDrawIndexedInstanced]";
  glTFx::debug::debug_DrawParams(drawParams);

  GFX_FAIL_IF(!commandContext.state, "EI_CommandContext should have been initialized"
      "with ptr to GlobalState before TFx_cbDrawIndexedInstanced() was called");

  GFX_FAIL_IF(!pso.shader, "EI_PSO should have been initialized"
      "with ptr to shader before TFx_cbDrawIndexedInstanced() was called");

  auto& state = *commandContext.state;
  EI_IndexBuffer& idx_buffer = *drawParams.pIndexBuffer;

  // set PSO
  state.update_draw_params(pso.draw_params);
  glBindVertexArray(idx_buffer.vao_gl_id);
  glBindBuffer(glUtils::BufferBindType::IndexBuffer, idx_buffer.buffer.gl_id);

  // draw
  auto verices_count = drawParams.numIndices;
  glDrawElementsInstanced(GL_TRIANGLES, verices_count,
      idx_buffer.buffer_el_type, 0, drawParams.numInstances);
}


/*
// Initialize and leave in state for use as index buffer.
// Index buffers are for either triangle-strip hair, or line segments.
EI_IndexBuffer* SuCreateIndexBuffer(EI_Device* context,
    AMD::uint32      indexCount,
    void*            pInitialData, EI_StringHash objectName)
{
    (void)objectName;

    SuGPUIndexBufferPtr* pIBPtr = new SuGPUIndexBufferPtr;
    SuGPUIndexBufferPtr& ib = *pIBPtr;
    SuGPUResourceManager* pResourceManager = (SuGPUResourceManager*)context;

    ib = SuGPUResourceManager::GetPtr()->CreateResourceIndex(
        SuGPUResource::GPU_RESOURCE_DYNAMIC, TRESSFX_INDEX_SIZE, indexCount, SuMemoryBufferPtr(0));

    uint8* p = (uint8*)ib->Map(SuGPUResource::MAP_WRITE_DISCARD);
    memcpy(p, pInitialData, TRESSFX_INDEX_SIZE * indexCount);
    ib->Unmap();
    ib->Transition(SuGPUResource::STATE_COPY_DEST, SuGPUResource::STATE_INDEX_BUFFER);

    return (EI_IndexBuffer*)pIBPtr;
}

void SuDestroyIB(EI_Device* pDevice, EI_IndexBuffer* pBuffer)
{
    delete (SuGPUIndexBufferPtr*)pBuffer;
}


void SuDrawIndexedInstanced(EI_CommandContextRef     commandContext,
    EI_PSO&                pso,
    AMD::EI_IndexedDrawParams& drawParams)
{
    (void)commandContext;
    //const SuEffectTechnique* pTechnique = pso;
    const SuEffectTechnique& technique = (const SuEffectTechnique&)(pso);
    SuEffect*                pEffect = const_cast<SuEffect*>(technique.GetParent());
    SuGPUIndexBufferPtr pIndexBuffer = *((SuGPUIndexBufferPtr*)drawParams.pIndexBuffer);

    pEffect->BindIndexBuffer(pIndexBuffer.get());

    uint32 numPasses;
    bool   techniqueFound = pEffect->Begin(&technique, &numPasses);

    if (techniqueFound)
    {
        for (uint32 i = 0; i < numPasses; ++i)
        {
            pEffect->BeginPass(i);
            SuRenderManager::GetRef().DrawIndexedInstanced(SuRenderManager::TRIANGLE_LIST,
                0,
                0, // Doesn't matter
                0, // Doesn't matter
                drawParams.numIndices,
                0,
                TRESSFX_INDEX_SIZE,
                drawParams.numInstances,
                0);
            pEffect->EndPass();
        }
        pEffect->End();
    }
}
*/
