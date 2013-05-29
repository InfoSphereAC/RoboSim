//
//  RobotTouchHandler.cpp
//  mindstormssimulation
//
//  Created by Torsten Kammer on 17.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "RobotTouchHandler.h"

#include "Drawer.h"
#include "NetworkInterface.h"

RobotTouchHandler::RobotTouchHandler(NetworkInterface *anInterface)
{
	touchIdentifiers[0] = UINT_MAX;
	touchIdentifiers[1] = UINT_MAX;
	networkInterface = anInterface;
}

void RobotTouchHandler::touchBegan(unsigned touchIdentifier, float screenX, float screenY, const ray4 &ray, float hitRobotAt)
{
	if (!networkInterface->getLocalRobot()) return;
	
	if (touchIdentifiers[0] == UINT_MAX)
	{
		// Is first touch.
		networkInterface->setIsLifted(true);
		touchIdentifiers[0] = touchIdentifier;
		
		lastTouchLocation[0] = ray.point(hitRobotAt);
		firstHitOffset = ray.point(hitRobotAt) - networkInterface->getLocalRobot()->getPosition().w;
		
		screenCoords[0].x = screenX;
		screenCoords[0].y = screenY;
	}
	else if (touchIdentifiers[1] == UINT_MAX)
	{
		// Second touch
		lastTouchLocation[1] = ray.planeIntersectionPoint(float4(0.f, 1.f, 0.f, 0.f), firstHitOffset);
		touchIdentifiers[1] = touchIdentifier;
		
		screenCoords[1].x = screenX;
		screenCoords[1].y = screenY;
	}
}

void RobotTouchHandler::touchEnded(unsigned touchIdentifier)
{
	if (!networkInterface->getLocalRobot()) return;
	
	if (touchIdentifier == touchIdentifiers[0])
	{
		if (touchIdentifiers[1] != UINT_MAX)
		{
			// Set second to first identifier
			touchIdentifiers[0] = touchIdentifiers[1];
			lastTouchLocation[0] = lastTouchLocation[1];
			screenCoords[0] = screenCoords[1];
			touchIdentifiers[1] = UINT_MAX;
		}
		else
		{
			// End moving
			networkInterface->setIsLifted(false);
			touchIdentifiers[0] = UINT_MAX;
		}
	}
	else if (touchIdentifier == touchIdentifiers[1])
		touchIdentifiers[1] = UINT_MAX;
}

void RobotTouchHandler::touchMoved(unsigned touchIdentifier, float screenX, float screenY, const ray4 &ray)
{
	if (!networkInterface->getLocalRobot()) return;
	
	if (touchIdentifiers[1] == UINT_MAX)
	{
		if (touchIdentifiers[0] == touchIdentifier)
		{
			// There's only one touch, and it just moved
			
			// Find point on plane with n=(0 1 0)^t through firstHitOffset
			float4 newDragPoint = ray.planeIntersectionPoint(float4(0.f, 1.f, 0.f, 0.f), firstHitOffset);
			
			// Move point that was clicked to point that was dragged to.
			float4 diff = newDragPoint - (networkInterface->getLocalRobot()->getPosition().w + firstHitOffset);
			networkInterface->moveLifted(diff);
			
			screenCoords[0].x = screenX;
			screenCoords[0].y = screenY;
		}
	}
	else
	{
		// Two touches active. Move and rotate robot.
		
		// Find current location of both touches
		float4 newTouchLocations[2];
		if (touchIdentifier == touchIdentifiers[0])
		{
			newTouchLocations[0] = ray.planeIntersectionPoint(float4(0.f, 1.f, 0.f, 0.f), firstHitOffset);
			newTouchLocations[1] = lastTouchLocation[1];
			screenCoords[0].x = screenX;
			screenCoords[0].y = screenY;
		}
		else if (touchIdentifier == touchIdentifiers[1])
		{
			newTouchLocations[0] = lastTouchLocation[0];
			newTouchLocations[1] = ray.planeIntersectionPoint(float4(0.f, 1.f, 0.f, 0.f), firstHitOffset);
			screenCoords[1].x = screenX;
			screenCoords[1].y = screenY;
		}
		else return;
		
		// Move by difference of midpoints
		float4 oldCenter = 0.5f * (lastTouchLocation[0] + lastTouchLocation[1]);
		float4 newCenter = 0.5f * (newTouchLocations[0] + newTouchLocations[1]);
		networkInterface->moveLifted(newCenter - oldCenter);
		
		// Rotate by angle between old line and new line.
		float4 oldP0P1 = lastTouchLocation[1] - lastTouchLocation[0];
		float4 newP0P1 = newTouchLocations[1] - newTouchLocations[0];
		
		float oldAngle = atan2f(oldP0P1.z, oldP0P1.x);
		float newAngle = atan2f(newP0P1.z, newP0P1.x);
		networkInterface->rotateLifted(oldAngle-newAngle);
		
		lastTouchLocation[0] = newTouchLocations[0];
		lastTouchLocation[1] = newTouchLocations[1];
	}
}

void RobotTouchHandler::update()
{
	Drawer *drawer = networkInterface->getDrawer();
	if (!drawer) return;
	
	if (touchIdentifiers[0] != UINT_MAX)
		touchMoved(touchIdentifiers[0], screenCoords[0].x, screenCoords[0].y, drawer->rayForCoords(screenCoords[0].x, screenCoords[0].y));
	
	if (touchIdentifiers[1] != UINT_MAX)
		touchMoved(touchIdentifiers[1], screenCoords[1].x, screenCoords[1].y, drawer->rayForCoords(screenCoords[1].x, screenCoords[1].y));
}
