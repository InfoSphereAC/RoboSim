/*
 *  NetworkPacket.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 04.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "NetworkPacket.h"

#include <cstdio>

#include "ByteOrder.h"

unsigned NetworkPacket::getNetworkLength() const
{
	return packetLength;
}

bool NetworkPacket::swappedFitsWithinRemainingBytes(unsigned bytes) const
{
    if (bytes < 4) return false;
	
	return (SwapU16LittleToHost(packetLength) <= bytes);
}

void NetworkPacket::swapFromNetwork()
{
#if __BIG_ENDIAN__
#define SWAP(a) (a) = SwapLittleToHost(a)
    SWAP(packetType);
	SWAP(packetLength);
    switch(packetType)
    {
        case LookingForServer:
            SWAP(lookingForServer.versionNumber);
            break;
        case ServerAnnounce:
            SWAP(serverAnnounce.versionNumber);
            break;
            
        case ConnectionRequest:
            SWAP(connectionRequest.versionNumber);
            break;
        case SpeedUpdate:
            for (unsigned i = 0; i < 3; i++)
			{
                SWAP(speedUpdate.motors[i].power);
				SWAP(speedUpdate.motors[i].turnTarget);
				SWAP(speedUpdate.motors[i].turnRatio);
			}
            break;
        case SensorUpdate:
            for (unsigned i = 0; i < 4; i++)
            {
                SWAP(sensorUpdate.sensor[i].type);
                SWAP(sensorUpdate.sensor[i].angle);
            }
            break;
        case CtSPlayTone:
            SWAP(ctsPlayTone.frequency);
            SWAP(ctsPlayTone.duration);
            SWAP(ctsPlayTone.volume);
            break;
        case CtSPlayFile:
            SWAP(ctsPlayFile.volume);
            SWAP(ctsPlayFile.nameLength);
            break;
        case LiftedMove:
			SwapU32LittleToHost(reinterpret_cast<uint32_t *> (liftedMove.delta), 3);
            break;
		case LiftedTurn:
			SWAP(liftedTurn.turnSpeed);
			break;
            
        case ConnectionAccepted:
            SWAP(connectionAccepted.clientID);
            break;
        case RobotUpdate:
            SWAP(robotUpdate.clientID);
            SwapU32LittleToHost(reinterpret_cast<uint32_t *>(robotUpdate.color), 3);
            for (unsigned i = 0; i < 4; i++)
            {
                SWAP(robotUpdate.sensor[i].type);
                SWAP(robotUpdate.sensor[i].angle);
            }
            break;
        case PositionUpdate:
            SWAP(positionUpdate.numRobots);
            SwapU32LittleToHost(reinterpret_cast<uint32_t *>(positionUpdate.robot), sizeof(PositionUpdatePacket::RobotData)/4*positionUpdate.numRobots);
            break;
        case SensorReadings:
			SwapU32LittleToHost(reinterpret_cast<uint32_t *> (sensorReadings.values), 4);
			SwapU32LittleToHost(reinterpret_cast<uint32_t *> (sensorReadings.motorBlockCounterValues), 3);
			SwapU32LittleToHost(reinterpret_cast<uint32_t *> (sensorReadings.motorRotationCounterValues), 3);
			SwapU32LittleToHost(reinterpret_cast<uint32_t *> (sensorReadings.motorTargetCounterValues), 3);
			SwapU32LittleToHost(reinterpret_cast<uint32_t *> (sensorReadings.turnRatio), 3);
            break;
        case RobotDeleted:
            SWAP(robotDeleted.clientID);
            break;
        case StCPlayTone:
            SWAP(stcPlayTone.clientID);
            SWAP(stcPlayTone.frequency);
            SWAP(stcPlayTone.duration);
            SWAP(stcPlayTone.volume);
            break;
        case StCPlayFile:
            SWAP(stcPlayFile.clientID);
            SWAP(stcPlayFile.volume);
            SWAP(stcPlayFile.nameLength);
            break;
        case GridOverview:
            SWAP(gridOverview.cellSize);
            SWAP(gridOverview.cellHeight);
            break;
            
        case SetCell:
            break; // No multi-byte fields
    }
#undef SWAP
#endif
}

void NetworkPacket::swapToNetwork()
{
#if __BIG_ENDIAN__
#define SWAP(a) (a) = SwapHostToLittle(a)
    switch(packetType)
    {
        case LookingForServer:
            SWAP(lookingForServer.versionNumber);
            break;
        case ServerAnnounce:
            SWAP(serverAnnounce.versionNumber);
            break;
            
        case ConnectionRequest:
            SWAP(connectionRequest.versionNumber);
            break;
        case SpeedUpdate:
            for (unsigned i = 0; i < 3; i++)
			{
                SWAP(speedUpdate.motors[i].power);
				SWAP(speedUpdate.motors[i].turnTarget);
				SWAP(speedUpdate.motors[i].turnRatio);
			}
            break;
        case SensorUpdate:
            for (unsigned i = 0; i < 4; i++)
            {
                SWAP(sensorUpdate.sensor[i].type);
                SWAP(sensorUpdate.sensor[i].angle);
            }
            break;
        case CtSPlayTone:
            SWAP(ctsPlayTone.frequency);
            SWAP(ctsPlayTone.duration);
            SWAP(ctsPlayTone.volume);
            break;
        case CtSPlayFile:
            SWAP(ctsPlayFile.volume);
            SWAP(ctsPlayFile.nameLength);
            break;
        case LiftedMove:
			SwapU32LittleToHost(reinterpret_cast<uint32_t *> (liftedMove.delta), 3);
            break;
            
        case ConnectionAccepted:
            SWAP(connectionAccepted.clientID);
            break;
        case RobotUpdate:
            SWAP(robotUpdate.clientID);
            SwapU32LittleToHost(reinterpret_cast<uint32_t *>(robotUpdate.color), 3);
            for (unsigned i = 0; i < 4; i++)
            {
                SWAP(robotUpdate.sensor[i].type);
                SWAP(robotUpdate.sensor[i].angle);
            }
            break;
        case PositionUpdate:
            SwapU32LittleToHost(reinterpret_cast<uint32_t *>(positionUpdate.robot), 20*positionUpdate.numRobots);
            SWAP(positionUpdate.numRobots);
            break;
        case SensorReadings:
			SwapU32LittleToHost(reinterpret_cast<uint32_t *> (sensorReadings.values), 4);
			SwapU32LittleToHost(reinterpret_cast<uint32_t *> (sensorReadings.motorBlockCounterValues), 3);
			SwapU32LittleToHost(reinterpret_cast<uint32_t *> (sensorReadings.motorRotationCounterValues), 3);
			SwapU32LittleToHost(reinterpret_cast<uint32_t *> (sensorReadings.motorTargetCounterValues), 3);
			SwapU32LittleToHost(reinterpret_cast<uint32_t *> (sensorReadings.turnRatio), 3);			
            break;
        case RobotDeleted:
            SWAP(robotDeleted.clientID);
            break;
        case StCPlayTone:
            SWAP(stcPlayTone.clientID);
            SWAP(stcPlayTone.frequency);
            SWAP(stcPlayTone.duration);
            SWAP(stcPlayTone.volume);
            break;
        case StCPlayFile:
            SWAP(stcPlayFile.clientID);
            SWAP(stcPlayFile.volume);
            SWAP(stcPlayFile.nameLength);
            break;
        case GridOverview:
            SWAP(gridOverview.cellSize);
            SWAP(gridOverview.cellHeight);
            break;
            
        case SetCell:
            break; // No multi-byte fields
    }
    SWAP(packetType);
	SWAP(packetLength);
#undef SWAP
#endif
}

void NetworkPacket::print() const
{
	printf("<");
	switch (packetType)
	{
		case ConnectionRequest:
			printf("ConnectionRequest version=%u magicValue=%16s", connectionRequest.versionNumber, connectionRequest.magicValue);
			break;
		case SpeedUpdate:
			printf("SpeedUpdate motors=3");
			break;
		case SensorUpdate:
			printf("SensorUpdate {%u,%f,%u},{%u,%f,%u},{%u,%f,%u},{%u,%f,%u}", sensorUpdate.sensor[0].type, sensorUpdate.sensor[0].angle, sensorUpdate.sensor[0].pointedDown, sensorUpdate.sensor[1].type, sensorUpdate.sensor[1].angle, sensorUpdate.sensor[1].pointedDown, sensorUpdate.sensor[2].type, sensorUpdate.sensor[2].angle, sensorUpdate.sensor[2].pointedDown, sensorUpdate.sensor[3].type, sensorUpdate.sensor[3].angle, sensorUpdate.sensor[3].pointedDown);
			break;
		case CtSPlayTone:
			printf("CtSPlayTone freq=%u dur=%u rep=%u vol=%f", ctsPlayTone.frequency, ctsPlayTone.duration, ctsPlayTone.loops, ctsPlayTone.volume);
			break;
		case LiftedMove:
			printf("LiftedMove");
			break;
		case ConnectionAccepted:
			printf("ConnectionAccepted clientID=%u magicValue=%16s", connectionAccepted.clientID, connectionAccepted.magicValue);
			break;
		case RobotUpdate:
			printf("RobotUpdate clientID=%u sensors={%u,%f,%u},{%u,%f,%u},{%u,%f,%u},{%u,%f,%u}", robotUpdate.clientID, robotUpdate.sensor[0].type, robotUpdate.sensor[0].angle, robotUpdate.sensor[0].pointedDown, robotUpdate.sensor[1].type, robotUpdate.sensor[1].angle, robotUpdate.sensor[1].pointedDown, robotUpdate.sensor[2].type, robotUpdate.sensor[2].angle, robotUpdate.sensor[2].pointedDown, robotUpdate.sensor[3].type, robotUpdate.sensor[3].angle, robotUpdate.sensor[3].pointedDown);
			break;
		case PositionUpdate:
			printf("PositionUpdate numRobots=%u", positionUpdate.numRobots);
			break;
		case SensorReadings:
			printf("SensorReadings values={%f,%f,%f,%f}", sensorReadings.values[0], sensorReadings.values[1], sensorReadings.values[2], sensorReadings.values[3]);
			break;
		case GridOverview:
			printf("GridOverview size=%f height=%f dims={%u,%u}", gridOverview.cellSize, gridOverview.cellHeight, gridOverview.sizeX, gridOverview.sizeZ);
			break;
		case SetCell:
			printf("SetCell pos={%u,%u} isWall=%u shade=%u", setCell.x, setCell.z, (setCell.cell & 0x80) >> 7, setCell.cell & 0x7F);
			break;
		default:
			printf("unknown packet type=%u (%2x %2x %2x %2x)", packetType, packetType & 0xFF, (packetType & 0xFF00) >> 8, (packetType & 0xFF0000) >> 16, (packetType & 0xFF000000) >> 24);
			break;
	}
	printf(">\n");
}