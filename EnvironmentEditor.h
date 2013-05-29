/*
 *  EnvironmentEditor.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 29.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include <stdexcept>

class Drawer;
class Environment;
class EnvironmentDrawer;
class NetworkInterface;
union NetworkPacket;

class EnvironmentEditor
{
public:
	enum EditingMode
	{
		None,
		Height,
		Darken,
		Lighten
	};
	
private:
	Environment *environment;
	EnvironmentDrawer *environmentDrawer;
	NetworkInterface *networkInterface;
	
	unsigned currentCellX;
	unsigned currentCellZ;
	
	EditingMode mode;
public:
	EnvironmentEditor(Environment *env);
	void setEnvironmentDrawer(EnvironmentDrawer *newDrawer) { environmentDrawer = newDrawer; }
	void setNetworkInterface(NetworkInterface *anInterface) { networkInterface = anInterface; }
	
	void loadFromSerialization(const NetworkPacket *packet);
	NetworkPacket *writeToSerialization() const;
	
	void setDimensions(unsigned sizeX, unsigned sizeZ, float cellSize, float cellHeight);
	
	void getSize(unsigned &xSize, unsigned &zSize) const throw();
	float getCellSize() const throw();
	float getCellHeight() const throw();
	bool getCellIsWall(unsigned x, unsigned z) const throw();
	float getCellShade(unsigned x, unsigned z) const throw();
	void setCellIsWall(unsigned x, unsigned z, bool isit) throw(std::range_error);
	void setCellShade(unsigned x, unsigned z, float shade) throw(std::range_error);
	
	EditingMode getMode() const throw() { return mode; }
	void setMode(EditingMode newMode) throw() { mode = newMode; }
	
	/*!
	 * @abstract Tells the editor to alter a particular cell.
	 * @discussion For lighten or darken mode, it lightens or darkens the cell
	 * by one step (i.e. to 0.0, 0.5 or 1.0). It is not the duty of the
	 * EnvironmentEditor to check that this editing operation can actually be
	 * done without causing inconsistencies; e.g. that
	 * no object is placed on a cell that you want to raise.
	 * @param x Location of the cell.
	 * @param z Location of the cell.
	 * @throws range_error if x or z specify a cell that does not exist.
	 */
	void editCell(unsigned x, unsigned z) throw(std::range_error);
	
	/*!
	 * @abstract Ends an editing session
	 * @discussion During an editing session, the editor remembers the last cell
	 * edited and refuses to change it again unless a different one has been
	 * changed in the meantime. Ending an editing session is typically done in
	 * response to the mouse button being released.
	 */
	void endEditing();
};
