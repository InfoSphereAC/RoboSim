/*
 *  Vec4.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 07.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Vec4.h"

matrix matrix::inverse() const
{	
	return matrix(float4(x.x, y.x, z.x, 0),
				  float4(x.y, y.y, z.y, 0),
				  float4(x.z, y.z, z.z, 0),
				  float4(-w*x, -w*y, -w*z, 1.0));
}

matrix matrix::rotation(float4 axis, float angle)
{
	matrix result;
	
	float sin = std::sin(angle);
	float cos = std::cos(angle);
	
	axis.w = 0.0f;
	float4 u(axis.normalized());
	
	/*
	 * According to Redbook:
	 *
	 * u = axis/||axis||
	 *
	 *     |  0 -z  y |
	 * S = |  z  0 -x |
	 *     | -y  x  0 |
	 *
	 * M = uu^t + cos(a)(I - uu^t) + sin(a)*S
	 *
	 * That is: M.x = (uu^t).x + cos(a)((1 0 0)^t - uu^t.x) + sin(a) (0 -z y)^t
	 * uu^t.x = u.x * u
	 * And so on for the others
	 */
	
	result.x = u.x*u + cos*(float4(1, 0, 0, 0) - u.x*u) + sin * float4( 0.0,  u.z, -u.y, 0);
	result.y = u.y*u + cos*(float4(0, 1, 0, 0) - u.y*u) + sin * float4(-u.z,  0.0,  u.x, 0);
	result.z = u.z*u + cos*(float4(0, 0, 1, 0) - u.z*u) + sin * float4( u.y, -u.x,  0.0, 0);
	result.w = float4(0, 0, 0, 1);
	
	return result;
}

matrix matrix::frustum(float angle, float aspect, float near, float far)
{
	float ymax = near * tanf(angle * float(M_PI) / 360.0f);
	float xmax = ymax * aspect;
	
	matrix result;
	
	result[0][0] = near/xmax;
	result[1][1] = near/ymax;
	
	result[2][2] = -(far + near) / (far - near);
	result[3][2] = -(2.0f * far * near) / (far - near);
	
	result[2][3] = -1.0f;
	result[3][3] = 0.0f;
	
	return result;
}

matrix matrix::inverseFrustum(float angle, float aspect, float near, float far)
{
	float ymax = near * tanf(angle * float(M_PI) / 360.0f);
	float xmax = ymax * aspect;
	
	matrix result;
	
	result[0][1] = 0.0f;
	result[1][0] = 0.0f;
	result[1][1] = ymax / near;
	result[0][0] = xmax / near;
	
	result[2][2] = 0.0f;
	result[2][3] = -(far - near) / (2.0f * near * far);
	result[3][3] = (far + near) / (2.0f * near * far);
	result[3][2] = -1.0f;
	
	return result;	
}

bool ray4::hitsAABB(const float4 &min, const float4 &max, float &entry, float &exit) const
{
	float4 dir = direction();
	float4 startCorner = (dir > float4(0)).select(min, max);
	float4 endCorner = (dir > float4(0)).select(max, min);
	
	float4 startTs = (startCorner-start()) / dir;
	float4 endTs = (endCorner-start()) / dir;
	
	startTs = startTs.is_finite().select(startTs, float4(0));
	endTs = endTs.is_finite().select(endTs, float4(1));
	
	entry = fmaxf(startTs.max(), 0.0f);
	exit = fminf(endTs.min(), 1.0f);
	
	return !(entry > 1.0f || exit < 0.0f || entry > exit);
}
