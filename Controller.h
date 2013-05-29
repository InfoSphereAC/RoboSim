/*
 *  Controller.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 31.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include <vector>

#ifdef ANDROID_NDK
#include <stdlib.h>
#include <asset_manager.h>
#endif

#include "Vec4.h"
#include "FileChooser.h"

class Drawer;
class Environment;
class EnvironmentEditor;
class ExecutionContext;
class NetworkInterface;
class Robot;
class RobotTouchHandler;
class ServerBrowser;
class Simulation;
class SoundController;
class TouchesRecognizer;
class UserInterface;

class Controller : public FileChooserDelegate
{
#ifdef ANDROID_NDK
	AndroidAssetManager *assetManager;
#endif
	Drawer *drawer;
	Environment *environment;
	EnvironmentEditor *editor;
	ExecutionContext *executionContext;
	NetworkInterface *networkInterface;
	Simulation *simulation;
	SoundController *soundController;
	ServerBrowser *serverBrowser;
	UserInterface *userinterface;
	
	TouchesRecognizer *touchRecognizer;
	RobotTouchHandler *robotTouchHandler;
	
	char *filename;
	
	bool upArrow;
	bool downArrow;
	bool leftArrow;
	bool rightArrow;
	bool aDown;
	bool dDown;
	bool sDown;
	bool wDown;
	bool xDown;
	bool cDown;
	bool mouseButtonPressed;
	
	float lastMouseX;
	float lastMouseY;
	
	float timeSinceStartedScroll;
	float wheelScrollDirection;
	
	void setCameraSpeedFromButtons();
	void updateTurnSpeed();
		
	void setNetworkInterface(NetworkInterface *interface);
public:
	enum NetworkMode
	{
		LetUserChoose,
		SingleMode,
		ClientMode,
		ServerMode
	};
	
	enum NetworkFlags
	{
		ClientFlagUIOnServer = 1 << 0,
		
		ServerFlagNoBroadcast = 1 << 0
	};
	
#ifndef ANDROID_NDK
	Controller(NetworkMode mode, unsigned flags, const char* filename, const char *address, const char *port);
#else
	Controller(AndroidAssetManager *mgr, NetworkMode mode, unsigned flags, const char* filename, const char *address, const char *port);
#endif
	~Controller();
	
	void initAudio();
	
	void initVideo();
	void setWindowSize(float width, float height);
	enum ScreenOrientation
	{
		Normal,
		RotatedLeft,
		RotatedRight,
		UpsideDown
	};	
	void setScreenOrientation(ScreenOrientation orientation);
	
	void setCameraSpeed(float screenX, float screenY);
	void rotateCamera(float radians);
	void moveCamera(float x, float y);
	
	void startSingle();
	void startAsServer(unsigned short port, bool autodiscovery);
	void startAsClient(const struct sockaddr *networkAddress, bool ipv6, bool uiOnServer = false);
	
	// From User interface
	void modeSelected(bool isSingle);
	void setIsPaused(bool shoulditpause);
	void togglePause();
	bool getIsPaused() const;
	void reload();
	void chooseFile();
	
	// Mouse events
	void mouseDown(float x, float y, unsigned button);
	void mouseMove(float newX, float newY);
	void mouseUp(float x, float y, unsigned button);
	
	void scroll(int lines);
	
	// Key events
	enum UnprintableKeys
	{
		UpArrow = 0,
		DownArrow,
		LeftArrow,
		RightArrow
	};
	
	void keyDown(int character, bool isPrintable);
	void keyUp(int character, bool isPrintable);

	// Touch events (raw)
	void touchDown(unsigned arbitraryIdentifier, float x, float y);
	void touchMoved(unsigned arbitraryIdentifier, float x, float y);
	void touchUp(unsigned arbitraryIdentifier);
	void touchCancelled(unsigned arbitraryIdentifier);
	
	// Touch events (processed)
	void tapDown(unsigned identifier, float x, float y);
	void tapDragged(unsigned identifier, float x, float y);
	void tapUp(unsigned identifier, float x, float y);
	void doubletap(float x, float y);
	void motion(unsigned identifier, float lastX, float lastY, float newX, float newY);
	void motionEnded(unsigned identifier);

	// File loading
	virtual void fileChooserFoundFile(const char *path);
	
	void update(float delta);
	void draw();
	
	const float4 &getCameraLocation(void) const throw();
	const matrix &getCameraMatrix(void) const throw();
	float4 getCameraVelocity() const throw();
	
	Drawer *getDrawer() { return drawer; }
	EnvironmentEditor *getEnvironmentEditor() { return editor; }
	NetworkInterface *getNetworkInterface() { return networkInterface; }
	
	const Robot *getLocalRobot() const throw();
	
	void shutDown();
};
