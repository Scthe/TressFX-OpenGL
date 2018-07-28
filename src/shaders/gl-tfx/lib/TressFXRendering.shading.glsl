// START TressFXRendering.shading

#define AMD_PI 3.1415926

/*
// We might break this down further.
cbuffer tressfxShadeParameters {
  //float       g_FiberAlpha        ; // Is this redundant with g_MatBaseColor.a?
  float g_HairShadowAlpha;
  float g_FiberRadius;
  float g_FiberSpacing;
  // TODO add padding here - one float

  vec4 g_MatBaseColor;
  vec4 g_MatKValue;

  float g_fHairKs2;
  float g_fHairEx2;
  int g_NumVerticesPerStrand;
  //float padding_
}
*/


struct HairShadeParams {
  vec3 cColor;
  float fRadius;
  float fSpacing;
  float fAlpha;
};


// Returns a vec3 which is the scale for diffuse, spec term, and colored spec term.
//
// The diffuse term is from Kajiya.
//
// The spec term is what Marschner calls "R", reflecting directly off the surface of the hair,
// taking the color of the light like a dielectric specular term.  This highlight is shifted
// towards the root of the hair.
//
// The colored spec term is caused by light passing through the hair, bouncing off the back, and
// coming back out.  It therefore picks up the color of the light.
// Marschner refers to this term as the "TRT" term.  This highlight is shifted towards the
// tip of the hair.
//
// vEyeDir, vLightDir and vTangentDir are all pointing out.
// coneAngleRadians explained below.
//
//
// hair has a tiled-conical shape along its lenght.  Sort of like the following.
//
// \    /
//  \  /
// \    /
//  \  /
//
// The angle of the cone is the last argument, in radians.
// It's typically in the range of 5 to 10 degrees (10*AMD_PI/180)
vec3 TressFX_ComputeDiffuseSpecFactors(vec3 vEyeDir, vec3 vLightDir, vec3 vTangentDir, float coneAngleRadians) {
  // From old code.
  float Kd = g_MatKValue.y;
  float Ks1 = g_MatKValue.z;
  float Ex1 = g_MatKValue.w;
  float Ks2 = g_fHairKs2;
  float Ex2 = g_fHairEx2;

  // in Kajiya's model: diffuse component: sin(t, l)
  float cosTL = (dot(vTangentDir, vLightDir));
  float sinTL = sqrt(1 - cosTL*cosTL);
  float diffuse = sinTL; // here sinTL is apparently larger than 0

  float cosTRL = -cosTL;
  float sinTRL = sinTL;
  float cosTE = (dot(vTangentDir, vEyeDir));
  float sinTE = sqrt(1 - cosTE*cosTE);

  // primary highlight: reflected direction shift towards root (2 * coneAngleRadians)
  float cosTRL_root = cosTRL * cos(2 * coneAngleRadians) - sinTRL * sin(2 * coneAngleRadians);
  float sinTRL_root = sqrt(1 - cosTRL_root * cosTRL_root);
  float specular_root = max(0, cosTRL_root * cosTE + sinTRL_root * sinTE);

  // secondary highlight: reflected direction shifted toward tip (3*coneAngleRadians)
  float cosTRL_tip = cosTRL*cos(-3 * coneAngleRadians) - sinTRL*sin(-3 * coneAngleRadians);
  float sinTRL_tip = sqrt(1 - cosTRL_tip * cosTRL_tip);
  float specular_tip = max(0, cosTRL_tip * cosTE + sinTRL_tip * sinTE);

  return vec3(Kd*diffuse, Ks1 * pow(specular_root, Ex1), Ks2 * pow(specular_tip, Ex2));
}


float fLightScale;

vec3 AccumulateHairLight(vec3 vTangent, vec3 vPositionWS, vec3 vViewWS,
  HairShadeParams params, float fExtraAmbientScale)
{
  // fExtraAmbientScale = 1

  // Light update system only counts lights that affect this object.
  int lightCount = nNumLights;

  vec3 color = vec3(0,0,0);
  vec3 V = normalize(vViewWS);
  vec3 T = normalize(vTangent);

  for (int i = 0; i < lightCount; ++i) {
    // volume lights will be treated as omni-directional.
    if (SuIsVolume(i)) {
      color += SuComputeVolumeAmbient( vPositionWS, i );
    } else {
      vec3 vLightDirWS, cLightColor;
      SuGetLightLocal(vPositionWS, i, vLightDirWS, cLightColor);

      int nLightID = nLightIndex[i]; // global ID used for shadows.

      // yes, I know this seems weird - but it's basically a way to control
      // spec vs diffuse through the light.
      float lightEmitsDiffuse = vLightParams[i].z;
      float lightEmitsSpecular = vLightParams[i].w;

      // if we get zero, we can save the BRDF eval, which is costly.
      float fShadowTerm = ComputeLightShadow(nLightID, vPositionWS);

      if (fShadowTerm > 0.00001 ) {
        vec3 L = normalize(vLightDirWS);
        vec3  reflection = TressFX_ComputeDiffuseSpecFactors(V, L, T);
        vec3 cReflectedLight = reflection.x * cLightColor * lightEmitsDiffuse * params.cColor;
        cReflectedLight += reflection.y * cLightColor * lightEmitsSpecular;
        cReflectedLight += reflection.z * cLightColor * lightEmitsSpecular * params.cColor;
        cReflectedLight *= fShadowTerm * fLightScale;// * 0.16;

        color += max(vec3(0,0,0), cReflectedLight);
      }
    }
  }

  return color;
}

vec4 ComputeSushiRGBA(vec2 pixelCoord, float depth, vec4 vTangentCoverage, vec3 baseColor) {
  vec3 vTangent = 2.0 * vTangentCoverage.xyz - 1.0;
  vec3 vNDC = ScreenToNDC(vec3(pixelCoord, depth), g_vViewport);
  vec3 vPositionWS = NDCToWorld(vNDC, g_mInvViewProj);
  vec3 vViewWS = g_vEye - vPositionWS;

  HairShadeParams params;
  params.cColor = baseColor;
  params.fRadius = g_FiberRadius;
  params.fSpacing = g_FiberSpacing;
  params.fAlpha = g_HairShadowAlpha;

  vec3 color = AccumulateHairLight(vTangent, vPositionWS, vViewWS, params);
  return vec4(color, vTangentCoverage.w);
}
#define HEAD_SHADING ComputeSushiRGBA
#define TAIL_SHADING ComputeSushiRGBA



// END TressFXRendering.shading
