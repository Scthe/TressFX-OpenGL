#include "../../include/pch.hpp"
#include "TFxHairStrands.hpp"
#include "print_me.hpp"

#include "../../libs/amd_tressfx/include/AMD_Types.h"
#include "../../libs/amd_tressfx/include/TressFXCommon.h"
#include "../../libs/amd_tressfx/include/TressFXGPUInterface.h"
#include "GpuInterface/TFxGpuInterface.hpp"
// #include "../../libs/amd_tressfx/include/TressFXAsset.h"
#include "../../libs/amd_tressfx/include/TressFXHairObject.h"
#include "../../libs/amd_tressfx/include/TressFXSimulation.h"

// #include "SuAnimatedModel.h"
// #include "SkeletonInterface.h"
// #include "SushiGPUInterface.h"


namespace glTFx {

  void TFxHairStrands::initialize(
      const char*const modelName,
      const char*const hairObjectName,
      const char*const tfxFilePath,
      // const char*const   tfxboneFilePath,
      int numFollowHairsPerGuideHair,
      float tipSeparationFactor)
  {
    LOGD << "Loading '" << modelName << "." << hairObjectName << "' from '" << tfxFilePath << "'";

    EI_Device* pDevice = GetDevice();
    EI_CommandContextRef uploadCommandContext = GetContext();

    TressFXHairObject* hairObject = new TressFXHairObject;
    AMD::TressFXAsset* asset = new AMD::TressFXAsset();

    // Load *.tfx
    // NOTE: asset will be destroyed after we upload data to GPU
    FILE * fp = fopen(tfxFilePath, "rb");
    GFX_FAIL_IF(!fp, ".tfx file could not be found");
    auto load_ok = asset->LoadHairData(fp);
    fclose(fp);
    GFX_FAIL_IF(!load_ok, "asset->LoadHairData returned false, some AMD internal problem when loading *.tfx file?");

    asset->GenerateFollowHairs(numFollowHairsPerGuideHair, tipSeparationFactor, 1.2f);
    asset->ProcessAsset();
    glTFx::debug::debug_asset(modelName, hairObjectName, *asset);
    LOGD << "Asset processing complete, will create hairObject (hairObject->Create)";

    // Load *.tfxbone
    // SkeletonInterface skeletonData;
    // skeletonData.SetModel(modelName);
    // fp = fopen(tfxboneFilePath, "rb");
    // asset->LoadBoneData(skeletonData, fp);
    // fclose(fp);
    // m_pSkeleton = dynamic_cast<SuAnimatedModel*>(SuObjectManager::GetRef().GetObjectByName(modelName));


    // The color texture is only used as an srv.
    // But, TressFX only understands EI_Resource*, so we just wrap it in that.
    // As user of TressFX, we have defined EI_Resource to work with this.
    // EI_Resource colorTextureWrapper;
    // colorTextureWrapper.srv = colorTextureView;
    EI_SRV colorTextureWrapper = nullptr;

    hairObject->Create(asset, pDevice, uploadCommandContext, hairObjectName, colorTextureWrapper);
    m_pStrands = hairObject;

    delete asset;
  }

  void TFxHairStrands::destroy(EI_Device* pDevice) {
    if (m_pStrands) {
      m_pStrands->Destroy(pDevice);
      delete m_pStrands;
    }
    m_pStrands = nullptr;
    // m_pSkeketon is not owned by this object.
  }

  // <editor-fold skinning&simulation>

  void TFxHairStrands::TransitionSimToRendering(EI_CommandContextRef context) {
    m_pStrands->GetPosTanCollection().TransitionSimToRendering(context);
  }

  void TFxHairStrands::TransitionRenderingToSim(EI_CommandContextRef context) {
    m_pStrands->GetPosTanCollection().TransitionRenderingToSim(context);
  }

  /*
  void TFxHairStrands::update_bones(EI_CommandContextRef context) {
    const SuArray<SuMatrix4>& boneMatrices = m_pSkeleton->GetSkinningMatrices();
    const float* pBoneMatricesInWS = (const float32*)boneMatrices[0];

    // update bone matrices for bone skinning of first two vertices of hair strands
    m_pStrands->UpdateBoneMatrices(context, pBoneMatricesInWS, 16 * sizeof(float) * boneMatrices.size());
  }
  */

  void TFxHairStrands::simulate(EI_CommandContextRef context, TressFXSimulation* pSimulation) {
    pSimulation->Simulate(context, *m_pStrands);
  }
  // </editor-fold>

} // namespace glTFx
