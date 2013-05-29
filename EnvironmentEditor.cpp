/*
 *  EnvironmentEditor.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 29.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include "EnvironmentEditor.h"

#include "Environment.h"
#include "EnvironmentDrawer.h"
#include "NetworkInterface.h"
#include "NetworkPacket.h"

namespace
{
	const float SHADE_EDITING_SPEED = 5.0f;
}

EnvironmentEditor::EnvironmentEditor(Environment *env)
: environment(env), environmentDrawer(0), networkInterface(0), currentCellX(0), currentCellZ(0)
{
	mode = None;
	
	// Make sure they don’t point to a valid cell
	environment->getSize(currentCellX, currentCellZ);
}

void EnvironmentEditor::loadFromSerialization(const NetworkPacket *packet)
{
	if (packet->gridOverview.packetType != NetworkPacket::GridOverview) throw std::invalid_argument("Not a GridOverview packet.");
	
	setDimensions(packet->gridOverview.sizeX, packet->gridOverview.sizeZ, packet->gridOverview.cellSize, packet->gridOverview.cellHeight);
	
	for (unsigned x = 0, i = 0; x < packet->gridOverview.sizeX; x++)
	{
		for (unsigned z = 0; z < packet->gridOverview.sizeZ; z++, i++)
		{
			setCellIsWall(x, z, packet->gridOverview.cells[i] & 0x80);
			setCellShade(x, z, float(packet->gridOverview.cells[i] & 0x7F) / 127.0f);
		}
	}
}
NetworkPacket *EnvironmentEditor::writeToSerialization() const
{
	unsigned sizeX, sizeZ;
	getSize(sizeX, sizeZ);
	
	NetworkPacket *environmentPacket = reinterpret_cast<NetworkPacket *>(malloc(sizeof(GridOverviewPacket) + sizeX * sizeZ));
	environmentPacket->gridOverview.packetType = NetworkPacket::GridOverview;
	environmentPacket->gridOverview.packetLength = sizeof(GridOverviewPacket) + sizeX * sizeZ;
	environmentPacket->gridOverview.cellSize = getCellSize();
	environmentPacket->gridOverview.cellHeight = getCellHeight();
	environmentPacket->gridOverview.sizeX = sizeX;
	environmentPacket->gridOverview.sizeZ = sizeZ;
	for (unsigned x = 0, i = 0; x < sizeX; x++)
		for (unsigned z = 0; z < sizeZ; z++, i++)
			environmentPacket->gridOverview.cells[i] = (getCellIsWall(x, z) ? 0x80 : 0x00) + uint8_t(getCellShade(x, z) * 127.0f);
	
	return environmentPacket;
}

void EnvironmentEditor::setDimensions(unsigned sizeX, unsigned sizeZ, float cellSize, float cellHeight)
{
	environment->setDimensions(sizeX, sizeZ, cellSize, cellHeight);
	if (environmentDrawer) environmentDrawer->reloadAll();
	
	// Make sure these variables point to a cell outside the area (hence invalid)
	environment->getSize(currentCellX, currentCellZ);
}

void EnvironmentEditor::getSize(unsigned &xSize, unsigned &zSize) const throw()
{
	environment->getSize(xSize, zSize);
}

float EnvironmentEditor::getCellSize() const throw()
{
	return environment->getCellSize();
}

float EnvironmentEditor::getCellHeight() const throw()
{
	return environment->getCellHeight();
}

bool EnvironmentEditor::getCellIsWall(unsigned x, unsigned z) const throw()
{
	return environment->getCellIsWall(x, z);
}

float EnvironmentEditor::getCellShade(unsigned x, unsigned z) const throw()
{
	return environment->getCellShade(x, z);
}

void EnvironmentEditor::setCellIsWall(unsigned x, unsigned z, bool isit) throw(std::range_error)
{
	if (isit == getCellIsWall(x, z)) return;
	
	environment->setCellIsWall(x, z, isit);
	if (environmentDrawer)
		environmentDrawer->updatedCellWallState(x, z);
}

void EnvironmentEditor::setCellShade(unsigned x, unsigned z, float shade) throw(std::range_error)
{	
	environment->setCellShade(x, z, shade);
	if (environmentDrawer)
		environmentDrawer->updatedCellShade(x, z);
}

void EnvironmentEditor::editCell(unsigned x, unsigned z) throw(std::range_error)
{
	unsigned sizeX, sizeZ;
	getSize(sizeX, sizeZ);
	if (x > sizeX || z > sizeZ) throw std::range_error("Coordinates for cell to edit are out of range.");
	
	if ((x == currentCellX) && (z == currentCellZ)) return;
	currentCellX = x;
	currentCellZ = z;
	
	if (getMode() == Height)
	{
		// Do not allow editing of edge pieces
		if (x == 0 || z == 0 || x == (sizeX-1) || z == (sizeZ-1)) return;
		
		bool isWall = getCellIsWall(x, z);
		setCellIsWall(x, z, !isWall);
		networkInterface->updatedCellState(x, z);
	}
	else if (getMode() == Lighten)
	{
		float currentShade = getCellShade(x, z);
		if (currentShade < 0.49f) setCellShade(x, z, 0.5f);
		else setCellShade(x, z, 1.0f);
		networkInterface->updatedCellState(x, z);
	}
	else if (getMode() == Darken)
	{
		float currentShade = getCellShade(x, z);
		if (currentShade > 0.51f) setCellShade(x, z, 0.5f);
		else setCellShade(x, z, 0.0f);
		networkInterface->updatedCellState(x, z);
	}
}

void EnvironmentEditor::endEditing()
{
	// Make sure they don’t point to a valid cell
	environment->getSize(currentCellX, currentCellZ);
}
