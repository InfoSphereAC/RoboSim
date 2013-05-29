/*
 *  Drawer.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include "Drawer.h"

#include "EnvironmentDrawer.h"
#include "Model.h"
#include "RobotDrawer.h"
#include "ShowMessageBoxAndExit.h"
#include "Texture.h"
#include "UserInterface.h"

#define PROC_POINTERS_NOT_EXTERN 1
#include "OpenGL.h"

#include <iostream>

#if NEEDS_GL_FUNCTION_POINTERS
// For function pointers
#include <SDL.h>
#endif

namespace
{
	const GLfloat lightAmbient [] = { 0.5f, 0.5f, 0.5f, 1.0f };
	const GLfloat lightDiffuse [] = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	const GLfloat materialAmbient [] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat materialSpecular [] = { 0.1f, 0.1f, 0.1f, 1.0f };
	
	const GLfloat lightPosition [] = { -20.0f, 50.0f, -5.0f, 1.0f };
	
	Drawer *globalDebugDrawer;
}

#ifdef ANDROID_NDK
Drawer::Drawer(EnvironmentEditor *editor, AndroidAssetManager *mgr)
{
	assetManager = mgr;
#else
Drawer::Drawer(EnvironmentEditor *editor)
{
#endif
	orientation = Normal;
	
#if !GLES
	// Find out if we have OpenGL 1.5 or, failing that, support for
	// GL_ARB_vertex_buffer_object
	const char *version = (const char *) glGetString(GL_VERSION);
	if (!version)
	{
		ShowErrorAndExit(L"Konnte OpenGL-Version nicht bestimmen.", L"Could not find OpenGL version");
	}
	unsigned major, minor;
	sscanf(version, "%u.%u", &major, &minor);
	if (major <= 1 && minor < 4)
	{
		// Earlier than OpenGL 1.5, so check for extension instead.
		
		const char *extensions = (const char *) glGetString(GL_EXTENSIONS);
		if (!strstr(extensions, "GL_ARB_vertex_buffer_object"))
		{
			ShowErrorAndExit(L"Ihre Grafikkarte unterst\u00FCtzt wichtige Funktionen nicht. Bitte aktualisieren sie ihre Treiber.", L"Your graphics card does not support some required functionality. Please update your drivers.");
		}
	}
#endif
	
#if NEEDS_GL_FUNCTION_POINTERS
	// Setup OpenGL function pointers.
	// Fall back to version with ARB suffix in the extremely rare case that the 
	// driver supports the extension, but not yet OpenGL 1.5
	glGenBuffers = (PFNGLGENBUFFERSPROC) SDL_GL_GetProcAddress("glGenBuffers");
	if (!glGenBuffers) glGenBuffers = (PFNGLGENBUFFERSPROC) SDL_GL_GetProcAddress("glGenBuffersARB");
	
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SDL_GL_GetProcAddress("glDeleteBuffers");
	if (!glDeleteBuffers) glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SDL_GL_GetProcAddress("glDeleteBuffersARB");
	
	glBindBuffer = (PFNGLBINDBUFFERPROC) SDL_GL_GetProcAddress("glBindBuffer");
	if (!glBindBuffer) glBindBuffer = (PFNGLBINDBUFFERPROC) SDL_GL_GetProcAddress("glBindBufferARB");
	
	glBufferData = (PFNGLBUFFERDATAPROC) SDL_GL_GetProcAddress("glBufferData");
	if (!glBufferData) glBufferData = (PFNGLBUFFERDATAPROC) SDL_GL_GetProcAddress("glBufferDataARB");
	
	glBufferSubData = (PFNGLBUFFERSUBDATAPROC) SDL_GL_GetProcAddress("glBufferSubData");
	if (!glBufferSubData) glBufferSubData = (PFNGLBUFFERSUBDATAPROC) SDL_GL_GetProcAddress("glBufferSubDataARB");
	
	if (!glGenBuffers || !glDeleteBuffers || !glBindBuffer || !glBufferData || !glBufferSubData)
	{
		ShowErrorAndExit(L"Einige Grafikfunktionen konnten nicht geladen werden. Bitte aktualisieren sie ihre Treiber.", L"Some graphics functions could not be loaded. Please update your drivers.");
	}
	
#endif
	// Set up OpenGL properties
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); checkGLError();
	glEnable(GL_DEPTH_TEST); checkGLError();
	glEnableClientState(GL_VERTEX_ARRAY); checkGLError();
	glEnableClientState(GL_NORMAL_ARRAY); checkGLError();
	glEnableClientState(GL_COLOR_ARRAY); checkGLError();
	glEnableClientState(GL_TEXTURE_COORD_ARRAY); checkGLError();
	
	glEnable(GL_LIGHTING); checkGLError();
	glEnable(GL_LIGHT0); checkGLError();
	glEnable(GL_COLOR_MATERIAL); checkGLError();
	
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightAmbient); checkGLError();
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse); checkGLError();
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition); checkGLError();
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient); checkGLError();
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular); checkGLError();
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 6.0f); checkGLError();
	
	glEnable(GL_TEXTURE_2D); checkGLError();
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); checkGLError();
	
	// Set up drawers for the various sub-elements
	environmentDrawer = new EnvironmentDrawer(editor, this);
	checkGLError();
	
	setCameraPosition(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	updateCamera(0.0f);
	checkGLError();
	
	globalDebugDrawer = this;
}

void Drawer::addRobot(Robot *aRobot)
{
	robotDrawers.push_back(new RobotDrawer(aRobot, this));
}

void Drawer::removeRobot(Robot *aRobot)
{
	for (std::vector<RobotDrawer *>::iterator iter = robotDrawers.begin(); iter != robotDrawers.end(); ++iter)
	{
		if ((*iter)->getRobot() == aRobot)
		{
			delete *iter;
			robotDrawers.erase(iter);
			return;
		}
	}
}

void Drawer::setWindowSize(float newWidth, float newHeight)
{	
	checkGLError();
	glViewport(0, 0, GLsizei(newWidth), GLsizei(newHeight)); checkGLError();

	screenWidth = newWidth;
	screenHeight = newHeight;
	float aspect = screenWidth/screenHeight;
	frustumMatrix = matrix::frustum(65.0f, aspect, 1.0f, 100.0f);
	inverseFrustumMatrix = matrix::inverseFrustum(65.0f, aspect, 1.0f, 100.0f);
	orthogonalMatrix = matrix();
	orthogonalMatrix.x.x = 2.0f / screenWidth;
	orthogonalMatrix.y.y = 2.0f / screenHeight;
	orthogonalMatrix.w.x = -1.0f;
	orthogonalMatrix.w.y = -1.0f;
	
	if (orientation == RotatedLeft)
	{
		float4 x = frustumMatrix.x;
		frustumMatrix.x = frustumMatrix.y;
		frustumMatrix.y = -x;
		
		inverseFrustumMatrix = matrix::inverseFrustum(aspect*65.0f, screenHeight/screenWidth, 1.0f, 100.0f);
		
		orthogonalMatrix = matrix();
		orthogonalMatrix.x.x = 0.0f;
		orthogonalMatrix.x.y = 2.0f / screenHeight;
		orthogonalMatrix.y.x = -2.0f / screenWidth;
		orthogonalMatrix.y.y = 0.0f;
		orthogonalMatrix.w.x = +1.0f;
		orthogonalMatrix.w.y = -1.0f;		
	}
	else if (orientation == RotatedRight)
	{
		float4 y = frustumMatrix.y;
		frustumMatrix.y = frustumMatrix.x;
		frustumMatrix.x = -y;
		
		inverseFrustumMatrix = matrix::inverseFrustum(aspect*65.0f, screenHeight/screenWidth, 1.0f, 100.0f);
		
		orthogonalMatrix = matrix();
		orthogonalMatrix.x.x = 0.0f;
		orthogonalMatrix.x.y = -2.0f / screenHeight;
		orthogonalMatrix.y.x = 2.0f / screenWidth;
		orthogonalMatrix.y.y = 0.0f;
		orthogonalMatrix.w.x = -1.0f;
		orthogonalMatrix.w.y = +1.0f;
	}
	else if (orientation == UpsideDown)
	{
		frustumMatrix.x *= -1.0f;
		frustumMatrix.y *= -1.0f;
		
		orthogonalMatrix.x.x *= -1.0f;
		orthogonalMatrix.y.y *= -1.0f;
		orthogonalMatrix.w.x *= -1.0f;
		orthogonalMatrix.w.y *= -1.0f;
	}
	
	glMatrixMode(GL_PROJECTION); checkGLError();
	glLoadMatrixf(frustumMatrix.c_ptr()); checkGLError();
	glMatrixMode(GL_MODELVIEW); checkGLError();
}

void Drawer::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Draw lit objects
	glEnable(GL_LIGHTING);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnable(GL_COLOR_MATERIAL);

	
	glLoadIdentity();
	glLoadMatrixf(modelMatrix.c_ptr());
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	
	// Draw environment
	glEnableClientState(GL_COLOR_ARRAY);
	environmentDrawer->draw();
	glDisableClientState(GL_COLOR_ARRAY);
	
	for (std::vector<RobotDrawer *>::iterator iter = robotDrawers.begin(); iter != robotDrawers.end(); ++iter)
		(*iter)->draw();
	
	// Draw unlit objects
	glDisable(GL_LIGHTING);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisable(GL_COLOR_MATERIAL);
		
	// - Draw 2D objects
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	
	glLoadMatrixf(orthogonalMatrix.c_ptr());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glEnable(GL_BLEND);
	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (orientation == RotatedLeft || orientation == RotatedRight)
		userinterface->draw(screenHeight, screenWidth);
	else
		userinterface->draw(screenWidth, screenHeight);
	
	glDisable(GL_BLEND);
	
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(frustumMatrix.c_ptr());
	
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
}

void Drawer::setScreenOrientation(Drawer::ScreenOrientation newOrientation)
{
	orientation = newOrientation;
	this->setWindowSize(screenWidth, screenHeight);
}

void Drawer::updateCamera(float timedelta)
{
	for (std::vector<RobotDrawer *>::iterator iter = robotDrawers.begin(); iter != robotDrawers.end(); ++iter)
		(*iter)->update(timedelta);
	environmentDrawer->update(timedelta);
	
	cameraPitch += timedelta * cameraAngularSpeedPitch;
	cameraYaw += timedelta * cameraAngularSpeedYaw;
	
	// Recalculate camera
	
	// Move a camera object there:
	// glTranslatef(x, height, z);
	// glRotatef(yaw, 0, 1, 0);
	// glRotatef(pitch, 0, 0, 1);
	// glTranslatef(0, 0, offset);
	
	matrix positionMatrix = matrix::position(cameraPosition);
	matrix yawMatrix = matrix::rotation(float4(0, 1, 0, 0), cameraYaw);
	matrix pitchMatrix = matrix::rotation(float4(1, 0, 0, 0), cameraPitch);
	matrix offsetMatrix = matrix::position(float4(0.0f, 0.0f, cameraZoom));
	
	inverseModelMatrix = positionMatrix * yawMatrix * pitchMatrix * offsetMatrix;
	modelMatrix = inverseModelMatrix.inverse();
	
	cameraPosition -= yawMatrix.x * cameraSpeedX * timedelta;
	cameraPosition -= yawMatrix.y * cameraSpeedY * timedelta;
	cameraPosition -= yawMatrix.z * cameraSpeedZ * timedelta;
}

void Drawer::setCameraPosition(float x, float y, float z, float pitch, float yaw, float zoom)
{
	cameraPosition = float4(x, y, z);
	cameraPitch = pitch;
	cameraYaw = yaw;
	
	cameraSpeedX = 0.0f;
	cameraSpeedY = 0.0f;
	cameraSpeedZ = 0.0f;
	cameraAngularSpeedPitch = 0.0f;
	cameraAngularSpeedYaw = 0.0f;
	
	cameraZoom = zoom;
}

void Drawer::setCameraSpeedX(float speed)
{
	cameraSpeedX = speed;
}

void Drawer::setCameraSpeedY(float speed)
{
	cameraSpeedY = speed;
}

void Drawer::setCameraSpeedZ(float speed)
{
	cameraSpeedZ = speed;
}

void Drawer::setCameraAngularSpeedPitch(float value)
{
	cameraAngularSpeedPitch = value;
}

void Drawer::setCameraAngularSpeedYaw(float value)
{
	cameraAngularSpeedYaw = value;
}

void Drawer::rotateCamera(float radians)
{
	cameraYaw -= radians * float(M_PI) / 90.0f;
}
void Drawer::moveCamera(float x, float y)
{
	matrix yawMatrix = matrix::rotation(float4(0, 1, 0, 0), cameraYaw);
	cameraPosition -= yawMatrix.x * x;
	cameraPosition -= yawMatrix.z * y;	
}

float4 Drawer::getCameraVelocity() const throw()
{
	matrix yawMatrix = matrix::rotation(float4(0, 1, 0), cameraYaw);
	float4 cameraVelocity(0.0f, 0.0f, 0.0f, 0.0f);
	cameraVelocity -= yawMatrix.x * cameraSpeedX;
	cameraVelocity -= yawMatrix.y * cameraSpeedY;
	cameraVelocity -= yawMatrix.z * cameraSpeedZ;
	
	return cameraVelocity;
}

float Drawer::getScreenWidth() const throw()
{
	if (orientation == RotatedLeft || orientation == RotatedRight)
		return screenHeight;
	else
		return screenWidth;
}

float Drawer::getScreenHeight() const throw()
{
	if (orientation == RotatedLeft || orientation == RotatedRight)
		return screenWidth;
	else
		return screenHeight;
}

void Drawer::transformPoint(float inX, float inY, float &outX, float &outY)
{
	// Note that we transform always except on Mac OS X.
#if defined(__APPLE_CC__) && !defined(IPHONE) && !defined(IPHONE_SIMULATOR)
	outX = inX;
	outY = inY;
	return;
#endif
	switch(orientation)
	{
		case RotatedLeft:
			outX = screenHeight - inY;
			outY = screenWidth - inX;
			break;
		case RotatedRight:
			outX = inY;
			outY = inX;
			break;
		case UpsideDown:
			outX = screenWidth - inX;
			outY = inY;
			break;
		case Normal:
		default:
			outX = inX;
			outY = screenHeight - inY;
			break;
	}
}

ray4 Drawer::rayForCoords(float x, float y)
{	
	float relX = x / getScreenWidth();
	float relY = y / getScreenHeight();
	
	relX = (relX * 2.0f - 1.0f);
	relY = (relY * 2.0f - 1.0f);
		
	// Create vectors
	ray4 ray(float4(relX, relY, -1.0, 1.f), float4(relX, relY, 1.0, 1.f));
	
	// Inverse matrix: Reverse order
	matrix inverseMatrix = inverseModelMatrix*inverseFrustumMatrix;

	ray = inverseMatrix * ray;
	
	ray.start() /= ray.start().w;
	ray.end() /= ray.end().w;
	
	return ray;
}

Texture *Drawer::textureWithFilename(const std::string &filename)
{
	Texture *& texture = textures[filename];
	if (texture == NULL)
#ifdef ANDROID_NDK
		texture = new Texture(assetManager, filename.c_str());
#else
		texture = new Texture(filename.c_str());
#endif
	
	return texture;
}

Model *Drawer::modelWithFilename(const std::string &filename)
{
	Model *& model = models[filename];
	if (model == NULL)
		model = new Model(filename.c_str(), this);
	
	return model;
}

void Drawer::debugLineClear(const std::string &name)
{
	debugLines[name].clear();
}

void Drawer::debugLineAddSegment(const std::string &name, const float4 &a, const float4 &b)
{
	debugLines[name].push_back(a);
	debugLines[name].push_back(b);
}

void Drawer::clearDebugLine(const char *name)
{
	globalDebugDrawer->debugLineClear(std::string(name));
}

void Drawer::addDebugLineSegment(const char *name, const float4 &a, const float4 &b)
{
	globalDebugDrawer->debugLineAddSegment(std::string(name), a, b);
}
