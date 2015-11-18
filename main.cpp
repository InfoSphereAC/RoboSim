#ifdef __APPLE_CC__
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h> 
#endif

#include <iostream>

#include "Controller.h"
#include "ShowMessageBoxAndExit.h"

namespace
{	
	Controller *controller;
	
	bool commandKeyIsDown = false;
	
	// change sreen size
	const unsigned defaultWindowWidth =  800;
	const unsigned defaultWindowHeight = 600;
	
	unsigned actualWindowWidth = defaultWindowWidth;
	unsigned actualWindowHeight = defaultWindowHeight;
}

void initVideo(const char* filename)
{
#ifdef _WIN32
	HINSTANCE handle = ::GetModuleHandle(NULL);
	HICON icon = ::LoadIcon(handle, MAKEINTRESOURCE(1));

	SDL_SysWMinfo wminfo;
	SDL_VERSION(&wminfo.version);
	SDL_GetWMInfo(&wminfo);
	HWND hwnd = wminfo.window;
	SetClassLong(hwnd, GCL_HICON, (LONG) icon);

#endif
	// Set Window half of current screen width 
	const SDL_VideoInfo* info = SDL_GetVideoInfo();
	actualWindowWidth = info->current_w / 2;
	actualWindowHeight = info->current_h / 4 * 3;

	// Set window position
	SDL_putenv("SDL_VIDEO_WINDOW_POS=center");

	if (filename){
		std::string c = std::string("Simulator - ") + std::string(filename);
		const char * window_name = c.c_str();
		SDL_WM_SetCaption(window_name, NULL);
	}
	else SDL_WM_SetCaption("Simulator", NULL);
	
	if (SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16) != 0)
	{
		ShowErrorAndExit(L"Konnte Grafik nicht richtig einstellen (kein Tiefenbuffer verf\u00FCgbar). Fehlerbeschreibung: \"%s\"", L"Could not configure graphics (no depth buffer available). Error description: \"%s\"", SDL_GetError());
	}
	if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) != 0)
	{
		ShowErrorAndExit(L"Konnte Grafik nicht richtig einstellen (kein Double Buffering verf\u00FCgbar). Fehlerbeschreibung: \"%s\"", L"Could not configure graphics (no double buffering available). Error description: \"%s\"", SDL_GetError());
	}
	if (SDL_SetVideoMode(actualWindowWidth, actualWindowHeight, 0, SDL_OPENGL | SDL_RESIZABLE) == NULL)
	{
		ShowErrorAndExit(L"Konnte Grafik nicht starten (Kein Video Mode). Fehlerbeschreibung: \"%s\"", L"Could not start graphics (no video mode). Error description: \"%s\"", SDL_GetError());
	}
	
	controller->initVideo();
	controller->setWindowSize(float(actualWindowWidth), float(actualWindowHeight));
	controller->draw();
	SDL_GL_SwapBuffers();
}

void initAudio()
{
	controller->initAudio();
}

void postQuitEvent()
{
	SDL_Event quitEvent;
	quitEvent.type = SDL_QUIT;
	SDL_PushEvent(&quitEvent);
}

int keyForSDLKey(SDLKey key)
{	
	switch(key)
	{
		case SDLK_UP: return Controller::UpArrow;
		case SDLK_DOWN: return Controller::DownArrow;
		case SDLK_RIGHT: return Controller::RightArrow;
		case SDLK_LEFT: return Controller::LeftArrow;
		default: return -1;
	}
	
}

void keyDown(const SDL_Event &event)
{
	bool isPrintable = (event.key.keysym.sym >= SDLK_SPACE && event.key.keysym.sym <= SDLK_z);
	if (isPrintable)
		controller->keyDown(event.key.keysym.sym, true);
	else
		controller->keyDown(keyForSDLKey(event.key.keysym.sym), false);
	
	if (event.key.keysym.sym == SDLK_LMETA) commandKeyIsDown = true;
}

void keyUp(const SDL_Event &event)
{
	bool isPrintable = (event.key.keysym.sym >= SDLK_SPACE && event.key.keysym.sym <= SDLK_z);
	if (isPrintable)
		controller->keyUp(event.key.keysym.sym, true);
	else
		controller->keyUp(keyForSDLKey(event.key.keysym.sym), false);
	
	if (event.key.keysym.sym == SDLK_LMETA) commandKeyIsDown = false;
	if (event.key.keysym.sym == SDLK_q && commandKeyIsDown) postQuitEvent();
}


void runLoop()
{
	unsigned oldTime = SDL_GetTicks();
	while(true)
	{
		try
		{
			long newTime = SDL_GetTicks();
			long diff = newTime - oldTime;
			oldTime = newTime;
			
			float delta = float(diff)/1000.0f;
			
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				switch(event.type)
				{
					case SDL_KEYDOWN:
						keyDown(event);
						break;
					case SDL_KEYUP:
						keyUp(event);
						break;
					case SDL_MOUSEBUTTONDOWN:
						if (event.button.button == SDL_BUTTON_WHEELDOWN)
							controller->scroll(-1);
						else if (event.button.button == SDL_BUTTON_WHEELUP)
							controller->scroll(1);
						else
							controller->mouseDown(float(event.button.x), float(event.button.y), event.button.button);
						break;
					case SDL_MOUSEBUTTONUP:
						if (event.button.button != SDL_BUTTON_WHEELDOWN && event.button.button != SDL_BUTTON_WHEELUP)
							controller->mouseUp(float(event.button.x), float(event.button.y), event.button.button);
						break;
					case SDL_MOUSEMOTION:
						controller->mouseMove(float(event.motion.x), float(event.motion.y));
						break;
					case SDL_VIDEORESIZE:
						controller->setWindowSize(float(event.resize.w), float(event.resize.h));
						SDL_SetVideoMode(event.resize.w, event.resize.h, 0, SDL_RESIZABLE | SDL_OPENGL);
						break;
						
					case SDL_QUIT:
						controller->shutDown();
						return;
				}
			}
			controller->update(delta);
			controller->draw();
			
			float targetDelta = 1.0f/60.0f;
			float delay = targetDelta - delta;
			if (delay > 0) SDL_Delay((unsigned) ( delay * 1000.0f) );
			
			SDL_GL_SwapBuffers();
		}
		catch (std::exception &e)
		{
			ShowErrorAndExit(L"Ein Fehler ist aufgetreten: \"%s\"", L"An error occured: \"%s\"", e.what());
		}
	}
}

extern "C" int main (int argc, char * argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		ShowErrorAndExit(L"Konnte SDL nicht starten. Fehlerbeschreibung: \"%s\"", L"Could not start SDL, error description: \"%s\"", SDL_GetError());
	}

	// Parse arguments

	const char *filename = NULL;
	char server[255];
	server[0] = 0;
	char port[255]; // Yes, as string, because that’s what parsing functions later use.
	port[0] = 0;
	bool robotUIOnServer = false;
	bool noAutoDiscovery = false;
	
	Controller::NetworkMode mode = Controller::LetUserChoose;
	unsigned flags = 0;
		
	for (int i = 1; i < argc; i++)
	{
		unsigned width, height;
		char serverTempString[255];
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0)
		{
			printf("Usage: Normally from the GUI, but if not\n\t%s [args] [filename]\nOptional arguments:", argv[0]);
			printf("\n\t-w=,--width=\tWindow width (default 800)");
			printf("\n\t-h=,--height=\tWindow height (default 600)");
			printf("\n\t-s=,--server=\tServer to connect to (bypasses selection screen)");
			printf("\n\t--asServer Run as server");
			printf("\n\t-p=,--port=\tPort (default 10412) to connect to/of server");
			printf("\n\t--robotUIOnServer Only valid for clients: Stuff like picking up and IO configuration is handled by server.\nOnly one such robot is allowed per server, and only if the server does not have a robot of its own.");	
			printf("\n\t--noAutodiscovery Only valid for server: Do not respond to autodiscovery messages. This means clients have to know the port and IP address of a server to connect to it.");
			printf("\n\t--\tStop scanning for arguments.");
		}
		else if (sscanf(argv[i], "--width=%u", &width) == 1 || sscanf(argv[i], "-w=%u", &width) == 1)
			actualWindowWidth = width;
		else if (sscanf(argv[i], "--height=%u", &height) == 1 || sscanf(argv[i], "-h=%u", &height) == 1)
			actualWindowHeight = height;
		else if (sscanf(argv[i], "--server=%255s", serverTempString) == 1 || sscanf(argv[i], "-s=%255s", serverTempString) == 1)
		{
			memcpy(server, serverTempString, 255);
			mode = Controller::ClientMode;
		}
		else if (sscanf(argv[i], "--port=%255s", serverTempString) == 1 || sscanf(argv[i], "-p=%255s", serverTempString) == 1)
			memcpy(port, serverTempString, 255);
		else if (strcmp(argv[i], "--asServer") == 0)
			mode = Controller::ServerMode;
		else if (strcmp(argv[i], "--noAutodiscovery") == 0)
			noAutoDiscovery = true;
		else if (strcmp(argv[i], "--robotUIOnServer") == 0)
			robotUIOnServer = true;
		else if (strcmp(argv[i], "--") == 0)
		{
			if (argc > (i+1)) filename = argv[i+1];
			break;
		}
		else
			filename = argv[i];
	}
	
	server[254] = 0;
	port[254] = 0;
	
	if (mode == Controller::ClientMode && robotUIOnServer)
		flags |= Controller::ClientFlagUIOnServer;
	else if (mode == Controller::ServerMode && noAutoDiscovery)
		flags |= Controller::ServerFlagNoBroadcast;
	
	controller = new Controller(mode, flags, filename, server[0] ? server : NULL, port[0] ? port : NULL);

#ifdef _WIN32
	// Get own path
	wchar_t exeFileName[MAX_PATH];
	GetModuleFileName(NULL, exeFileName, MAX_PATH);

	// Register file type (if it does not yet exist)
	const char programID[] = "Legosimulator.RXE.1";
	const char fileTypeDescription[] = "Lego Mindstorms RXE file";
	HKEY hRXEKey;
	LONG error;
	DWORD disposition;
	error = RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Classes\\.rxe", 0, NULL, 0, KEY_WRITE, NULL, &hRXEKey, &disposition);
	if (error == ERROR_SUCCESS && disposition == REG_CREATED_NEW_KEY)
		error = RegSetValueExA(hRXEKey, NULL, 0, REG_SZ, (LPBYTE) programID, (DWORD) strlen(programID) + 1);
	RegFlushKey(hRXEKey);
	RegCloseKey(hRXEKey);
	
	// Register file type description (if it does not yet exist)
	HKEY hProgramKey;
	error = RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Classes\\Legosimulator.RXE.1", 0, NULL, 0, KEY_WRITE, NULL, &hProgramKey, NULL);
	if (error == ERROR_SUCCESS && disposition == REG_CREATED_NEW_KEY)
		error = RegSetValueExA(hProgramKey, NULL, 0, REG_SZ, (LPBYTE) fileTypeDescription, (DWORD) strlen(fileTypeDescription) + 1);
	if (error == ERROR_SUCCESS && disposition == REG_CREATED_NEW_KEY)
		error = RegSetValueExA(hProgramKey, (LPCSTR) "FriendlyTypeName", 0, REG_SZ, (LPBYTE) fileTypeDescription, (DWORD) strlen(fileTypeDescription) + 1);
	RegFlushKey(hProgramKey);
	
	// Set path to open it (always, in case this program gets moved)
	wchar_t openCommand[MAX_PATH + 20];
	wsprintf(openCommand, L"\"%s\" \"%%1\"", exeFileName);
	HKEY hOpenKey;
	error = RegCreateKeyExA(hProgramKey, "shell\\open\\command", 0, NULL, 0, KEY_WRITE, NULL, &hOpenKey, NULL);
	if (error == ERROR_SUCCESS)
		error = RegSetValueExW(hOpenKey, NULL, 0, REG_SZ, (LPBYTE) openCommand, (DWORD) ((wcslen(openCommand) + 1) * sizeof(wchar_t)));
	RegFlushKey(hOpenKey);
	RegCloseKey(hOpenKey);
	RegCloseKey(hProgramKey);
	
	// Setup correct working directory
	wchar_t *lastSlash = wcsrchr(exeFileName, L'\\');
	wcscpy(lastSlash, L"\\ressources");

	SetCurrentDirectory(exeFileName);
#endif

	initVideo(filename);

	initAudio();
	runLoop();
	
	SDL_Quit();
	
	return 0;
}
