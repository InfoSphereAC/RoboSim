/*
 *  Environment.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include <stdexcept>

union float4;
class ray4;

struct EnvironmentCell
{
	bool isWall;
	float shade;
	
	bool intersectsCircle(float radius, float *center);
};

class Environment
{
private:
	EnvironmentCell *cells;
	unsigned sizeX;
	unsigned sizeZ;
	float cellSize;
	float cellHeight;
	
	void throwIfOutOfRange(unsigned x, unsigned z) const throw(std::range_error);
	void clampToRange(unsigned &x, unsigned &z) const throw();
	
public:
	Environment(unsigned sizeX, unsigned sizeZ, float cellSize, float cellHeight);
	~Environment();
	
	void setDimensions(unsigned sizeX, unsigned sizeZ, float cellSize, float cellHeight);
	
	float getCellSize() const throw() { return cellSize; }
	float getCellHeight() const throw() { return cellHeight; }
	void getSize(unsigned &x, unsigned &z) const throw();
	bool getCellIsWall(unsigned x, unsigned z) const throw();
	void setCellIsWall(unsigned x, unsigned z, bool isWall) throw(std::range_error);
	float getCellShade(unsigned x, unsigned z) const throw();
	void setCellShade(unsigned x, unsigned z, float shade) throw(std::range_error);
	
	/*!
	 * @abstract Calculates the first cell hit when following a line.
	 * @discussion This method follows the ray and reports the very first cell
	 * hit. It is aware of wall/no wall states and will report the correct
	 * solution for any ray in any direction. Notice that it is possible that
	 * no cell at all is hit.
	 * @param start The start of the ray.
	 * @param direction The direction of the ray.
	 * @param x On exit, the x-index of the cell, or undefined if no cell was
	 * hit.
	 * @param y On exit, the y-index of the cell, or undefined if no cell was
	 * hit.
	 * @param length The distance from start 
	 * @result true if a cell was hit, with indices of the cell
	 * in x and z and the distance in length, or false and the three outputs
	 * undefined otherwise.
	 */
	bool getFirstCellOnRay(const ray4 &start, int &x, int &z, float &length) const throw();
};
