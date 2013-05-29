//
//  RobotTouchHandler.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 17.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "Vec4.h"

class NetworkInterface;

/*!
 * @abstract Handles user movement of the robot.
 * @discussion This is mainly intended for multi-touch devices, but used on normal computers as well. It handles one or two taps on the robot and how they move.
 *
 * It recognizes three patterns:
 * - Move with one touch (or mouse)
 *   -> Robot moves.
 * - Two touches moving approximately in the same direction
 *   -> treat as move.
 * - Two touches moving in different directions
 *   -> treat as rotate
 *
 * The latter two can also happen at the same time. To do its job, all touches that hit the robot have to go through here.
 */

class RobotTouchHandler
{
	unsigned touchIdentifiers[2];
	struct
	{
		float x;
		float y;
	} screenCoords[2];
	float4 lastTouchLocation[2];
	float4 firstHitOffset;
	
	NetworkInterface *networkInterface;
		
public:
	RobotTouchHandler(NetworkInterface *anInterface);
	
	void touchBegan(unsigned touchIdentifier, float screenX, float screenY, const ray4 &ray, float hitRobotAt);
	void touchMoved(unsigned touchIdentifier, float screenX, float screenY, const ray4 &ray);
	void touchEnded(unsigned touchIdentifier);
	void update();
};
