#include "dinput.h"
#include "keyboard.h"
#include "UIScreen.h"
#include "GameMana.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

DInputKeyboard::DInputKeyboard(){
	memset(this, 0, sizeof(DInputKeyboard));
	status = -1;
	for(int x=0; x<256; x++)
		scancode_virtual_map[x] = MapVirtualKey(x,3);
	// Unmapped keys
	scancode_virtual_map[0xd2] = VK_INSERT;		//insert
	scancode_virtual_map[0xd3] = VK_DELETE;		//delete
	scancode_virtual_map[0xc7] = VK_HOME;		//home
	scancode_virtual_map[0xcf] = VK_END;		//end
	scancode_virtual_map[0xc9] = VK_NEXT;		//page up
	scancode_virtual_map[0xd1] = VK_PRIOR;		//page down
	scancode_virtual_map[0xc8] = VK_UP;			//up
	scancode_virtual_map[0xd0] = VK_DOWN;		//down
	scancode_virtual_map[0xcb] = VK_LEFT;		//left
	scancode_virtual_map[0xcd] = VK_RIGHT;		//right
	scancode_virtual_map[0x9c] = VK_RETURN;		//numpad enter
	scancode_virtual_map[0xb5] = VK_DIVIDE;		//numpad /
}

HRESULT DInputKeyboard::create(HINSTANCE hInst, HWND hwnd){
	m_hInst = hInst;
	m_hWnd = hwnd;

	// Calibrate timer
	LARGE_INTEGER qwTime;
	if (QueryPerformanceFrequency(&qwTime) == FALSE)
		return -1;
	freq = qwTime.QuadPart;
	LARGE_INTEGER tm;
	QueryPerformanceCounter(&tm);
	initTickCount = GetTickCount();
	initPerfCount = tm.QuadPart;
	offset = initPerfCount-initTickCount/1000.0*freq;

	HRESULT hr;

	// Create DInput object
	hr = DirectInput8Create(m_hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_lpDI, NULL); 
	if FAILED(hr) 
		return 1;

	// Create DInput Device
	hr = m_lpDI->CreateDevice(GUID_SysKeyboard, &m_lpDIDevice, NULL); 
	if FAILED(hr)
		return 2;

	// Set Data format
	hr = m_lpDIDevice->SetDataFormat(&c_dfDIKeyboard);
	if FAILED(hr)
		return 3;

	// Set buffer size
	DIPROPDWORD c_diprop = {{sizeof(DIPROPDWORD),sizeof(DIPROPHEADER),0,DIPH_DEVICE}, 16};
	hr = m_lpDIDevice->SetProperty(DIPROP_BUFFERSIZE, &c_diprop.diph);
	if FAILED(hr)
		return 4;

	// Set cooperative level
	//hr = m_lpDIDevice->SetCooperativeLevel(m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	hr = m_lpDIDevice->SetCooperativeLevel(m_hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE); 
	if FAILED(hr)
		return 5;

	// Create event
	m_hEvent = CreateEvent(NULL,false,false,NULL);
	if(!m_hEvent)
		return 6;

	status = 0;
	if(m_Start){
		start();
		m_Start = false;
	}

	return NOERROR;
}

DWORD DInputKeyboard::DInputKeyboardThreadFunc(){
	status = 1;
	DIDEVICEOBJECTDATA buf[16];
	while(true){
		WaitForSingleObject(m_hEvent, INFINITE);
		DWORD n = 16;
		if(!m_lpDIDevice)
			break;
		HRESULT hr = m_lpDIDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), buf, &n, 0);
		if(hr != DI_OK){	// device lost
			if(!m_lpDIDevice || m_Stop)
				goto exit;
			while(m_lpDIDevice->Acquire()!=DI_OK){
				Sleep(100);
				if(!m_lpDIDevice || m_Stop)
					goto exit;
			}
			continue;
		}
		LONGLONG timeStamp = GetCurrentCount();
		if(GetForegroundWindow()==m_hWnd){
			for(int x=0; x<n; x++){
				DWORD vkCode = scancode_virtual_map[buf[x].dwOfs & 0xff];
				bool bKeyDown = ((buf[x].dwData&0x80) !=0);

				if( GAMEDATA.gamemode==GAME_MODE_NORMAL && ScreenState==SCREEN_GAMEMAIN && !core.paused )
					core._notemana.PushKeyEvent(vkCode, bKeyDown, timeStamp);

				/*// debug scan code
				static int kk=0;
				char str[256];
				sprintf(str,"DInput %d %08x->%08x %c",++kk,buf[x].dwOfs,vkCode,bKeyDown?'D':'U');
				SetWindowTitle(m_hWnd,str);*/
			}
		}
		if(m_Stop){
			m_Stop = false;
			break;
		}
	}
exit:
	m_hThread = NULL;
	m_threadID = 0;
	status = 0;
	return 0;
}

DWORD WINAPI _DInputKeyboardThreadFunc(void *data){
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
	return ((DInputKeyboard*)data)->DInputKeyboardThreadFunc();
}

void DInputKeyboard::start(){
	HRESULT hr;

	if (!m_lpDIDevice){
		m_Start = true;
		return;
	}

	hr = m_lpDIDevice->SetEventNotification(m_hEvent);
	if(hr!=NOERROR)
		return;

	hr = m_lpDIDevice->Acquire();
	if(hr!=NOERROR)
		return;

	m_hThread = CreateThread(NULL,0,&_DInputKeyboardThreadFunc,this,0,&m_threadID);
}

void DInputKeyboard::stop(){
	if(m_hThread){
		m_Stop = true;
		PulseEvent(m_hEvent);
	}
	if(m_lpDIDevice) m_lpDIDevice->Unacquire();
	m_lpDIDevice->SetEventNotification(NULL);
}

void DInputKeyboard::release(){
	stop();
	SAFE_RELEASE(m_lpDIDevice);
	SAFE_RELEASE(m_lpDI);
	if(m_hEvent){
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
	status = -1;
}

DInputKeyboard::~DInputKeyboard(){
	release();
}

DInputKeyboard dInputKeyboard;
