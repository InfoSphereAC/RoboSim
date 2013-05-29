/*
 *  EnvironmentDrawer.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 27.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include <list>
#include <stdint.h>

class Drawer;
class EnvironmentEditor;
class Texture;

class EnvironmentDrawer
{
	EnvironmentEditor *editor;
	
	unsigned buffer;
	unsigned numElements;
	
	Texture *texture;
	
#ifdef _MSC_VER
#pragma pack (1)
#endif
	struct Vertex
	{
		float position[3];
		uint8_t color[4];
		int8_t normal[3];
		int8_t alignment; // Ignore this field.
		int16_t texCoords[2];
		void fill(float x, float y, float z, float shade, float nx, float ny, float nz, float r, float s);
#ifdef _MSC_VER
	};
#else
	} __attribute__((packed));
#endif
	
	struct ChangingCell
	{
		unsigned x;
		unsigned z;
		float currentHeight;
	};
	
	std::list<ChangingCell> raisingCells;
	std::list<ChangingCell> loweringCells;
	bool hasDirtyCells;
	
	static void writeBufferContentForCell(unsigned x, unsigned z, float squareSize, float height, float shade, Vertex *vertices);
	
public:
	EnvironmentDrawer(EnvironmentEditor *env, Drawer *drawer);
	~EnvironmentDrawer();

	void reloadAll();
	
	void update(float timediff);
	
	void draw();
	
	void updatedCellWallState(unsigned x, unsigned z);
	void updatedCellShade(unsigned x, unsigned z);
};
