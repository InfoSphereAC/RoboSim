/*
 *  Simulation.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include <stdexcept>
#include <vector>

class Environment;
class Robot;
union float4;
class ray4;

class Simulation
{
	Environment *environment;
	std::vector<Robot *> robots;
	
	std::vector<std::pair<float, float> > possibleStartLocations;
	std::vector<std::pair<float, float> >::iterator lastStartLocation;
	
	const std::pair<float, float> &nextPossibleStartingLocation();
	const std::pair<float, float> &nextValidStartingLocation();
	bool canPlaceRobotInAreaStartingAt(float x, float z) const;
	
	static bool objectsOverlapAlongAxis(const float4 *a, unsigned numA, const float4 *b, unsigned numB, const float4 &axis, float &overlap) throw();
	
	bool getCellsCoveredByBox(const float4 *corners, int &minX, int &maxX, int &minZ, int &maxZ) const throw();
	bool getCellsCoveredByAABB(const float4 *corners, int &minX, int &maxX, int &minZ, int &maxZ) const throw();
	bool testRobotCollidesWithEnvironment(const Robot *aRobt, float4 &resolutionVector) const throw();
	bool testRobotsCollide(const Robot *robot1, const Robot *robot2, float4 &resolutionVector) const throw();
	
	/*!
	 * @abstract Checks whether two oriented bounding boxes overlap.
	 * @discussion A semi-general collision detection method. It assumes that
	 * (a[0], a[1]) is parallel to (a[2], a[3]) and likewise that (a[1], a[1]) is
	 * parallel to (a[3], a[0]). Same goes for b.
	 * @param a The vertices of the first oriented bounding box. At least four.
	 * @param b The vertices of the other oriented bounding box. At least four.
	 * @param resolutionVector On exit, If a collision is found, contains the
	 * shortest vector along which one of the objects is to be moved to stop them
	 * from colliding.
	 */
	static bool orientedBoundingBoxesCollide(const float4 *a, const float4 *b, float4 &resolutionVector) throw();
	
	/*!
	 * @abstract Checks whether a robot collides with a particular cell.
	 * @param cellX Location of the cell.
	 * @param cellZ Location of the cell.
	 * @param aRobot The robot to collide with.
	 * @param resolutionVector On output, if the result is true, contains the
	 * vector the robots position needs to be altered by to make it cease
	 * colliding. If not colliding, is undefined.
	 * @result Whether the robot collides with that particular cell.
	 * @throws range_error if x or z are out of range
	 */
	bool cellCollidesWithRobot(unsigned cellX, unsigned cellZ, const Robot *aRobot, float4 &resolutionVector) const throw(std::range_error);
	
public:
	Simulation(Environment *anEnvironment);
	
	void resetRobots();
	
	void addRobot(Robot *aRobot) throw(std::invalid_argument);
	void removeRobot(Robot *aRobot);
	
	void update(float timedelta) throw();
	
	bool firstHitOfRay(const ray4 &ray, bool ignoringRobots, float &outHit) const throw();
	
	void getEnvironmentSize(unsigned &x, unsigned &z) const throw();
	float getCellShade(unsigned x, unsigned z) const throw(std::range_error);
	
	/*!
	 * @abstract Checks whether something intersects anything else in the scene.
	 * @discussion This is not part of the normal collision recognition, but
	 * simply a function to find the answer to "does this hit anything". It is
	 * useful e.g. for implementing a touch sensor. The oriented bounding box
	 * is assumed to be a rectangle on the xz plane, with the sides 01 parallel
	 * to 23 and 12 parallel to 30.
	 * @param obb The oriented bounding box of the object. Has to have exactly
	 * four elements.
	 * @result true if anything intersects this, false otherwise.
	 */
	bool objectCollidesWithOthers(const float4 *obb) const throw();
	
	/*!
	 * @abstract Determines whether a cell can be changed from wall to floor or
	 * vice versa.
	 * @discussion A wall cell can always be turned into a floor cell. A floor
	 * cell can be turned into a wall cell if no robot were to collide with it
	 * if you did so.
	 * @param x location of the cell.
	 * @param z location of the cell.
	 * @result true if the cell can be lowered/raised, false if something is
	 * blocking it.
	 * @throws range_error if x or z are out of range
	 */
	bool canToggleCellIsWall(unsigned x, unsigned z) throw(std::range_error);
	
};
