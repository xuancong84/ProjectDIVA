#pragma once

#include "dinput.h"
#include "resource.h"
#include "defines.h"

class DInputKeyboard{
	
	LONGLONG freq, offset;
	DWORD initTickCount;
	LONGLONG initPerfCount;
	LPDIRECTINPUT8 m_lpDI;
	LPDIRECTINPUTDEVICE8 m_lpDIDevice;
	HINSTANCE m_hInst;
	HWND m_hWnd;
	HANDLE m_hThread;
	DWORD m_threadID;
	HANDLE m_hEvent;
	bool m_Start, m_Stop;

	BYTE scancode_virtual_map[256];

public:
	int status;

	DInputKeyboard();
	~DInputKeyboard();

	DWORD DInputKeyboardThreadFunc();

	HRESULT create(HINSTANCE hInst, HWND hwnd);
	void release();
	void start();
	void stop();
};

extern DInputKeyboard dInputKeyboard;

