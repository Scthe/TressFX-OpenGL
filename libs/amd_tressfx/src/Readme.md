TressFXAsset.cpp:
  add '#include <cstring>', otherwise memset is undefined

TressFXAsset.cpp
  instrumented with debug statements

TressFXHairObject.cpp
  debug statements e.g. in InitialDataUpload when copying skipped data (skinning, simulation)
