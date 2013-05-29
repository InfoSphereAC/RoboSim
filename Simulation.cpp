/*
 *  Simulation.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include "Simulation.h"

#include <cmath>
#include <iostream>
#include <stdlib.h>

#include "Environment.h"
#include "Robot.h"
#include "Vec4.h"

namespace
{
	const float robotStartSpace = 5.0f;
}

const std::pair<float, float> &Simulation::nextPossibleStartingLocation()
{
	std::vector<std::pair<float, float> >::iterator current = lastStartLocation;
	
	++lastStartLocation;
	
	if (lastStartLocation == possibleStartLocations.end()) lastStartLocation = possibleStartLocations.begin();
	
	return *current;
}

const std::pair<float, float> &Simulation::nextValidStartingLocation()
{
	const std::pair<float, float> *location = &(nextPossibleStartingLocation());
	float firstX = location->first;
	float firstZ = location->second;
	
	while (!canPlaceRobotInAreaStartingAt(location->first, location->second))
	{
		location = &(nextPossibleStartingLocation());
		
		if (location->first == firstX && location->second == firstZ)
		{
			// Tried all, none are possible. Just return this one then and hope something works out.
			return *location;
		}
	}
	return *location;
}

bool Simulation::canPlaceRobotInAreaStartingAt(float x, float z) const
{
	// First: Check for walls in area
	unsigned minCellX = unsigned(x / environment->getCellSize());
	unsigned maxCellX = unsigned((x + robotStartSpace) / environment->getCellSize());
	
	unsigned minCellZ = unsigned(z / environment->getCellSize());
	unsigned maxCellZ = unsigned((z + robotStartSpace) / environment->getCellSize());
	
	for (unsigned cellX = minCellX; cellX <= maxCellX; cellX++)
	{
		for (unsigned cellZ = minCellZ; cellZ <= maxCellZ; cellZ++)
		{
			if (environment->getCellIsWall(cellX, cellZ)) return false;
		}
	}
	
	// Second: Check for robots in area
	float4 areaBounds[] = {
		float4(x, 0, z),
		float4(x, 0, z + robotStartSpace),
		float4(x + robotStartSpace, 0, z + robotStartSpace),
		float4(x + robotStartSpace, 0, z),
	};
	float4 unusedResolutionVector;
	for (std::vector<Robot *>::const_iterator iter = robots.begin(); iter != robots.end(); ++iter)
	{
		if (orientedBoundingBoxesCollide(areaBounds, (*iter)->getOrientedBoundingBox(), unusedResolutionVector))
			return false;
	}
	
	return true;
}

bool Simulation::objectsOverlapAlongAxis(const float4 *a, unsigned numA, const float4 *b, unsigned numB, const float4 &axis, float &overlap) throw()
{
	float minA = std::numeric_limits<float>::infinity(), minB = std::numeric_limits<float>::infinity();
	float maxA = -std::numeric_limits<float>::infinity(), maxB = -std::numeric_limits<float>::infinity();
	
	for (unsigned i = 0; i < numA; i++)
	{
		float dot = a[i] * axis;
		minA = fminf(dot, minA);
		maxA = fmaxf(dot, maxA);
	}
	
	for (unsigned i = 0; i < numB; i++)
	{
		float dot = b[i] * axis;
		minB = fminf(dot, minB);
		maxB = fmaxf(dot, maxB);
	}
	
	float overlapLeft = maxB - minA;
	float overlapRight = maxA - minB;
	if (overlapLeft < overlapRight) overlap = -overlapLeft;
	else overlap = overlapRight;
	
	return (fminf(overlapLeft, overlapRight) > 0.0f);
}

bool Simulation::orientedBoundingBoxesCollide(const float4 *a, const float4 *b, float4 &resolutionVector) throw()
{
	float overlap = 20000000.0f;
	float newOverlap;
	float4 direction;
	
	direction = (a[0] - a[1]).normalized();
	if (!objectsOverlapAlongAxis(a, 4, b, 4, direction, newOverlap)) return false;
	else if (std::fabs(newOverlap) < std::fabs(overlap))
	{
		overlap = newOverlap;
		resolutionVector = direction * overlap;
	}
	
	direction = (a[1] - a[2]).normalized();
	if (!objectsOverlapAlongAxis(a, 4, b, 4, direction, newOverlap)) return false;
	else if (std::fabs(newOverlap) < std::fabs(overlap))
	{
		overlap = newOverlap;
		resolutionVector = direction * overlap;
	}
	
	direction = (b[0] - b[1]).normalized();
	if (!objectsOverlapAlongAxis(a, 4, b, 4, direction, newOverlap)) return false;
	else if (std::fabs(newOverlap) < std::fabs(overlap))
	{
		overlap = newOverlap;
		resolutionVector = direction * overlap;
	}
	
	direction = (b[1] - b[2]).normalized();
	if (!objectsOverlapAlongAxis(a, 4, b, 4, direction, newOverlap)) return false;
	else if (std::fabs(newOverlap) < std::fabs(overlap))
	{
		overlap = newOverlap;
		resolutionVector = direction * overlap;
	}
	
	return true;
}

bool Simulation::cellCollidesWithRobot(unsigned cellX, unsigned cellZ, const Robot *aRobot, float4 &resolutionVector) const throw(std::range_error)
{
	if (!environment->getCellIsWall(cellX, cellZ)) return false;
	
	const float cellSize = environment->getCellSize();
	
	float4 cellCorners[] = {
		float4(cellSize * float(cellX), 0.0f, cellSize * float(cellZ)),
		float4(cellSize * float(cellX+1), 0.0f, cellSize * float(cellZ)),
		float4(cellSize * float(cellX+1), 0.0f, cellSize * float(cellZ+1)),
		float4(cellSize * float(cellX), 0.0f, cellSize * float(cellZ+1)),
	};
	
	return orientedBoundingBoxesCollide(cellCorners, aRobot->getOrientedBoundingBox(), resolutionVector);
}

// Returns false if the result is entirely out of the bounds of the environment
inline bool Simulation::getCellsCoveredByBox(const float4 *corners, int &minX, int &maxX, int &minZ, int &maxZ) const throw()
{
	float4 aabb[2] = {
		corners[0].min(corners[1].min(corners[2].min(corners[3]))),
		corners[0].max(corners[1].max(corners[2].max(corners[3])))
	};
	
	return getCellsCoveredByAABB(aabb, minX, maxX, minZ, maxZ);
}

bool Simulation::getCellsCoveredByAABB(const float4 *corners, int &minX, int &maxX, int &minZ, int &maxZ) const throw()
{
	const float cellSize = environment->getCellSize();
	float4 cellSizeVector(cellSize, 1.0f, cellSize);
	float4 min = corners[0] / cellSizeVector;
	float4 max = corners[1] / cellSizeVector;
	
	unsigned xSize, zSize;
	environment->getSize(xSize, zSize);
	
	minX = int(min.x);
	minZ = int(min.z);
	
	// Out of environment: Bail
	if (minX > int(xSize) || minZ > int(zSize)) return false;
	
	maxX = int(max.x);
	maxZ = int(max.z);
	
	// Out of environment: Bail
	if (maxX < 0 || maxZ < 0) return false;
	
	// Put it inside the box.
	minX = std::max(minX, 0);
	maxX = std::min(maxX, int(xSize) - 1);
	
	minZ = std::max(minZ, 0);
	maxZ = std::min(maxZ, int(zSize) - 1);
	
	return true;
}

bool Simulation::testRobotCollidesWithEnvironment(const Robot *aRobot, float4 &resolutionVector) const throw()
{	
	// This is used to smooth out the amount the robot can move. This will cause
	// it to be far less jumpy when squeezed into a less-than robot sized space.
	// It will also hinder it from tunneling out of that tiny space again.
	float totalResolutions = 0.0f;
	resolutionVector = float4(0.0f);
	
	// Find area of the map that the robot covers
	int minX, maxX, minZ, maxZ;
	if (!getCellsCoveredByAABB(aRobot->getAxisAlignedBoundingBox(), minX, maxX, minZ, maxZ)) return false;
	
	const float4 *orientedBoundingBox = aRobot->getOrientedBoundingBox();
	const float cellSize = environment->getCellSize();
	
	// Test collision
	for (int x = minX; x <= maxX; x++)
	{
		for (int z = minZ; z <= maxZ; z++)
		{
			if (!environment->getCellIsWall(x, z)) continue;
			
			float4 cellCorners[] = {
				float4(cellSize * float(x), 0.0f, cellSize * float(z)),
				float4(cellSize * float(x+1), 0.0f, cellSize * float(z)),
				float4(cellSize * float(x+1), 0.0f, cellSize * float(z+1)),
				float4(cellSize * float(x), 0.0f, cellSize * float(z+1)),
			};
			
			float4 newResolution;
			if (orientedBoundingBoxesCollide(cellCorners, orientedBoundingBox, newResolution))
			{
				totalResolutions++;
				resolutionVector += newResolution;
			}
		}
	}
	resolutionVector /= float4(totalResolutions);
	
	return totalResolutions != 0.0f;
}


bool Simulation::testRobotsCollide(const Robot *robot1, const Robot *robot2, float4 &resolutionVector) const throw()
{
	// Start with basic AABB check
	// Notice: 0 is min, 1 is max
	const float4 *aabb1 = robot1->getAxisAlignedBoundingBox();
	const float4 *aabb2 = robot2->getAxisAlignedBoundingBox();
	
	if (aabb1[1].x < aabb2[0].x || aabb2[1].x < aabb1[0].x) return false;
	if (aabb1[1].z < aabb2[0].z || aabb2[1].z < aabb1[0].z) return false;

	// Okay, collision is not entirely implausible
	return orientedBoundingBoxesCollide(robot1->getOrientedBoundingBox(), robot2->getOrientedBoundingBox(), resolutionVector);
}

		  
Simulation::Simulation(Environment *anEnvironment) : environment(anEnvironment)
{
	// Initialize the start locations array
	unsigned xSize, zSize;
	environment->getSize(xSize, zSize);
	
	unsigned numLocationsX = unsigned((float(xSize) * environment->getCellSize()) / robotStartSpace) - 1;
	unsigned numLocationsZ = unsigned((float(zSize) * environment->getCellSize()) / robotStartSpace) - 1;
	
	float locationX = robotStartSpace * 0.5f;
	for (unsigned x = 0; x < numLocationsX; x++)
	{
		float locationZ = robotStartSpace * 0.5f;
		for (unsigned z = 0; z < numLocationsZ; z++)
		{
			possibleStartLocations.push_back(std::pair<float, float>(locationX, locationZ));
			locationZ += robotStartSpace;
		}
		locationX += robotStartSpace;
	}
	
	// Now randomize it
	for (unsigned i = 0; i < numLocationsX*numLocationsZ; i++)
	{
		std::pair<float, float> scratch = possibleStartLocations[i];
		unsigned swapWith = rand() % (numLocationsX*numLocationsZ);
		possibleStartLocations[i] = possibleStartLocations[swapWith];
		possibleStartLocations[swapWith] = scratch;
	}
	
	lastStartLocation = possibleStartLocations.begin();
}
		  
void Simulation::update(float timedelta) throw()
{
	timedelta = fminf(timedelta, 0.5f); // Prevent times and hence robot movements from getting too large.
	
	for (std::vector<Robot *>::iterator iter = robots.begin(); iter != robots.end(); ++iter)
		(*iter)->updatePhysics(timedelta);
	
	// Test for collisions
	unsigned xSize, zSize;
	environment->getSize(xSize, zSize);
	
	for (std::vector<Robot *>::iterator iter = robots.begin(); iter != robots.end(); ++iter)
	{
		if ((*iter)->isLifted()) continue;
		
		float4 environmentResolution(0.0f);
		
		// Check whether robot tunnelled through anything
		float4 oldPosition = (*iter)->getLastPosition().w;
		float4 newPosition = (*iter)->getPosition().w;
		int x, z;
		float length;
		if (environment->getFirstCellOnRay(ray4(oldPosition, newPosition), x, z, length))
			(*iter)->setPosition((*iter)->getLastPosition());

		if (testRobotCollidesWithEnvironment(*iter, environmentResolution))
			(*iter)->moveDirectly(environmentResolution);
		
		for (std::vector<Robot *>::iterator iter2 = (iter + 1); iter2 != robots.end(); ++iter2)
		{
			if ((*iter2)->isLifted()) continue;
			
			float4 resolutionVector(0.0f);
			if (!testRobotsCollide(*iter, *iter2, resolutionVector)) continue;
			(*iter2)->moveDirectly(resolutionVector / 2);
			(*iter)->moveDirectly(-resolutionVector / 2);
		}
		
	}
}

bool Simulation::objectCollidesWithOthers(const float4 *orientedBoundingBox) const throw()
{
	int minX, maxX, minZ, maxZ;
	if (!getCellsCoveredByBox(orientedBoundingBox, minX, maxX, minZ, maxZ)) return false;
	
	const float cellSize = environment->getCellSize();
	
	// Test collision
	for (int x = minX; x <= maxX; x++)
	{
		for (int z = minZ; z <= maxZ; z++)
		{
			if (!environment->getCellIsWall(x, z)) continue;
			
			float4 cellCorners[] = {
				float4(cellSize * float(x), 0.0f, cellSize * float(z)),
				float4(cellSize * float(x+1), 0.0f, cellSize * float(z)),
				float4(cellSize * float(x+1), 0.0f, cellSize * float(z+1)),
				float4(cellSize * float(x), 0.0f, cellSize * float(z+1)),
			};
			
			float4 ignoredResolution;
			if (orientedBoundingBoxesCollide(cellCorners, orientedBoundingBox, ignoredResolution)) return true;
		}
	}
	
	// Test robot
	for (std::vector<Robot *>::const_iterator iter = robots.begin(); iter != robots.end(); ++iter)
	{
		if ((*iter)->isLifted()) continue;
		float4 ignoredResolution;
		if (orientedBoundingBoxesCollide((*iter)->getOrientedBoundingBox(), orientedBoundingBox, ignoredResolution)) return true;
	}
	
	return false;
}

void Simulation::addRobot(Robot *aRobot) throw(std::invalid_argument)
{
	if (!aRobot) throw std::invalid_argument("Trying to add NULL robot to Simulation");
	
	robots.push_back(aRobot);
	
	const std::pair<float, float> &location = nextValidStartingLocation();
	
	aRobot->setPosition(matrix::position(float4(location.first, 0.0f, location.second)));
}

void Simulation::removeRobot(Robot *aRobot)
{
	for (std::vector<Robot *>::iterator iter = robots.begin(); iter != robots.end(); ++iter)
	{
		if (*iter == aRobot)
		{
			robots.erase(iter);
			return;
		}
	}
}

bool Simulation::firstHitOfRay(const ray4 &ray, bool ignoringRobots, float &outHit) const throw()
{
	int x, z;
	
	bool hitAnything = environment->getFirstCellOnRay(ray, x, z, outHit);
	
	if (!ignoringRobots)
	{
		for (std::vector<Robot *>::const_iterator iter = robots.begin(); iter != robots.end(); ++iter)
		{
			float robotDistance;
			if ((*iter)->hitByRay(ray, robotDistance)) continue;
			
			hitAnything = true;
			outHit = fminf(outHit, robotDistance);
		}
	}
	
	return hitAnything;
}

void Simulation::getEnvironmentSize(unsigned &x, unsigned &z) const throw()
{
	environment->getSize(x, z);
}

float Simulation::getCellShade(unsigned x, unsigned z) const throw(std::range_error)
{
	return environment->getCellShade(x, z);
}


bool Simulation::canToggleCellIsWall(unsigned x, unsigned z) throw(std::range_error)
{
	if (environment->getCellIsWall(x, z)) return true;
	
	const float cellSize = environment->getCellSize();
	
	float4 cellCorners[] = {
		float4(cellSize * float(x), 0.0f, cellSize * float(z)),
		float4(cellSize * float(x+1), 0.0f, cellSize * float(z)),
		float4(cellSize * float(x+1), 0.0f, cellSize * float(z+1)),
		float4(cellSize * float(x), 0.0f, cellSize * float(z+1)),
	};
	
	float4 resolutionVector;
	for (std::vector<Robot *>::iterator iter = robots.begin(); iter != robots.end(); ++iter)
	{
		if ((*iter)->isLifted()) continue;
		if (orientedBoundingBoxesCollide(cellCorners, (*iter)->getOrientedBoundingBox(), resolutionVector)) return false;
	}
	
	return true;
}
