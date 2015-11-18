/*
 *  Controller.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 31.04.10
 *	Edited by Thiemo Leonhardt in 2015
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include "Controller.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

#include <iostream>

#include "Client.h"
#include "Drawer.h"
#include "Environment.h"
#include "EnvironmentEditor.h"
#include "ExecutionContext.h"
#include "NetworkConstants.h"
#include "NetworkPacket.h"
#include "Robot.h"
#include "RobotTouchHandler.h"
#include "Server.h"
#include "ServerBrowser.h"
#include "ShowMessageBoxAndExit.h"
#include "Simulation.h"
#include "Single.h"
#include "Time.h"
#include "TouchesRecognizer.h"
#include "SoundController.h"
#include "UserDefaults.h"
#include "UserInterface.h"

namespace
{
	const float cameraSpeed = 5.0f;
	const float cameraAngularSpeed = float(M_PI);
	//const float cameraPitch = -0.75f;
	const float cameraPitch = -0.6f;
	const float robotCPUTimeFraction = 0.04f;
	const float robotTurnSpeed = float(M_PI) * 0.5f;
	
	const float scrollWheelTurnFactor = float(M_PI) * 2.0f;
	const float scrollFadeoutTime = 0.5f;
}

const Robot *Controller::getLocalRobot() const throw()
{
	if (!networkInterface) return 0;
	return networkInterface->getLocalRobot();
}

Controller::Controller(
#ifdef ANDROID_NDK
					   AndroidAssetManager *mgr,
#endif
					   Controller::NetworkMode mode,
					   unsigned flags,
					   const char* aFile,
					   const char *address,
					   const char *port)
{
	// Platform-specific initalisation
#if defined(ANDROID_NDK)
	// Save asset manager
	assetManager = mgr;
#elif defined(_WIN32)
	// Start WinSock library for networking
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 0), &wsa);
#endif
	startTimeCount();

	networkInterface = NULL;
	
	touchRecognizer = new TouchesRecognizer(this);
	robotTouchHandler = NULL;
	
	upArrow = false;
	downArrow = false;
	leftArrow = false;
	rightArrow = false;
	aDown = false;
	dDown = false;
	sDown = false;
	wDown = false;
	xDown = false;
	cDown = false;
	wheelScrollDirection = 0.0f;
	
	if (aFile)
	{
		filename = new char[strlen(aFile)+1];
		strncpy(filename, aFile, strlen(aFile)+1);
		try
		{
			executionContext = new ExecutionContext(filename);
		}
		catch (std::runtime_error &e)
		{
			ShowErrorAndExit(L"Fehler beim \u00D6ffnen der Bytecode-Datei %s: \"%s\"", L"Error opening bytecode file %s: \"%s\"", filename, e.what());
		}
	}
	else
	{
		filename = NULL;
		executionContext = NULL;
	}
	// environment properties
	environment = new Environment(25, 25, 1.0f, 0.75f);
	simulation = new Simulation(environment);
	editor = new EnvironmentEditor(environment);
	editor->setMode(EnvironmentEditor::None);
	drawer = 0;
	soundController = 0;
	userinterface = 0;
	
	unsigned dataSize;
	char *data = (char *) getDataAndSizeForUserInterfaceKey("environment", dataSize);
	if (data)
		editor->loadFromSerialization((const NetworkPacket *) data);

	
	if (mode == SingleMode)
	{
		startSingle();
		serverBrowser = 0;
	}
	else if (mode == ClientMode)
	{
		if (!address) throw std::invalid_argument("No adress specified for client to connect to.");
		if (!port) port = "10412";
		// Get the address
		struct addrinfo *result;
		if (getaddrinfo(address, port, NULL, &result) != 0)
			throw std::runtime_error("Could not resolve host name.");
		
		for (struct addrinfo *res = result; res != 0; res = res->ai_next)
		{
			if (res->ai_family != AF_INET && res->ai_family != AF_INET6)
				continue; // Maybe other protocols could work, but that seems
			// like too much work.
			
			startAsClient(res->ai_addr, res->ai_family == AF_INET6, flags & ClientFlagUIOnServer);
			serverBrowser = 0;
			break;
		}
		if (!networkInterface) throw std::runtime_error("Not an IPv4 or IPv6 address.");
	}
	else if (mode == ServerMode)
	{
		unsigned short portForServer = networkPortNumber;
		if (port) portForServer = (uint16_t) strtoul("port", NULL, 10);
		startAsServer(portForServer, !(flags & ServerFlagNoBroadcast));
		serverBrowser = 0;
	}
	else // Includes LetUserChoose
		serverBrowser = new ServerBrowser(networkPortNumber);
}

Controller::~Controller()
{
#ifdef ANDROID_NDK
	delete assetManager;
#endif
	delete editor;
	delete drawer;
	delete environment;
	delete simulation;
	delete soundController;
	delete touchRecognizer;
}

void Controller::setCameraSpeedFromButtons()
{
	float speedX = 0.0f;
	float speedZ = 0.0f;
	if (downArrow || sDown) speedZ = -cameraSpeed;
	else if (upArrow || wDown) speedZ = +cameraSpeed;
	
	if (leftArrow) drawer->setCameraAngularSpeedYaw(+cameraAngularSpeed);
	else if (rightArrow) drawer->setCameraAngularSpeedYaw(-cameraAngularSpeed);
	else drawer->setCameraAngularSpeedYaw(0.0f);
	
	if (aDown) speedX = +cameraSpeed;
	else if (dDown) speedX = -cameraSpeed;
	
	setCameraSpeed(speedX, speedZ);
}

void Controller::setCameraSpeed(float tangential, float lateral)
{
	drawer->setCameraSpeedX(tangential);
	drawer->setCameraSpeedZ(lateral);
}
void Controller::rotateCamera(float radians)
{
	drawer->rotateCamera(radians);
}

void Controller::updateTurnSpeed()
{
	if (!networkInterface) return;
	
	if ((scrollFadeoutTime - timeSinceStartedScroll) < 0.0f) wheelScrollDirection = 0.0f;
	float currentScrollwheelTurnSpeed = scrollWheelTurnFactor * wheelScrollDirection * (scrollFadeoutTime - timeSinceStartedScroll);
	
	if (xDown) networkInterface->setRobotTurnSpeed(-robotTurnSpeed);
	else if (cDown) networkInterface->setRobotTurnSpeed(robotTurnSpeed);
	else networkInterface->setRobotTurnSpeed(currentScrollwheelTurnSpeed);	
}

void Controller::initVideo()
{
#ifdef ANDROID_NDK
	drawer = new Drawer(editor, assetManager);
#else
	drawer = new Drawer(editor);
#endif
	// camera position
	//drawer->setCameraPosition(15.0f, 2.5f, 15.0f, cameraPitch, 0.0f, 2.5f);
	//drawer->setCameraPosition(5.0f, 3.5f, 15.0f, cameraPitch, 0.0f, 2.5f);
	drawer->setCameraPosition(-0.2f, 3.0f, -0.2f, cameraPitch, 4.0f, 2.5f);
	userinterface = new UserInterface(this);
	
	if (networkInterface)
	{
		networkInterface->setDrawer(drawer);
	}
}

void Controller::initAudio()
{
	try
	{
		soundController = new SoundController(this);
		soundController->update();
		if (networkInterface)
			networkInterface->setSoundController(soundController);
	}
	catch (std::exception e)
	{
		ShowErrorAndExit(L"Fehler beim Starten des Sound-Systems: \"%s\"", L"Error when starting sound system: \"%s\"", e.what());
	}
}

void Controller::modeSelected(bool isSingle)
{
	if (networkInterface) return;
	
	if (isSingle)
	{
		delete serverBrowser;
		serverBrowser = NULL;
		startSingle();
	}
	else
	{
		if (serverBrowser->getLastServerAddress())
		{
			startAsClient(serverBrowser->getLastServerAddress(), serverBrowser->isLastServerIPv6());
			
			delete serverBrowser;
			serverBrowser = NULL;
		}
		else
		{
			delete serverBrowser;
			serverBrowser = NULL;
			startAsServer(0, true);
		}
	}
}

void Controller::setNetworkInterface(NetworkInterface *newInterface)
{
	networkInterface = newInterface;
	
	editor->setNetworkInterface(networkInterface);
	robotTouchHandler = new RobotTouchHandler(networkInterface);
	if (executionContext) executionContext->setNetworkInterface(networkInterface);
	if (drawer) networkInterface->setDrawer(drawer);
	if (soundController) networkInterface->setSoundController(soundController);
}

void Controller::setIsPaused(bool shoulditpause)
{
	if (executionContext) executionContext->setIsPaused(shoulditpause);
	if (networkInterface) networkInterface->setIsPaused(shoulditpause);
	
	// Note: Recording should only happen when not paused
	if (soundController) soundController->setIsRecording(!shoulditpause);
}
void Controller::togglePause()
{
	setIsPaused(!getIsPaused());
}
bool Controller::getIsPaused() const
{
	if (executionContext) return executionContext->getIsPaused();
	else return false;
}
void Controller::reload()
{
	executionContext->reload();
}
void Controller::chooseFile()
{
	FileChooser *chooser = FileChooser::sharedFileChooser();
	chooser->run(this);
}
void Controller::fileChooserFoundFile(const char *path)
{	
	try
	{
		executionContext = new ExecutionContext(path);
		if (networkInterface) executionContext->setNetworkInterface(networkInterface);
		
		delete [] filename;
		filename = new char[strlen(path) + 1];
		strncpy(filename, path, strlen(path)+1);
	}
	catch (std::runtime_error &e)
	{
		ShowError(L"Fehler beim \u00D6ffnen der Bytecode-Datei %s: \"%s\"", L"Error opening bytecode file %s: \"%s\"", path, e.what());
	}
}

void Controller::moveCamera(float x, float y)
{
	drawer->moveCamera(x, y);
}

void Controller::setWindowSize(float width, float height)
{
	drawer->setWindowSize(width, height);
}
void Controller::setScreenOrientation(Controller::ScreenOrientation orientation)
{
	drawer->setScreenOrientation(Drawer::ScreenOrientation(orientation));
}

void Controller::mouseDown(float rawX, float rawY, unsigned button)
{
	// Ignore right click
	if (button > 1) return;
	
	mouseButtonPressed = true;
	float x, y;
	drawer->transformPoint(rawX, rawY, x, y);
	lastMouseX = x;
	lastMouseY = y;
	
	// Check whether the userinterface has anything to say about this
	if (userinterface->mouseDown(0, x, y)) return;
	
	// Find world ray for click.
	ray4 ray = drawer->rayForCoords(x, y);
	
	// Check whether it hits the robot
	float lengthToLocalRobot = 1.0f;
	bool hitsRobot = false;
	const Robot *localRobot = getLocalRobot();
	if (localRobot)
		hitsRobot = localRobot->hitByRay(ray, lengthToLocalRobot);
	
	// Find actuall cell for that click.
	int cellX, cellZ;
	float lengthToEnvironment;
	bool hitsEnvironment = environment->getFirstCellOnRay(ray, cellX, cellZ, lengthToEnvironment);
	
	if (!hitsRobot && !hitsEnvironment) return;
	
	if (hitsRobot && lengthToLocalRobot < lengthToEnvironment)
	{
		if (robotTouchHandler)
			robotTouchHandler->touchBegan(0, x, y, ray, lengthToLocalRobot);
	}
	else if (hitsEnvironment)
	{
		// When in wall mode, make sure that we can actually edit the cell
		// without causing the robot to jump.
		if ((editor->getMode() == EnvironmentEditor::Height) && !simulation->canToggleCellIsWall(cellX, cellZ))
			return;
		
		// Edit cell
		editor->editCell(cellX, cellZ);
	}
}

void Controller::mouseMove(float rawX, float rawY)
{
	if (!mouseButtonPressed) return;
	
	float x, y;
	drawer->transformPoint(rawX, rawY, x, y);
	if (userinterface->mouseDragged(0, x, y)) return;
	
	lastMouseX = x;
	lastMouseY = y;
	
	// Find world ray for click.
	ray4 ray = drawer->rayForCoords(x, y);
	
	// Find whether this is a drag of the local robot
	const Robot *localRobot = getLocalRobot();
	if (localRobot && localRobot->isLifted())
	{
		if (robotTouchHandler)
			robotTouchHandler->touchMoved(0, x, y, ray);
	}
	else
	{
		// Find actuall cell for that click.
		int cellX, cellZ;
		float length; // Unused
		if (!environment->getFirstCellOnRay(ray, cellX, cellZ, length))
			return;
		
		// When in wall mode, make sure that we can actually edit the cell
		// without causing the robot to jump.
		if ((editor->getMode() == EnvironmentEditor::Height) && !simulation->canToggleCellIsWall(cellX, cellZ))
			return;

		editor->editCell(cellX, cellZ);
	}
}

void Controller::mouseUp(float rawX, float rawY, unsigned button)
{
	// Ignore right mouse button
	if (button > 1) return;
	
	mouseButtonPressed = false;
	
	float x, y;
	drawer->transformPoint(rawX, rawY, x, y);
	
	userinterface->mouseUp(0, x, y);
	
	editor->endEditing();
	
	if (robotTouchHandler)
		robotTouchHandler->touchEnded(0);
}

void Controller::keyDown(int character, bool isPrintable)
{
	if (isPrintable)
	{
		switch(character)
		{
			case 'a':
				aDown = true;
				break;
			case 'd':
				dDown = true;
				break;
			case 's':
				sDown = true;
				break;
			case 'w':
				wDown = true;
				break;
				
			case 'x':
				xDown = true;
				break;
			case 'c':
				cDown = true;
				break;
				
			case '0':
				if (!this->getEnvironmentEditor()->getChallenge()) editor->setMode(EnvironmentEditor::None);
				break;
			case '1':
				if (!this->getEnvironmentEditor()->getChallenge()) editor->setMode(EnvironmentEditor::Height);
					break;
			case '2':
				if (!this->getEnvironmentEditor()->getChallenge()) editor->setMode(EnvironmentEditor::Darken);
				break;
			case '3':
				if (!this->getEnvironmentEditor()->getChallenge()) editor->setMode(EnvironmentEditor::Lighten);
				break;
		}
	}
	else
	{
		switch(character)
		{
			case UpArrow:
				upArrow = true;
				break;
			case DownArrow:
				downArrow = true;
				break;
			case LeftArrow:
				leftArrow = true;
				break;
			case RightArrow:
				rightArrow = true;
				break;
		}
	}
	setCameraSpeedFromButtons();
	updateTurnSpeed();
}

void Controller::keyUp(int character, bool isPrintable)
{
	if (isPrintable)
	{
		switch(character)
		{
			case 'a':
				aDown = false;
				break;
			case 'd':
				dDown = false;
				break;
			case 's':
				sDown = false;
				break;
			case 'w':
				wDown = false;
				break;
				
			case 'x':
				xDown = false;
				break;
			case 'c':
				cDown = false;
				break;
		}
	}
	else
	{
		switch(character)
		{
			case UpArrow:
				upArrow = false;
				break;
			case DownArrow:
				downArrow = false;
				break;
			case LeftArrow:
				leftArrow = false;
				break;
			case RightArrow:
				rightArrow = false;
				break;		
		}
	}
	setCameraSpeedFromButtons();
	updateTurnSpeed();
}

void Controller::scroll(int lines)
{
	if (((lines < 0) && (wheelScrollDirection > 0.0f)) || ((lines > 0) && (wheelScrollDirection < 0.0f)))
	{
		// if this is a reversal of direction, simply stop
		wheelScrollDirection = 0.0f;
	}
	else
	{
		wheelScrollDirection = float(lines);
		timeSinceStartedScroll = 0.0f;
	}
}
	
void Controller::touchDown(unsigned arbitraryIdentifier, float rawX, float rawY)
{
	float x, y;
	drawer->transformPoint(rawX, rawY, x, y);
	lastMouseX = x;
	lastMouseY = y;
	
	if (!userinterface->mouseDown(arbitraryIdentifier, x, y))	
		touchRecognizer->touchDown(arbitraryIdentifier, x, y);
}
void Controller::touchMoved(unsigned arbitraryIdentifier, float rawX, float rawY)
{
	float x, y;
	drawer->transformPoint(rawX, rawY, x, y);
	lastMouseX = x;
	lastMouseY = y;
	
	if (!userinterface->mouseDragged(arbitraryIdentifier, x, y))
		touchRecognizer->touchMoved(arbitraryIdentifier, x, y);
}
void Controller::touchUp(unsigned arbitraryIdentifier)
{
	if (userinterface->mouseUp(arbitraryIdentifier, lastMouseX, lastMouseY))
		return;
	
	touchRecognizer->touchUp(arbitraryIdentifier);
}
void Controller::touchCancelled(unsigned arbitraryIdentifier)
{
	if (!userinterface->mouseUp(arbitraryIdentifier, lastMouseX, lastMouseY))
		touchRecognizer->touchCancelled(arbitraryIdentifier);
}

void Controller::tapDown(unsigned identifier, float x, float y)
{
	// Find world ray for click.
	ray4 ray = drawer->rayForCoords(x, y);
	
	// Check whether it hits the robot
	float lengthToLocalRobot = 1.0f;
	bool hitsRobot = false;
	const Robot *localRobot = getLocalRobot();
	if (localRobot)
		hitsRobot = localRobot->touchHitByRay(ray, lengthToLocalRobot, 1.47f);
	
	// Find actuall cell for that click.
	int cellX, cellZ;
	float lengthToEnvironment;
	bool hitsEnvironment = environment->getFirstCellOnRay(ray, cellX, cellZ, lengthToEnvironment);
	
	if (!hitsRobot && !hitsEnvironment) return;
	
	if (hitsRobot && lengthToLocalRobot < lengthToEnvironment)
	{
		if (robotTouchHandler)
			robotTouchHandler->touchBegan(identifier, x, y, ray, lengthToLocalRobot);
	}
	else if (hitsEnvironment)
	{
		// When in wall mode, make sure that we can actually edit the cell
		// without causing the robot to jump.
		if ((editor->getMode() == EnvironmentEditor::Height) && !simulation->canToggleCellIsWall(cellX, cellZ))
			return;
		
		// Edit cell
		editor->editCell(cellX, cellZ);
	}
}
void Controller::tapDragged(unsigned identifier, float x, float y)
{
	// Find world ray for click.
	ray4 ray = drawer->rayForCoords(x, y);
	
	// Find whether this is a drag of the local robot
	const Robot *localRobot = getLocalRobot();
	if (localRobot && localRobot->isLifted())
	{
		if (robotTouchHandler)
			robotTouchHandler->touchMoved(identifier, x, y, ray);
	}
	else
	{
		// Find actuall cell for that click.
		int cellX, cellZ;
		float length; // Unused
		if (!environment->getFirstCellOnRay(ray, cellX, cellZ, length))
			return;
		
		// When in wall mode, make sure that we can actually edit the cell
		// without causing the robot to jump.
		if ((editor->getMode() == EnvironmentEditor::Height) && !simulation->canToggleCellIsWall(cellX, cellZ))
			return;
		
		editor->editCell(cellX, cellZ);
	}
}
void Controller::tapUp(unsigned identifier, float x, float y)
{
	editor->endEditing();
	
	const Robot *localRobot = getLocalRobot();
	if (!localRobot) return;
	
	robotTouchHandler->touchEnded(identifier);
}
void Controller::doubletap(float x, float y)
{
	// Find world ray for click.
	ray4 ray = drawer->rayForCoords(x, y);
	
	// Check whether it hits the robot
	float lengthToLocalRobot = 1.0f;
	bool hitsRobot = false;
	const Robot *localRobot = getLocalRobot();
	if (!localRobot) return;
	
	hitsRobot = localRobot->touchHitByRay(ray, lengthToLocalRobot, 1.47f);
	if (!hitsRobot) return;
	
	// Find actuall cell for that click. Needed in case a cell was hit instead of the robot behind it.
	int cellX, cellZ;
	float lengthToEnvironment;
	bool hitsEnvironment = environment->getFirstCellOnRay(ray, cellX, cellZ, lengthToEnvironment);
	
	if (!hitsEnvironment || lengthToLocalRobot < lengthToEnvironment)
	{
		// Did actually hit robot.
		userinterface->toggleShowSensorScreen();
	}
}

void Controller::motion(unsigned identifier, float fromX, float fromY, float toX, float toY)
{
	drawer->setCameraSpeedZ((toY - fromY)*-2.0f);
	if (fabsf(toX-fromX) > fabsf(toY-fromY)*0.5f)
		drawer->setCameraAngularSpeedYaw((toX - fromX)*0.5f);
	else
		drawer->setCameraAngularSpeedYaw(0.0f);
}
void Controller::motionEnded(unsigned identifier)
{
	drawer->setCameraSpeedX(0.0);
	drawer->setCameraSpeedZ(0.0);
	drawer->setCameraAngularSpeedYaw(0.0f);
}

void Controller::startSingle()
{
	if (networkInterface) return;
	
	setNetworkInterface(new Single(simulation));
	
}

void Controller::startAsServer(unsigned short port, bool broadcast)
{
	if (networkInterface) return;
	
	if (port == 0) port = networkPortNumber;
	unsigned flags = 0;
	if (executionContext) flags |= Server::CreateLocalRobot;
	if (broadcast) flags |= Server::AllowDiscovery;

	setNetworkInterface(new Server(port, simulation, editor, flags));
}

void Controller::startAsClient(const struct sockaddr *networkAddress, bool ipv6, bool uiOnServer)
{
	if (networkInterface) return;
	
	setNetworkInterface(new Client(networkAddress, ipv6, editor, uiOnServer));
}

void Controller::update(float delta)
{
	if (!networkInterface)
	{
		try
		{
			serverBrowser->update();
		}
		catch (std::runtime_error e)
		{
			std::cerr << "Error updating server browser: " << e.what() << std::endl;
		}
		return;
	}
	
	const Robot *localRobot = getLocalRobot();
	if (localRobot && localRobot->isLifted())
	{
		if (robotTouchHandler)
			robotTouchHandler->update();
	}
	
	touchRecognizer->update(delta);
	
	if (wheelScrollDirection != 0.0f)
	{
		timeSinceStartedScroll += delta;
		updateTurnSpeed();
	}
	
	if (executionContext) executionContext->runForTime(delta * robotCPUTimeFraction);
	drawer->updateCamera(delta);
	simulation->update(delta);
	soundController->update();
	networkInterface->update();
}

void Controller::draw()
{
	drawer->draw();
}

const float4 &Controller::getCameraLocation(void) const throw()
{
	return drawer->getCameraLocation();
}

const matrix &Controller::getCameraMatrix(void) const throw()
{
	return drawer->getCameraMatrix();
}

float4 Controller::getCameraVelocity(void) const throw()
{
	return drawer->getCameraVelocity();
}

void Controller::shutDown(void)
{
	NetworkPacket *grid = editor->writeToSerialization();
	
	setDataForUserInterfaceKey("environment", grid->getNetworkLength(), (void *) grid);
	
	free(grid);
}
