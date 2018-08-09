TressFXAsset.cpp
  1) instrumented with debug statements
  2) add '#include <cstring>', otherwise memset is undefined


TressFXSimulation.cpp
  1) in TressFXSimulation::Simulate removed bindings. In OpenGL
     it is required to call glUseProgram before settings uniforms.
     This can only be done inside subsequent EI_Dispatch calls,
     so the binds were moved there.


TressFXLayouts.cpp
  1) in CreateSimLayout2() removed SRV:
      * g_HairStrandType (was unused?)
      * g_BoneSkinningData (no skinning support)
     NUM_SRVS was adjusted accordingly (7 to 5)


TressFXHairObject.cpp
  1) debug statements e.g. in InitialDataUpload when copying skipped data
     (skinning, simulation)
  2) Removed references to unused SRVs(see TressFXLayouts.cpp):
      * mHairStrandTypeBuffer
      * mBoneSkinningDataBuffer
     EI_SRV SRVs[5]; // adjusted accordingly
  3) Added:
      * `EI_BindSet& GetSimBindSet() { return *m_pSimBindSet; }`
        to TressFXHairObject class. Original code uses:
        `friend class TressFXSimulation;`
