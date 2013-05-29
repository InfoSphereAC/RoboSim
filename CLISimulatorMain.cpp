/*
 *  CLISimulatorMain.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 26.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include <iostream>

#include "Interpreter.h"
#include "System.h"
#include "RXEFile.h"

void printUsageAndExit()
{
	std::cout << "Crude Simulator for RXE files." << std::endl;
	std::cout << "Usage: RXEAnalyzer inputfile" << std::endl;
	exit(0);
}

int main(int argc, char *argv[])
{
	if (argc != 2) printUsageAndExit();
	
	System *system = new System;
	RXEFile *file = new RXEFile(argv[1]);
	Interpreter *interp = new Interpreter(file, system);
	
	while (true)
	{
		interp->step();
	}
	
	return 0;
}
