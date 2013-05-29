/*
 *  Drawer.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include <map>
#include <vector>
#include <string>

#ifdef ANDROID_NDK
#include <asset_manager.h>
#endif

#include "Vec4.h"

class EnvironmentDrawer;
class EnvironmentEditor;
class Model;
class Robot;
class RobotDrawer;
class Texture;
class UserInterface;

class Drawer
{
public:
	enum ScreenOrientation
	{
		Normal,
		RotatedLeft,
		RotatedRight,
		UpsideDown
	};
	
protected:
	EnvironmentDrawer *environmentDrawer;
	std::vector<RobotDrawer *> robotDrawers;
	UserInterface *userinterface;
	
	matrix frustumMatrix;
	matrix inverseFrustumMatrix;
	matrix orthogonalMatrix;
	ScreenOrientation orientation;
	
	matrix modelMatrix;
	matrix inverseModelMatrix;
	float4 cameraPosition;
	float cameraPitch;
	float cameraYaw;
	float cameraZoom;
	
	float cameraSpeedX;
	float cameraSpeedY;
	float cameraSpeedZ;
	float cameraAngularSpeedPitch;
	float cameraAngularSpeedYaw;
	
	float screenHeight;
	float screenWidth;
	
#ifdef ANDROID_NDK
	AndroidAssetManager *assetManager;
#endif
	std::map<std::string, Model *> models;
	std::map<std::string, Texture *> textures;
	std::map<std::string, std::vector<float4> > debugLines;
	
public:
#ifdef ANDROID_NDK
	Drawer(EnvironmentEditor *editor, AndroidAssetManager *assetManager);
#else
	Drawer(EnvironmentEditor *editor);
#endif
	
	void addRobot(Robot *aRobot);
	void removeRobot(Robot *aRobot);
	
	void setUserInterface(UserInterface *anInterface) { userinterface = anInterface; }
	void setWindowSize(float newWidth, float newHeight);
	
	void updateCamera(float timedelta);
	void draw();
	
	void setCameraPosition(float x, float y, float z, float pitch, float yaw, float zoom);
	void setCameraSpeedX(float v);
	void setCameraSpeedY(float v);
	void setCameraSpeedZ(float v);
	void setCameraAngularSpeedPitch(float omega);
	void setCameraAngularSpeedYaw(float omega);
	
	void rotateCamera(float radians);
	void moveCamera(float x, float y);
	
	const float4 &getCameraLocation() const throw() { return cameraPosition; }
	const matrix &getCameraMatrix() const throw() { return inverseModelMatrix; }
	float4 getCameraVelocity() const throw();
	
	float getScreenWidth() const throw();
	float getScreenHeight() const throw();
	
	/*!
	 * @abstract Transforms a point from window coordinates into the
	 * current drawing coord space.
	 * @discussion This takes the coordinates from the current window
	 * system and transforms them into a system where the origin is in
	 * the logical left corner of the screen, taking into account 
	 * inconsistencies and current rotation.
	 */
	void transformPoint(float inX, float inY, float &outX, float &outY);
	/*!
	 * @abstract Calculates ray from near to far clipping plane.
	 * @discussion x and y should be transformed with transformPoint first.
	 */
	ray4 rayForCoords(float x, float y);
	
	Texture *textureWithFilename(const std::string &filename);
	Model *modelWithFilename(const std::string &filename);
#ifdef ANDROID_NDK
	AndroidAssetManager *getAssetManager() { return assetManager; }
#endif
	
	void setScreenOrientation(ScreenOrientation orientation);
	ScreenOrientation getScreenOrientation() const { return orientation; }
	
	void debugLineClear(const std::string &name);
	void debugLineAddSegment(const std::string &name, const float4 &a, const float4 &b);
	
	static void clearDebugLine(const char *name);
	static void addDebugLineSegment(const char *name, const float4 &a, const float4 &b);
};
