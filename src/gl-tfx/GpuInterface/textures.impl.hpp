EI_Resource* TFx_cbCreate2D(EI_Device* pContext,
    const size_t     width,
    const size_t     height,
    const size_t     arraySize,
    EI_StringHash    strHash)
{
  LOGD << "[TFx_cbCreate2D] Create Texture2D " << strHash << "("
      << "w=" << width << ", "
      << "h=" << height << ", "
      << "arraySize=" << arraySize
      << ")";

  // TODO this is actually a buffer? it tries to allocate 'PPLLHeads'.
  //      Though might as well be texture single uint per channel.
  //      OTOH, PPLL heads may need to be writable..
  //      maybe add: if (name=="PPLLHeads")?
  //      or the whole ImageStore thingy?

  GFX_FAIL_IF(arraySize != 1, "You sure You wanted texture with arraySize=", arraySize,
      " instead of 1? Why would AMD::TressFX need that?");

  EI_Resource* pRW2D = new EI_Resource;
  pRW2D->type = TFx_ResourceType::ClearableTexture2D;
  pRW2D->name = strHash;

  auto& texture = pRW2D->texture.texture;
  texture = glUtils::Texture(); // pRW2D->texture is part of the union, which means that content is freaking garbage. reset it now!
  texture.gl_type = GL_TEXTURE_2D;
  texture.dimensions[0] = static_cast<u32>(width);
  texture.dimensions[1] = static_cast<u32>(height);
  texture.dimensions[2] = 0; // ignore arraySize
  texture.sized_pixel_format = GL_R32UI;
  texture.mipmap_levels = 0; // only base
  texture.multisample = 1;
  texture.opts.filter_min = glUtils::TextureFilterMin::Nearest;
  texture.opts.filter_mag = glUtils::TextureFilterMag::Nearest;
  glUtils::allocate_texture(texture, true);

  // create clear buffer - when clearing the PPLL head texture,
  // we are just going to copy this_buffer->texture
  // using glBindBuffer(GL_PIXEL_UNPACK_BUFFER, this_buffer);
  // @see https://github.com/gangliao/Order-Independent-Transparency-GPU/blob/master/source%20code/src/sceneoit.cpp#L256
  auto& clear_buffer = pRW2D->texture.clear_data_buffer;
  u32 clear_buffer_size = width * height,
      clear_buffer_bytes = clear_buffer_size * sizeof(GLuint);
  clear_buffer = glUtils::malloc(clear_buffer_bytes, glUtils::BufferUsagePattern::Default);
  std::vector<GLuint> headPtrClearBuf(clear_buffer_size, TRESSFX_PPLL_NULL_PTR);
  glUtils::write(clear_buffer, {0, clear_buffer_bytes}, &headPtrClearBuf[0]);

  // done!
  return pRW2D;
}

EI_Resource* TFx_cbCreateRT(EI_Device* pContext,
    const size_t     width,
    const size_t     height,
    const size_t     channels,
    EI_StringHash    strHash,
    float clearR, float clearG, float clearB, float clearA)
{
  GFX_FAIL("TFx_cbCreateRT");
  return nullptr;
}

void TFx_cbClear2D(EI_CommandContext* pContext, EI_RWTexture2D* pResource,
  AMD::uint32 clearValue)
{
  LOGD << "[TFx_cbClear2D] clear R32UI '" << pResource->name
       << "' texture with (clear_value=" << clearValue << ")";

  GFX_FAIL_IF(clearValue != TRESSFX_PPLL_NULL_PTR, "Texture was prepared to be cleared"
      " with TRESSFX_PPLL_NULL_PTR(", TRESSFX_PPLL_NULL_PTR, "), but requested to clear"
      " with ", clearValue);
  GFX_FAIL_IF(pResource->type != TFx_ResourceType::ClearableTexture2D,
      "Tried to clear resource that is not ClearableTexture2D. How?");

  ClearableTexture& clearable_tex = pResource->texture;
  auto& tex = clearable_tex.texture;

  glBindBuffer(glUtils::BufferBindType::WriteIntoTextureBuffer, clearable_tex.clear_data_buffer.gl_id);
  glTextureSubImage2D(tex.gl_id, 0, //
    0, 0, tex.dimensions[0], tex.dimensions[1],
    GL_RED_INTEGER,
    GL_UNSIGNED_INT, nullptr); // data is nullptr, will take from GL_PIXEL_UNPACK_BUFFER
  glBindBuffer(glUtils::BufferBindType::WriteIntoTextureBuffer, 0);

  // (void)pContext;
  // uint32_t clearVector[4];
  // clearVector[0] = clearVector[1] = clearVector[2] = clearVector[3] = clearValue;
  // pResource->uav->ClearUInt(clearVector);
  // pResource->resource->UAVBarrier();
}

/*
EI_Resource* SuCreate2D(EI_Device* pContext,
    const size_t     width,
    const size_t     height,
    const size_t     arraySize,
    EI_StringHash    strHash)
{
    (void)pContext;

    EI_Resource* pRW2D = new EI_Resource;

    SuGPUTexture2DArrayPtr texPtr = SuGPUResourceManager::GetPtr()->CreateTexture2DArray(
        SuGPUResource::GPU_RESOURCE_DYNAMIC,
        SuGPUResource::BIND_RENDER_TARGET | SuGPUResource::BIND_SHADER_RESOURCE | SuGPUResource::BIND_UNORDERED_ACCESS,
        0,
        SuGPUResourceFormat::SU_FORMAT_R32_UINT,
        SuGPUTexture::MIP_NONE,
        1,  // Mip levels
        (uint16)width, (uint16)height, (uint16)arraySize,
        1,
        0,  // sample count and quality
        SuMemoryBufferPtr(0),
        strHash);

    pRW2D->resource = texPtr.cast<SuGPUResource>();
    pRW2D->srv = texPtr->GetDefaultSamplingView();
    SuGPUResourceDescription     desc = texPtr->GetDefaultResourceDesc();
    SuGPUResourceViewDescription viewDesc = SuGPUResourceViewDescription(
            SU_UNORDERED_ACCESS_VIEW,
            texPtr->GetFormat(),
            SuGPUResource::GPU_RESOURCE_TEXTURE_2D_ARRAY,
            desc);
    pRW2D->uav = texPtr->GetUnorderedAccessView(viewDesc);
    pRW2D->rtv = SuGPURenderableResourceViewPtr(0);

    return pRW2D;
}

EI_Resource* SuCreateRT(EI_Device* pContext,
    const size_t     width,
    const size_t     height,
    const size_t     channels,
    EI_StringHash    strHash,
    float            clearR,
    float            clearG,
    float            clearB,
    float            clearA)
{
    EI_Resource* pRW2D = new EI_Resource;

    SuGPUResourceFormat format;
    if (channels == 1)
    {
        format = SU_FORMAT_R16_FLOAT;
    }
    else if (channels == 2)
    {
        format = SU_FORMAT_R16G16_FLOAT;
    }
    else if (channels == 4)
    {
        format = SU_FORMAT_R16G16B16A16_FLOAT;
    }

    SuGPUTexture2DArrayClearInfo clearInfo;
    clearInfo.viewFormat = format;
    clearInfo.color = SuVector4(clearR, clearG, clearB, clearA);

    SuGPUTexture2DArrayPtr texPtr = SuGPUResourceManager::GetPtr()->CreateTexture2DArray(SuGPUResource::GPU_RESOURCE_DYNAMIC,
        SuGPUResource::BIND_RENDER_TARGET |
        SuGPUResource::BIND_SHADER_RESOURCE,
        0,
        format,
        SuGPUTexture::MIP_NONE,
        1,  // Mip levels
        (uint16)width,
        (uint16)height,
        1,
        1,
        0,  // sample count and quality
        SuMemoryBufferPtr(0),
        strHash,
        NULL,
        0,
        &clearInfo);

    pRW2D->resource = texPtr.cast<SuGPUResource>();

    pRW2D->srv = texPtr->GetDefaultSamplingView();
    pRW2D->uav = SuGPUUnorderedAccessViewPtr(0);
    SuGPUResourceDescription     desc = pRW2D->resource->GetDefaultResourceDesc();
    SuGPUResourceViewDescription viewDesc =
        SuGPUResourceViewDescription(SU_RENDERABLE_VIEW,
            texPtr->GetFormat(),
            SuGPUResource::GPU_RESOURCE_TEXTURE_2D_ARRAY,
            desc);
    pRW2D->rtv = texPtr->GetRenderableView(viewDesc);

    return pRW2D;
}
*/
