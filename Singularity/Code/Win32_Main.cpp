/* ----==== WIN32_MAIN.CPP ====---- */

//--------------------------------------
//	SINGULARITY ENGINE v 0.2
//
//	By:			Jeff Kiah
//	Started:	3/27/02
//--------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <multimon.h>
#include "win32_main.h"
#include "options.h"
#include "engine.h"
#include "console.h"
#include "UTILITYCODE\glfont.h"
#include "UTILITYCODE\timer.h"

/*-----------------
---- VARIABLES ----
-----------------*/

HINSTANCE		hInst;
MSG				msg;
HWND			hWnd;
HDC				hDC;
HGLRC			hRC;

bool			active = true;
const char		appName[] = "Singularity Engine";


/*-----------------
---- FUNCTIONS ----
-----------------*/

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


void killWindow(void)
{
	if (gOptions.FULLSCREEN) {
		ChangeDisplaySettings(NULL,0);
		ShowCursor(true);
	}

	if (hRC) {
		if (!wglMakeCurrent(NULL,NULL))
			MessageBox(0,"Release of DC and RC failed","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);

		if (!wglDeleteContext(hRC))
			MessageBox(0,"Release rendering context failed","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);

		hRC = NULL;
	}

	if (hDC && !ReleaseDC(hWnd,hDC)) {
		MessageBox(0,"Release device context failed","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		hDC = NULL;
	}

	if (hWnd && !DestroyWindow(hWnd))
	{
		MessageBox(0,"Could not release hWnd","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		hWnd = NULL;
	}

	if (!UnregisterClass("WNDCLASS1",hInst)) {
		MessageBox(0,"Could not unregister class","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		hInst = NULL;
	}
}


bool initWindow(void)
{
	WNDCLASSEX	wc;
	DWORD		dwExStyle;
	DWORD		dwStyle;
	UINT		pixelFormat;
	
	hInst = GetModuleHandle(NULL);

    wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "WNDCLASS1";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(0,"Failed to register the window class","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	if (gOptions.FULLSCREEN) {
//		POINT point = {0,0};

//		MONITORINFO monitorinfo;
//		monitorinfo.cbSize = sizeof(MONITORINFO);
//		::GetMonitorInfo(::MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY), &monitorinfo);
//		EnumDisplaySettings(

		
		DEVMODE dmScreenSettings;

		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));		
		dmScreenSettings.dmSize	= sizeof(DEVMODE);
		dmScreenSettings.dmPelsWidth = gOptions.RESX;
		dmScreenSettings.dmPelsHeight = gOptions.RESY;
		dmScreenSettings.dmBitsPerPel = gOptions.COLORDEPTH;
		dmScreenSettings.dmDisplayFrequency = gOptions.REFRESH;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
		int check = ChangeDisplaySettings(&dmScreenSettings,CDS_TEST);
		
		switch (check) {
			case DISP_CHANGE_SUCCESSFUL:
				ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN);
				break;

			case DISP_CHANGE_FAILED:
				killWindow();
				MessageBox(0,"Failed to change to desired settings","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
				return false;

			case DISP_CHANGE_BADMODE:
				killWindow();
				MessageBox(0,"Fullscreen video mode not supported","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
				return false;

			case DISP_CHANGE_RESTART:
				killWindow();
				MessageBox(0,"Must restart to get fullscreen video mode","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
				return false;
		}
	}

	if (gOptions.FULLSCREEN) {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
		dwStyle = WS_POPUP;
	} else {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW;
		dwStyle = WS_POPUP | WS_BORDER;
	}

	hWnd = CreateWindowEx(dwExStyle,"WNDCLASS1",appName,dwStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,0,0,gOptions.RESX,gOptions.RESY,NULL,NULL,hInst,NULL);
	if (!hWnd) {
		killWindow();
		MessageBox(0,"Window creation error","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_LAYER_BUFFERS | PFD_SWAP_EXCHANGE,
		PFD_TYPE_RGBA,
		gOptions.COLORDEPTH,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		gOptions.ZDEPTH,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	hDC = GetDC(hWnd);
	if (!hDC) {
		killWindow();
		MessageBox(NULL,"Cannot create a device context","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	pixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (!pixelFormat) {
		killWindow();
		MessageBox(0,"Cannot find suitable pixel format","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	if (!SetPixelFormat(hDC, pixelFormat, &pfd)) {
		killWindow();
		MessageBox(0,"Cannot set the pixel format","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	hRC = wglCreateContext(hDC);
	if (!hRC) {
		killWindow();
		MessageBox(0,"Cannot create an OpenGL rendering context","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	if (!wglMakeCurrent(hDC, hRC)) {
		killWindow();
		MessageBox(0,"Cannot activate OpenGL rendering context","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
	wglMakeCurrent(hDC, hRC);

	// Add display info to console
	console.addLine("RESX: %i   RESY: %i   COLORDEPTH: %i   REFRESH: %i",gOptions.RESX,gOptions.RESY,gOptions.COLORDEPTH,gOptions.REFRESH);

	return true;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_CREATE:
			break;

		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_DESTROY:
			break;

		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE || HIWORD(wParam)) {
				active = false;
				ShowCursor(true);				
			} else if (!active && !HIWORD(wParam)) {
				active = true;
				ShowCursor(false);
				SetCursorPos(gOptions.RESX / 2, gOptions.RESY / 2);
				timer.resetLastTime();
			}
			break;

		case WM_SYSCOMMAND:
			switch (wParam) {
				case SC_MOVE:
				case SC_SIZE:
				case SC_MAXIMIZE:
				case SC_KEYMENU:				
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					break;
			}
			break;
	
		case WM_NCPAINT:
		case WM_ERASEBKGND:
		case WM_SIZE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			break;

		default:
			return DefWindowProc(hwnd,msg,wParam,lParam);
	}

	return 0;
}


int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int iCmdShow)
{
	srand(0);

	Timer *timerInst = new Timer;
	timer.initTimer();

	Engine *engineInst = new Engine;

	if (!initWindow()) return 0;

	engine.initOpenGL();
	engine.setUpGame();

	for (;;) {
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);

		} else {
			if (active) {			
				timer.calcTimeFix();
				engine.playGame();
				timer.calcFPS();
				Sleep(0);
				
			} else {
				WaitMessage();	// avoid 100% CPU when inactive
			}

			if (engine.exitEngine) SendMessage(hWnd,WM_CLOSE,0,0);
		}
	}
	
	killWindow();
	
	delete engineInst;
	delete timerInst;

	_CrtCheckMemory();

	return msg.wParam;
}

////////////////////


/*void InitFMOD(void)
{
	FSOUND_SetMaxHardwareChannels(32);
	FSOUND_Init(44100,64,0);
	Sound[0] = FSOUND_Sample_Load(FSOUND_FREE,"data/sound/jump.wav",FSOUND_8BITS | FSOUND_MONO | FSOUND_LOOP_OFF,0);
	Sound[1] = FSOUND_Sample_Load(FSOUND_FREE,"data/sound/chaingun_once.wav",FSOUND_8BITS | FSOUND_MONO | FSOUND_LOOP_OFF,0);
//	Music = FSOUND_Stream_OpenFile("data/sound/m001.mp3",FSOUND_LOOP_NORMAL | FSOUND_HW3D,0);
	CurrentSong = 0;
	Music[0] = FMUSIC_LoadSong("data/music/track_01.it");
	Music[1] = FMUSIC_LoadSong("data/music/track_02.it");
	Music[2] = FMUSIC_LoadSong("data/music/track_03.it");
	Music[3] = FMUSIC_LoadSong("data/music/track_04.it");
	FMUSIC_SetMasterVolume(Music[0],128);
	FMUSIC_SetMasterVolume(Music[1],128);
	FMUSIC_SetMasterVolume(Music[2],128);
	FMUSIC_SetMasterVolume(Music[3],128);
}


void DisableFMOD(void)
{
	FSOUND_Close();
}


void HandleMusic(void)
{
	if (GetAsyncKeyState('M') & 0x8000) {
		if (!ButtonDown['M']) {
			ButtonDown['M'] = true;
//			FSOUND_Stream_Play(FSOUND_FREE,Music);
			FMUSIC_StopSong(Music[CurrentSong]);
			CurrentSong++;
			if (CurrentSong > 3) CurrentSong = 0;
			FMUSIC_PlaySong(Music[CurrentSong]);
		}
	} else ButtonDown['M'] = false;

	if (GetAsyncKeyState('N') & 0x8000) {
		if (!ButtonDown['N']) {
			ButtonDown['N'] = true;
//			FSOUND_Stream_Stop(Music);
			FMUSIC_StopSong(Music[CurrentSong]);
			CurrentSong--;
			if (CurrentSong < 0) CurrentSong = 3;
			FMUSIC_PlaySong(Music[CurrentSong]);
		}
	} else ButtonDown['N'] = false;

	if (GetAsyncKeyState('B') & 0x8000) {
		if (!ButtonDown['B']) {
			ButtonDown['B'] = true;
			if (FMUSIC_IsPlaying(Music[CurrentSong])) FMUSIC_StopSong(Music[CurrentSong]);
			else FMUSIC_PlaySong(Music[CurrentSong]);
		}
	} else ButtonDown['B'] = false;

	if (FMUSIC_IsFinished(Music[CurrentSong])) {
			FMUSIC_StopSong(Music[CurrentSong]);
			CurrentSong++;
			if (CurrentSong > 3) CurrentSong = 0;
			FMUSIC_PlaySong(Music[CurrentSong]);
	}
}*/
