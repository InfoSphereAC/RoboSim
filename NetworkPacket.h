/*
 *  NetworkPacket.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 04.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdint.h>

#include "Vec4.h"

#ifdef _MSC_VER
#define PACKED
#pragma pack(push, 1)
#else
#define PACKED __attribute__((packed))
#endif

// These two are obviously the same, I keep them (and ConnectionRequest)
// separate so I don’t get confused.
struct LookingForServerPacket
{
	uint16_t packetType;
	uint16_t packetLength;
	uint32_t versionNumber;
	uint8_t magicValue[16];
} PACKED;

struct ServerAnnouncePacket
{
	uint16_t packetType;
	uint16_t packetLength;
	uint32_t versionNumber;
	uint8_t magicValue[16];
} PACKED;

enum ConnectionRequestFlags
{
	NetworkControlledByServer = 1 << 0	// Things like lifting and sensor configuration should be handled by the server, as the server's local robot.
	// This is only allowed if the server has no local robot already.
};

struct ConnectionRequestPacket
{
	uint16_t packetType;
	uint16_t packetLength;
	uint32_t versionNumber;
	uint8_t magicValue[16];
	uint32_t flags; // 0x1: Use as loca
} PACKED;

enum SpeedUpdateFlags
{
	// Used per motor
	NetworkSetPower = 1 << 0,
	NetworkSetTurnTarget = 1 << 1,
	NetworkSetTurnRatio = 1 << 2,
	NetworkResetBlockCounter = 1 << 3,
	NetworkResetRotationCounter = 1 << 4,
	NetworkResetAllCounters = 1 << 5, // Notice: Not an OR of others because it also resets the target counter, which has no dedicated reset flag.
	
	// Used for global packet
	NetworkChangeMotorASynchronized = 1 << 0,
	NetworkChangeMotorBSynchronized = 1 << 1,
	NetworkChangeMotorCSynchronized = 1 << 2,
};

struct SpeedUpdatePacket
{
	uint16_t packetType;
	uint16_t packetLength;
	
	uint8_t flags;
	uint8_t isMotorSynchronized;
	
	struct individualMotor {
		uint32_t flags; // values: OR of things in SpeedUpdateFlags
		float power; // only defined when NetworkSetPower is set in flags
		float turnTarget; // only defined when NetworkSetTargetCounter is set in flags
		float turnRatio; // only defined when NetworkSetTurnRatio is set in flags
	} motors[3] PACKED;
} PACKED;

struct SensorUpdatePacket
{
	uint16_t packetType;
	uint16_t packetLength;
	
	uint8_t leftMotorIndex;		// 255: No change
	uint8_t rightMotorIndex;	// 255: No change
	
	struct {
		uint32_t type; // Values from Robot::SensorType
		float angle;
		uint8_t pointedDown;
	} PACKED sensor[4];	
} PACKED;

struct CtSPlayTonePacket
{
	uint16_t packetType;
	uint16_t packetLength;
	uint32_t frequency;
	uint32_t duration;
	uint8_t loops;
	float volume;
} PACKED;

struct CtSPlayFilePacket
{
	uint16_t packetType;
	uint16_t packetLength;
	uint8_t loops;
	float volume;
	
	uint32_t nameLength;
	uint8_t name[]; // Has to be zero-terminated
} PACKED;

struct LiftedMovePacket
{
	uint16_t packetType;
	uint16_t packetLength;
	float delta[3];
	uint8_t isLifted;
} PACKED;

struct LiftedTurnPacket
{
	uint16_t packetType;
	uint16_t packetLength;
	float turnSpeed;
	float turnDirectly;
} PACKED;

struct ConnectionAcceptedPacket
{
	uint16_t packetType;
	uint16_t packetLength;
	uint32_t clientID;
	uint8_t magicValue[16];
} PACKED;

struct RobotUpdatePacket
{
	uint16_t packetType;
	uint16_t packetLength;
	
	uint32_t clientID; // ID of the robot. The value will stay the same even
	// if people log on or off, and is not guaranteed to have any relation to
	// anything or follow any particular pattern.
	
	float color[3];
	
	struct {
		uint32_t type; // Values from Robot::SensorType
		float angle;
		uint8_t pointedDown;
	} PACKED sensor[4];	
} PACKED;

struct PositionUpdatePacket
{
	uint16_t packetType;
	uint16_t packetLength;
	
	uint32_t numRobots;
	struct RobotData {
		uint32_t clientID;
		float position[16];
		float trackSpeed[2]; // 0: Left, 1: Right
		uint32_t isLifted;
	} PACKED robot[];
} PACKED;

struct SensorReadingsPacket
{
	uint16_t packetType;
	uint16_t packetLength;
	// Does not include robot ID, as it only gets send to the correct client.
	float values[4];
	
	// Also transmits motor counter values
	float motorBlockCounterValues[3];
	float motorRotationCounterValues[3];
	float motorTargetCounterValues[3];
	
	// Only needed for reading back out, to make sure Client and Server have
	// consistent state.
	float turnRatio[3];
	uint8_t synchronizedMotors;
} PACKED;

struct RobotDeletedPacket
{
	uint16_t packetType;
	uint16_t packetLength;
	
	uint32_t clientID;
} PACKED;

struct StCPlayTonePacket
{
	uint16_t packetType;
	uint16_t packetLength;
	uint32_t clientID;
	uint32_t frequency;
	uint32_t duration;
	uint8_t loops;
	float volume;
} PACKED;

struct StCPlayFilePacket
{
	uint16_t packetType;
	uint16_t packetLength;
	uint32_t clientID;
	uint8_t loops;
	float volume;
	
	uint32_t nameLength;
	uint8_t name[];
} PACKED;

struct GridOverviewPacket
{
	uint16_t packetType;
	uint16_t packetLength;
	float cellSize;
	float cellHeight;
	uint8_t sizeX;
	uint8_t sizeZ;
	
	uint8_t cells[];
	// cells[i] & 0x80 ==> is wall
	// cells[i] & 0x7F ==> shade
} PACKED;

struct SetCellPacket
{
	uint16_t packetType;
	uint16_t packetLength;
	uint8_t x;
	uint8_t z;
	uint8_t cell;
	// cell & 0x80 ==> is wall
	// cell & 0x7F ==> shade
} PACKED;


union NetworkPacket
{
	enum PacketType {
		// Server browsing
		LookingForServer = 0,
		ServerAnnounce,
		
		// Client to Server
		ConnectionRequest = 100,
		SpeedUpdate,
		SensorUpdate,
		CtSPlayTone,
		CtSPlayFile,
		LiftedMove,
		LiftedTurn,
		
		// Server to Client
		ConnectionAccepted = 200,
		RobotUpdate,
		PositionUpdate,
		SensorReadings,
		RobotDeleted,
		StCPlayTone,
		StCPlayFile,
		GridOverview,
		
		// Either to either
		SetCell = 300
	};
	
	struct {
		uint16_t packetType;
		uint16_t packetLength;
	};
	
	LookingForServerPacket lookingForServer;
	ServerAnnouncePacket serverAnnounce;
	
	ConnectionRequestPacket connectionRequest;
	SpeedUpdatePacket speedUpdate;
	SensorUpdatePacket sensorUpdate;
	CtSPlayTonePacket ctsPlayTone;
	CtSPlayFilePacket ctsPlayFile;
	LiftedMovePacket liftedMove;
	LiftedTurnPacket liftedTurn;
	
	ConnectionAcceptedPacket connectionAccepted;
	RobotUpdatePacket robotUpdate;
	PositionUpdatePacket positionUpdate;
	SensorReadingsPacket sensorReadings;
	RobotDeletedPacket robotDeleted;
	StCPlayTonePacket stcPlayTone;
	StCPlayFilePacket stcPlayFile;
	GridOverviewPacket gridOverview;
	
	SetCellPacket setCell;
	
	unsigned getNetworkLength() const;
    bool swappedFitsWithinRemainingBytes(unsigned bytes) const;

	// Byte ordering
    void swapFromNetwork();
    void swapToNetwork();
    
	// For debug purposes
	void print() const;
};

#ifdef _MSC_VER
#pragma pack(pop, 1)
#endif
