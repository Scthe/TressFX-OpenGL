// START _SimQuat.glsl

vec4 MakeQuaternion(float angle_radian, vec3 axis)
{
    // create quaternion using angle and rotation axis
    vec4 quaternion;
    float halfAngle = 0.5f * angle_radian;
    float sinHalf = sin(halfAngle);

    quaternion.w = cos(halfAngle);
    quaternion.xyz = sinHalf * axis.xyz;

    return quaternion;
}

// Makes a quaternion from a 4x4 column major rigid transform matrix. Rigid transform means that rotational 3x3 sub matrix is orthonormal.
// Note that this function does not check the orthonormality.
vec4 MakeQuaternion(/*column_major*/ mat4 m)
{
	vec4 q;
	float trace = m[0][0] + m[1][1] + m[2][2];

	if (trace > 0.0f)
	{
		float r = sqrt(trace + 1.0f);
		q.w = 0.5 * r;
		r = 0.5 / r;
		q.x = (m[1][2] - m[2][1])*r;
		q.y = (m[2][0] - m[0][2])*r;
		q.z = (m[0][1] - m[1][0])*r;
	}
	else
	{
		int i = 0, j = 1, k = 2;

		if (m[1][1] > m[0][0])
		{
			i = 1; j = 2; k = 0;
		}
		if (m[2][2] > m[i][i])
		{
			i = 2; j = 0; k = 1;
		}

		float r = sqrt(m[i][i] - m[j][j] - m[k][k] + 1.0f);

		float qq[4];

		qq[i] = 0.5f * r;
		r = 0.5f / r;
		q.w = (m[j][k] - m[k][j])*r;
		qq[j] = (m[j][i] + m[i][j])*r;
		qq[k] = (m[k][i] + m[i][k])*r;

		q.x = qq[0]; q.y = qq[1]; q.z = qq[2];
	}

	return q;
}

vec4 InverseQuaternion(vec4 q)
{
    float lengthSqr = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;

    if ( lengthSqr < 0.001 )
        return vec4(0, 0, 0, 1.0f);

    q.x = -q.x / lengthSqr;
    q.y = -q.y / lengthSqr;
    q.z = -q.z / lengthSqr;
    q.w = q.w / lengthSqr;

    return q;
}

vec3 MultQuaternionAndVector(vec4 q, vec3 v)
{
    vec3 uv, uuv;
    vec3 qvec = vec3(q.x, q.y, q.z);
    uv = cross(qvec, v);
    uuv = cross(qvec, uv);
    uv *= (2.0f * q.w);
    uuv *= 2.0f;

    return v + uv + uuv;
}

vec4 MultQuaternionAndQuaternion(vec4 qA, vec4 qB)
{
    vec4 q;

    q.w = qA.w * qB.w - qA.x * qB.x - qA.y * qB.y - qA.z * qB.z;
    q.x = qA.w * qB.x + qA.x * qB.w + qA.y * qB.z - qA.z * qB.y;
    q.y = qA.w * qB.y + qA.y * qB.w + qA.z * qB.x - qA.x * qB.z;
    q.z = qA.w * qB.z + qA.z * qB.w + qA.x * qB.y - qA.y * qB.x;

    return q;
}

vec4 NormalizeQuaternion(vec4 q)
{
	vec4 qq = q;
	float n = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;

	if (n < 1e-10f)
	{
		qq.w = 1;
		return qq;
	}

	qq *= 1.0f / sqrt(n);
	return qq;
}

// Compute a quaternion which rotates u to v. u and v must be unit vector.
vec4 QuatFromTwoUnitVectors(vec3 u, vec3 v)
{
	float r = 1.f + dot(u, v);
	vec3 n;

	// if u and v are parallel
	if (r < 1e-7)
	{
		r = 0.0f;
		n = abs(u.x) > abs(u.z) ? vec3(-u.y, u.x, 0.f) : vec3(0.f, -u.z, u.y);
	}
	else
	{
		n = cross(u, v);
	}

	vec4 q = vec4(n.x, n.y, n.z, r);
	return NormalizeQuaternion(q);
}

// END _SimQuat.glsl
