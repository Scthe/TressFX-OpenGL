// START _SimCapsuleCollision.glsl

struct CollisionCapsule {
    vec4 p0; // xyz = position of capsule 0, w = radius 0
    vec4 p1; // xyz = position of capsule 1, w = radius 1
};

// Resolve hair vs capsule collisions. To use this, set TRESSFX_COLLISION_CAPSULES to 1 in both hlsl and cpp sides.
bool ResolveCapsuleCollisions(inout vec4 curPosition, vec4 oldPos, float friction = 0.4f) {
  bool bAnyColDetected = false;

#if TRESSFX_COLLISION_CAPSULES
  if (g_numCollisionCapsules.x > 0) {
    vec3 newPos;

    for (int i = 0; i < g_numCollisionCapsules.x; i++) {
      vec3 center0 = g_centerAndRadius0[i].xyz;
      vec3 center1 = g_centerAndRadius1[i].xyz;

      CollisionCapsule cc;
      cc.p0.xyz = center0;
      cc.p0.w = g_centerAndRadius0[i].w;
      cc.p1.xyz = center1;
      cc.p1.w = g_centerAndRadius1[i].w;

      bool bColDetected = CapsuleCollision(curPosition, oldPos, newPos, cc, friction);

      if (bColDetected)
          curPosition.xyz = newPos;

      bAnyColDetected = bColDetected ? true : bAnyColDetected;
    }
  }
#endif

  return bAnyColDetected;
}

//  Moves the position based on collision with capsule
bool CapsuleCollision(vec4 curPosition, vec4 oldPosition, inout vec3 newPosition, CollisionCapsule cc, float friction = 0.4f) {
    const float radius0 = cc.p0.w;
    const float radius1 = cc.p1.w;
	newPosition = curPosition.xyz;

    if ( !IsMovable(curPosition) )
        return false;

    vec3 segment = cc.p1.xyz - cc.p0.xyz;
    vec3 delta0 = curPosition.xyz - cc.p0.xyz;
    vec3 delta1 = cc.p1.xyz - curPosition.xyz;

    float dist0 = dot(delta0, segment);
    float dist1 = dot(delta1, segment);

    // colliding with sphere 1
    if (dist0 < 0.f )
    {
        if ( dot(delta0, delta0) < radius0 * radius0)
        {
            vec3 n = normalize(delta0);
            newPosition = radius0 * n + cc.p0.xyz;
            return true;
        }

        return false;
    }

    // colliding with sphere 2
    if (dist1 < 0.f )
    {
        if ( dot(delta1, delta1) < radius1 * radius1)
        {
            vec3 n = normalize(-delta1);
            newPosition = radius1 * n + cc.p1.xyz;
            return true;
        }

        return false;
    }

    // colliding with middle cylinder
    vec3 x = (dist0 * cc.p1.xyz + dist1 * cc.p0.xyz) / (dist0 + dist1);
    vec3 delta = curPosition.xyz - x;

    float radius_at_x = (dist0 * radius1 + dist1 * radius0) / (dist0 + dist1);

    if ( dot(delta, delta) < radius_at_x * radius_at_x)
    {
        vec3 n = normalize(delta);
        vec3 vec = curPosition.xyz - oldPosition.xyz;
        vec3 segN = normalize(segment);
        vec3 vecTangent = dot(vec, segN) * segN;
        vec3 vecNormal = vec - vecTangent;
        newPosition = oldPosition.xyz + friction * vecTangent + (vecNormal + radius_at_x * n - delta);
        return true;
    }

    return false;
}

// END _SimCapsuleCollision.glsl
