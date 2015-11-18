/*
 *  Environment.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10.
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include "Environment.h"

#include "Vec4.h"

#include <math.h>

Environment::Environment(unsigned sX, unsigned sZ, float cS, float cH) : cells(0)
{
	setDimensions(sX, sZ, cS, cH);
}

void Environment::setDimensions(unsigned sX, unsigned sZ, float cS, float cH)
{
	sizeX = sX;
	sizeZ = sZ;
	cellSize = cS;
	cellHeight = cH;
	
	if (cells) delete [] cells;
	
	cells = new EnvironmentCell [sizeX * sizeZ];
	
	for (unsigned x = 0; x < sizeX; x++)
	{
		for (unsigned z = 0; z < sizeZ; z++)
		{
			cells[x*sizeZ+z].isWall = (x == 0) || (z == 0) || (x == sizeX - 1) || (z == sizeZ - 1);
			cells[x*sizeZ+z].shade = 0.5f;
		}
	}
}

Environment::~Environment()
{
	delete [] cells;
}

inline void Environment::throwIfOutOfRange(unsigned x, unsigned z) const throw(std::range_error)
{
	if (x >= sizeX) throw std::range_error("Value for x is out of range.");
	else if (z >= sizeZ) throw std::range_error("Value for z is out of range.");
}

inline void Environment::clampToRange(unsigned &x, unsigned &z) const throw()
{
	if (x >= sizeX) x = sizeX - 1;
	if (z >= sizeZ) z = sizeZ - 1;
}

void Environment::setCellShade(unsigned x, unsigned z, float value) throw(std::range_error)
{
	throwIfOutOfRange(x, z);
	if (value > 1.0f) value = 1.0f;
	else if (value < 0.0f) value = 0.0f;
	cells[x * sizeZ + z].shade = value;
}

float Environment::getCellShade(unsigned x, unsigned z) const throw()
{
	clampToRange(x, z);
	return cells[x * sizeZ + z].shade;
}

void Environment::setCellIsWall(unsigned x, unsigned z, bool isWall) throw(std::range_error)
{
	throwIfOutOfRange(x, z);
	cells[x * sizeZ + z].isWall = isWall;
}

bool Environment::getCellIsWall(unsigned x, unsigned z) const throw()
{
	clampToRange(x, z);
	return cells[x * sizeZ + z].isWall;
}

void Environment::getSize(unsigned &x, unsigned &z) const throw()
{
	x = sizeX;
	z = sizeZ;
}

void Environment::setChallenge(bool mode) throw(std::range_error){
	challenge = mode;
}

bool Environment::getFirstCellOnRay(const ray4 &ray, int &x, int &z, float &length) const throw()
{
	// 0th step: Transform ray into coord system where height of wall is 1
	// and cell size is 1, to make later calculations easier.
	
	float4 transformVector(getCellSize(), getCellHeight(), getCellSize());
	ray4 relRay = ray / transformVector;
	float4 relDirection = relRay.direction();
	
	// 1st step: Clip ray into area covered by this Environment
	float4 max(float(sizeX) - 0.000001f, 1.0f, float(sizeZ) - 0.000001f);
	float4 min(0, 0, 0);
	
	float tStart, tEnd;
	if (!relRay.hitsAABB(min, max, tStart, tEnd)) return false;
	
	float4 clippedStart = relRay.point(tStart);
	float4 clippedEnd = relRay.point(tEnd);
	length = tStart;
		
	// 2nd step: Do DDA following the ray. The moment we found a wall, we have a
	// guaranteed hit.
	float4 point = clippedStart;
	
	bool dirXPositive = relDirection.x > 0;
	bool dirZPositive = relDirection.z > 0;
	int lastX = unsigned(clippedEnd.x);
	int lastZ = unsigned(clippedEnd.z);
	
	relDirection = relDirection.normalized();
	
	while(true)
	{
		float4 lowerPoint = point.floor();
		float4 relativePoint = point - lowerPoint;
		
		x = int(lowerPoint.x);
		z = int(lowerPoint.z);

		// Check whether on last cell. If that is a floor cell and the clipped
		// end is on the floor and the ray is going down, we assume a hit at the
		// clipped end point.
		if (x == lastX && z == lastZ)
		{
			if (!getCellIsWall(x, z) && (clippedEnd.y <= 0.00001f) && (relDirection.y < 0.0f))
			{
				length = tEnd;
				return true;
			}
			else if (!getCellIsWall(x, z))
				return false;
		}
		
		// Check whether we missed last cell (can happen in rare circumstances)
		if (((dirXPositive && x > lastX) || (!dirXPositive && x < lastX))
			|| (dirZPositive && z > lastZ) || (!dirZPositive && z < lastZ))
			break;		
		
		if (getCellIsWall(x, z))
			return true;
		
		float4 toNextCell = (relDirection > float4(0)).select(float4(1) - relativePoint, -relativePoint);
		float4 tToNext = toNextCell / relDirection;
		float t = fminf(tToNext.x, tToNext.z) + 0.01f;
		point += t * relDirection;
		length += t;
	}
	
	return false;
}
