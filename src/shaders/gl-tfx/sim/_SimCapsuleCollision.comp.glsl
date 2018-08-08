// START _SimCapsuleCollision.glsl

const int NUM_COLLISION_CAPSULES = 4;

uniform vec4 g_Capsules[4];

bool CapsuleCollision(vec4 curPosition, inout vec3 newPosition, vec4 capsule) {
  const float radius1 = capsule.w;
  newPosition = curPosition.xyz;

  if (!IsMovable(curPosition)) {
    return false;
  }

  vec3 delta1 = capsule.xyz - curPosition.xyz;
  if (dot(delta1, delta1) < radius1 * radius1) {
    vec3 n = normalize(-delta1);
    newPosition = radius1 * n + capsule.xyz; // :(
    return true;
  }

  return false;
}

// Resolve hair vs capsule collisions. To use this,
// set TRESSFX_COLLISION_CAPSULES to 1
bool ResolveCapsuleCollisions(inout vec4 curPosition, vec4 oldPos) {
  bool bAnyColDetected = false;

#if TRESSFX_COLLISION_CAPSULES
  vec3 newPos;

  for (int i = 0; i < NUM_COLLISION_CAPSULES; i++) {
    bool bColDetected = CapsuleCollision(curPosition, newPos, g_Capsules[i]);

    if (bColDetected) {
      curPosition.xyz = newPos;
    }

    bAnyColDetected = bColDetected ? true : bAnyColDetected;
  }
#endif

  return bAnyColDetected;
}


// END _SimCapsuleCollision.glsl
