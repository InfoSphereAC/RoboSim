/*
 *  EnvironmentDrawer.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 27.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include "EnvironmentDrawer.h"

#include <cstddef>

#include "Drawer.h"
#include "EnvironmentEditor.h"
#include "OpenGL.h"
#include "Texture.h"

namespace
{
	const unsigned verticesPerCell = 30;
	const float cellRaisingSpeed = 50.0f;
}

inline void EnvironmentDrawer::Vertex::fill(float x, float y, float z, float shade, float nx, float ny, float nz, float r, float s)
{
	position[0] = x;
	position[1] = y;
	position[2] = z;
	uint8_t byteShade = uint8_t(shade * 255.0f);
	color[0] = byteShade;
	color[1] = byteShade;
	color[2] = byteShade;
	color[3] = 255;
	normal[0] = int8_t(nx*127.0f);
	normal[1] = int8_t(ny*127.0f);
	normal[2] = int8_t(nz*127.0f);
	texCoords[0] = int16_t(r);
	texCoords[1] = int16_t(s);
}

void EnvironmentDrawer::writeBufferContentForCell(unsigned x, unsigned z, float squareSize, float height, float shade, Vertex *vertices)
{
	// Draw a cube without bottom.
	// The eventual result will be drawn as GL_QUADS directly.
	float xMin = x * squareSize;
	float zMin = z * squareSize;
	float xMax = (x + 1) * squareSize;
	float zMax = (z + 1) * squareSize;
	
	// +x wall
	vertices[ 0].fill(xMax, height, zMin, shade, +1, 0, 0,  1, 0);
	vertices[ 1].fill(xMax, height, zMax, shade, +1, 0, 0,  1, 1);
	vertices[ 2].fill(xMax, 000.0f, zMax, shade, +1, 0, 0,  0, 1);
	
	
	vertices[ 3].fill(xMax, height, zMin, shade, +1, 0, 0,  1, 0);
	vertices[ 4].fill(xMax, 000.0f, zMax, shade, +1, 0, 0,  0, 1);
	vertices[ 5].fill(xMax, 000.0f, zMin, shade, +1, 0, 0,  0, 0);
	
	// -x wall
	vertices[ 6].fill(xMin, height, zMin, shade, -1, 0, 0,  1, 0);
	vertices[ 7].fill(xMin, height, zMax, shade, -1, 0, 0,  1, 1);
	vertices[ 8].fill(xMin, 000.0f, zMax, shade, -1, 0, 0,  0, 1);
	
	vertices[ 9].fill(xMin, height, zMin, shade, -1, 0, 0,  1, 0);
	vertices[10].fill(xMin, 000.0f, zMax, shade, -1, 0, 0,  0, 1);
	vertices[11].fill(xMin, 000.0f, zMin, shade, -1, 0, 0,  0, 0);
	
	// +z wall
	vertices[12].fill(xMin, height, zMax, shade, 0, 0, +1,  0, 1);
	vertices[13].fill(xMax, height, zMax, shade, 0, 0, +1,  1, 1);
	vertices[14].fill(xMax, 000.0f, zMax, shade, 0, 0, +1,  1, 0);
	
	vertices[15].fill(xMin, height, zMax, shade, 0, 0, +1,  0, 1);
	vertices[16].fill(xMax, 000.0f, zMax, shade, 0, 0, +1,  1, 0);
	vertices[17].fill(xMin, 000.0f, zMax, shade, 0, 0, +1,  1, 0);
	
	// -z wall
	vertices[18].fill(xMin, height, zMin, shade, 0, 0, -1,  0, 1);
	vertices[19].fill(xMax, height, zMin, shade, 0, 0, -1,  1, 1);
	vertices[20].fill(xMax, 000.0f, zMin, shade, 0, 0, -1,  1, 0);
	
	vertices[21].fill(xMin, height, zMin, shade, 0, 0, -1,  0, 1);
	vertices[22].fill(xMax, 000.0f, zMin, shade, 0, 0, -1,  1, 0);
	vertices[23].fill(xMin, 000.0f, zMin, shade, 0, 0, -1,  0, 0);
	
	// top wall
	vertices[24].fill(xMin, height, zMin, shade, 0, +1, 0, 0, 0);
	vertices[25].fill(xMax, height, zMin, shade, 0, +1, 0, 1, 0);
	vertices[26].fill(xMax, height, zMax, shade, 0, +1, 0, 1, 1);
	
	vertices[27].fill(xMin, height, zMin, shade, 0, +1, 0, 0, 0);
	vertices[28].fill(xMax, height, zMax, shade, 0, +1, 0, 1, 1);
	vertices[29].fill(xMin, height, zMax, shade, 0, +1, 0, 0, 1);
}

EnvironmentDrawer::EnvironmentDrawer(EnvironmentEditor *env, Drawer *drawer) : editor(env)
{	
	glGenBuffers(1, &buffer); checkGLError();
	
	texture = drawer->textureWithFilename("frame.tga");
	
	reloadAll();
	
	editor->setEnvironmentDrawer(this);
}

EnvironmentDrawer::~EnvironmentDrawer()
{
	glDeleteBuffers(1, &buffer); checkGLError();
	delete texture;
}

void EnvironmentDrawer::reloadAll()
{
	unsigned xSize, zSize;
	editor->getSize(xSize, zSize);
	numElements = verticesPerCell * xSize * zSize;
	float squareSize = editor->getCellSize();
	float cellHeight = editor->getCellHeight();
	
	Vertex *vertices = new Vertex[numElements];
	unsigned vertexCounter = 0;
	for (unsigned x = 0; x < xSize; x++)
	{
		for (unsigned z = 0; z < zSize; z++, vertexCounter += verticesPerCell)
		{
			bool isWall = editor->getCellIsWall(x, z);
			float shade = editor->getCellShade(x, z);
			writeBufferContentForCell(x, z, squareSize, isWall * cellHeight, shade, &vertices[vertexCounter]);
		}
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numElements, vertices, GL_DYNAMIC_DRAW);
	checkGLError();
	
	delete [] vertices;	
}

void EnvironmentDrawer::update(float timediff)
{
	float maxHeight = editor->getCellHeight();
	
	while (!raisingCells.empty() && raisingCells.front().currentHeight >= maxHeight) raisingCells.pop_front();
	while (!loweringCells.empty() && loweringCells.front().currentHeight <= 0.0f) loweringCells.pop_front();

	for (std::list<ChangingCell>::iterator raiser = raisingCells.begin(); raiser != raisingCells.end(); ++raiser)
	{
		raiser->currentHeight += cellRaisingSpeed*timediff;
	}
	
	for (std::list<ChangingCell>::iterator lower = loweringCells.begin(); lower != loweringCells.end(); ++lower)
	{
		lower->currentHeight -= cellRaisingSpeed*timediff;
	}
	
	
	hasDirtyCells = (raisingCells.size() + loweringCells.size()) > 0;
}


void EnvironmentDrawer::updatedCellWallState(unsigned x, unsigned z)
{
	if (editor->getCellIsWall(x, z))
	{
		ChangingCell changer;
		changer.x = x;
		changer.z = z;
		changer.currentHeight = 0.0f;
		raisingCells.push_back(changer);
	}
	else
	{
		ChangingCell changer;
		changer.x = x;
		changer.z = z;
		changer.currentHeight = editor->getCellHeight();
		loweringCells.push_back(changer);
	}

}

void EnvironmentDrawer::updatedCellShade(unsigned x, unsigned z)
{	
	float height = editor->getCellHeight() * editor->getCellIsWall(x, z);
	
	Vertex newVertices[verticesPerCell];
	writeBufferContentForCell(x, z, editor->getCellSize(), height, editor->getCellShade(x, z), newVertices);
	
	unsigned xSize, zSize;
	editor->getSize(xSize, zSize);
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer); checkGLError();
	glBufferSubData(GL_ARRAY_BUFFER, (x * zSize + z) * sizeof(Vertex [verticesPerCell]), sizeof(Vertex [verticesPerCell]), newVertices); checkGLError();
}

void EnvironmentDrawer::draw()
{
	texture->set();
	glBindBuffer(GL_ARRAY_BUFFER, buffer); checkGLError();
	
	// Update cells that have changed. Do it here instead of update since GL is handled on a different thread (at least on android)
	if (hasDirtyCells)
	{
		unsigned xSize, zSize;
		editor->getSize(xSize, zSize);
		float maxHeight = editor->getCellHeight();
		
		for (std::list<ChangingCell>::iterator raiser = raisingCells.begin(); raiser != raisingCells.end(); ++raiser)
		{
			unsigned x = raiser->x;
			unsigned z = raiser->z;
			float height = fminf(raiser->currentHeight, maxHeight);
			
			Vertex newVertices[verticesPerCell];
			writeBufferContentForCell(x, z, editor->getCellSize(), height, editor->getCellShade(x, z), newVertices);
			glBufferSubData(GL_ARRAY_BUFFER, (x * zSize + z) * sizeof(Vertex [verticesPerCell]), sizeof(Vertex [verticesPerCell]), newVertices); checkGLError();
		}
		
		for (std::list<ChangingCell>::iterator lower = loweringCells.begin(); lower != loweringCells.end(); ++lower)
		{
			unsigned x = lower->x;
			unsigned z = lower->z;
			float height = fmaxf(lower->currentHeight, 0.0f);
			
			Vertex newVertices[verticesPerCell];
			writeBufferContentForCell(x, z, editor->getCellSize(), height, editor->getCellShade(x, z), newVertices);
			glBufferSubData(GL_ARRAY_BUFFER, (x * zSize + z) * sizeof(Vertex [verticesPerCell]), sizeof(Vertex [verticesPerCell]), newVertices); checkGLError();
		}		
	}
	hasDirtyCells = false;
		
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, position))); checkGLError();
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, color))); checkGLError();
	glNormalPointer(GL_BYTE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, normal))); checkGLError();
	glTexCoordPointer(2, GL_SHORT, sizeof(Vertex), reinterpret_cast<void *> (offsetof(Vertex, texCoords))); checkGLError();
	
	glDrawArrays(GL_TRIANGLES, 0, numElements); checkGLError();
}
