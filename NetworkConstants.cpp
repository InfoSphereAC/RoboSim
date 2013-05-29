/*
 *  NetworkConstants.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 13.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "NetworkConstants.h"

const char serverToClientHandshake[] = { 'R', 'o', 'b', 'o', 'S', 'i', 'm', 'S', 'e', 'r', 'v', 'e', 'r', 'H', 'i', '!' };
const char clientToServerHandshake[] = { 'R', 'o', 'b', 'o', 'S', 'i', 'm', 'C', 'l', 'i', 'e', 'n', 't', 'H', 'i', '!' };
const char serverSearchToken[]       = { 'R', 'o', 'b', 'o', 'S', 'i', 'm', 'A', 'n', 'y', 'S', 'e', 'r', 'v', 'e', 'r' };
const char serverBroadcastToken[]    = { 'R', 'o', 'b', 'o', 'S', 'i', 'm', 'I', 'A', 'm', 'S', 'e', 'r', 'v', 'e', 'r' };

unsigned protocolVersionNumber = 6;
// New in v3: turn robot that has been picked up.
// New in v4: Changed SpeedUpdate for richer motor representation.
// New in v5: packet length added, flags in connectionRequest, port assignment changed to sensors
// New in v6: Robot can be turned directly.

unsigned maxBacklog = 8;
unsigned networkPortNumber = 10412;