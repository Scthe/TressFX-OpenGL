// unused?

float fLightScale;

float3 AccumulateHairLight(float3 vTangent, float3 vPositionWS, float3 vViewWS, HairShadeParams params, float fExtraAmbientScale = 1)
{

  // Light update system only counts lights that affect this object.
  int lightCount = nNumLights;

  float3 color = float3(0,0,0);
      float3 V = normalize(vViewWS);
      float3 T = normalize(vTangent);

  for(int i=0; i < lightCount; ++i)
  {
    // volume lights will be treated as omni-directional.
    if( SuIsVolume( i ) )
    {
      color += SuComputeVolumeAmbient( vPositionWS, i );
    }
    else
    {
      float3 vLightDirWS, cLightColor;
      SuGetLightLocal( vPositionWS, i,  vLightDirWS, cLightColor);

      int nLightID = nLightIndex[i]; // global ID used for shadows.

              // yes, I know this seems weird - but it's basically a way to control
              // spec vs diffuse through the light.
              float lightEmitsDiffuse = vLightParams[i].z;
              float lightEmitsSpecular =  vLightParams[i].w;

      // if we get zero, we can save the BRDF eval, which is costly.
      float fShadowTerm = ComputeLightShadow( nLightID, vPositionWS);

      if(fShadowTerm > 0.00001 )
      {
        float3 L = normalize(vLightDirWS);

                  float3  reflection = TressFX_ComputeDiffuseSpecFactors(V, L, T);

        float3 cReflectedLight = reflection.x * cLightColor * lightEmitsDiffuse * params.cColor;
                  cReflectedLight += reflection.y * cLightColor * lightEmitsSpecular;
                  cReflectedLight += reflection.z * cLightColor * lightEmitsSpecular * params.cColor;
                  cReflectedLight *= fShadowTerm * fLightScale;// * 0.16;

        color += max( float3(0,0,0), cReflectedLight);

      }
    }
  }

  return color;

}
