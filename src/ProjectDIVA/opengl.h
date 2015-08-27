/*
	Opengl.h
	OpenGL Graphics Engine Header file
Note:
	This API assumes that the user follows the convension that by default, the currently
	selected matrix is the modelview matrix. Unless otherwise specified, all functions
	do not preserve modelview matrix in order to boost performance. Users are liable to
	preserve matrix data prior to calling API functions if neccessary. Creating multiple
	GL windows is supported but not recommended, users are encouraged to implement their
	own windows callback functions in that case.

	By Wang Xuancong (NUS)
*/
#ifndef __OPENGL_H
#define __OPENGL_H

#include "stdio.h"
#include "math.h"
#include <windows.h>
#include "gl\gl.h"

#define	MAXGLWINDOWS	16
#define	FONTSTART		0
#define	FONTCOUNT		128

#ifndef MAXWMHANDLERS
#define	MAXWMHANDLERS	64
#endif

#ifndef	PI
#define	PI				3.1415926535897932384626
#endif

#define	DEFAULT_TIMER_ID	 1000

#define GL_ARRAY_BUFFER_ARB 		0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB	0x8893
#define GL_STATIC_DRAW_ARB			0x88E4

int		isVBOsupported;
void	APIENTRY glNullFunc(){}
void	(APIENTRY*glGenBuffers)(int,DWORD*)					=	(void(APIENTRY*)(int,DWORD*)) glNullFunc;
void	(APIENTRY*glBindBuffer)(DWORD,DWORD)				=	(void(APIENTRY*)(DWORD,DWORD)) glNullFunc;
void	(APIENTRY*glBufferData)(DWORD,int,void*,DWORD)		=	(void(APIENTRY*)(DWORD,int,void*,DWORD)) glNullFunc;
void	(APIENTRY*glDeleteBuffers)(int,DWORD*)				=	(void(APIENTRY*)(int,DWORD*)) glNullFunc;
void	(APIENTRY*glMultiDrawArrays)(GLenum,int*,int*,int)	=	(void(APIENTRY*)(GLenum,int*,int*,int)) glNullFunc;
void	(APIENTRY*glPointParameterf)(GLenum, GLfloat)		=	(void(APIENTRY*)(GLenum,GLfloat)) glNullFunc;
void	(APIENTRY*glPointParameterfv)(GLenum, const GLfloat*) =	(void(APIENTRY*)(GLenum,const GLfloat*)) glNullFunc;
void	(APIENTRY*glMultiDrawElements)( GLenum mode,int *count,GLenum type,const void **indices,int primcount )
		=	(void(APIENTRY*)(GLenum,int*,GLenum,const void **,int)) glNullFunc;

class		GLwindow;
LRESULT		CALLBACK _wndProc(HWND, UINT, WPARAM, LPARAM);
void		CALLBACK _timerProc(HWND,UINT,UINT,DWORD);
bool		_DrawGLScene(GLwindow*);
bool		RunMsgLoop(DWORD = 0);

GLwindow	*All_GLWindows[MAXGLWINDOWS];
int			N_GLWindows = 0;
char		_defaultWindowName[]="OpenGL Window";

class GLwindow{
public:
	class Font2D{
		public:
		int 	base;
		HFONT   hFont;
		Font2D(int,HFONT);
		~Font2D();
	};
	class Font3D{
		public:
		int     base;
		HFONT   hFont;
		float   thickness;
		GLYPHMETRICSFLOAT   gmf[FONTCOUNT];
		Font3D(int,HFONT);
		~Font3D();
	};
	struct WMHandle{
		UINT	msg;
		WNDPROC	handleProc;
	}*pWMHandleList;
	int			N_WMHandles;

	char		className[16];
	char		*windowName;
	bool		fullScreen;
	bool		isActive;
	bool		bUse3D;
	bool		bExtDC;
	int			cbStack;
	WNDCLASSEX	wcex;
	DEVMODE		dmScreenSettings;
	HWND		hWindow,hParentWindow;
	HGLRC		hRC;
	HDC			hDC;
	int			Left,Top,Width,Height;
	int			BitsPerPixel;
	HMENU		ChildWindowID;			//Use wcex.lpszMenuName for menu
	DWORD		dwExStyle,dwStyle;
	DWORD		ticksPerFrame;
	DWORD		userData;
	int			fontBase;				//Display list offset of current font
	double		ddFOVY;					//Field of view angle along X-Z plane in degrees
	double		ddClipNear,ddClipFar;	//The near and far clipping plane
	PIXELFORMATDESCRIPTOR 	pfd;
	WNDPROC		oldWndProc;

	DWORD		CreateGLWindow( bool=false );
	DWORD		CreateGLWindow( HWND, bool=false );	//Create OpenGL context from HWND
	DWORD		CreateGLWindow( HDC );				//Create OpenGL context from HDC
	DWORD		DestroyGLWindow();
	bool		SetFont(int,int,int,bool,bool,bool,char* =NULL);
	HFONT       SetFont(HFONT);
	Font2D*	    BuildFont2D(int,int,int,bool,bool,bool,char* =NULL);
	Font2D*	    BuildFont2D(HFONT);
	Font3D*	    BuildFont3D(HFONT,float,int=WGL_FONT_POLYGONS);
	int			GetExtFunc();
	bool        DestroyFont(int);
	bool		SetFPS(int FPS = 0);
	void		ResizeGLScene(int,int);
	void		glPrintf(char*,...);
	void		glPrintf(Font3D*,char*,...);
	void        glPrintf(Font2D*,char*,...);
	void        glXYPrintf(int,int,WORD,char*,...);
	void        glXYPrintf(Font2D*,int,int,WORD,char*,...);
	void        glXYZPrintf(Font3D*,int,int,int,DWORD,char*,...);
	bool		(*DrawGLScene)(GLwindow*);

	WNDPROC		RegisterWMHandler(UINT,WNDPROC);
	WNDPROC		inline	UnregisterWMHandler(UINT msg){return RegisterWMHandler(msg,NULL);}
	LRESULT	CALLBACK	wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	GLwindow();
	~GLwindow();
};

GLwindow::Font2D::Font2D(int base, HFONT hFont){
	this->base  =   base;
	this->hFont =   hFont;
}
GLwindow::Font2D::~Font2D(){
	if(base)	glDeleteLists(base, FONTCOUNT);
	if(hFont)   DeleteObject(hFont);
}
GLwindow::Font3D::Font3D(int base, HFONT hFont){
	this->base  =   base;
	this->hFont =   hFont;
}
GLwindow::Font3D::~Font3D(){
	if(base)	glDeleteLists(base, FONTCOUNT);
	if(hFont)   DeleteObject(hFont);
}

GLwindow::GLwindow(){
	memset(this, 0, sizeof(GLwindow));

//Set default parameteric member variables
	HDC		hdc		=	GetDC(GetDesktopWindow());
	BitsPerPixel	=	GetDeviceCaps(hdc,BITSPIXEL);
	ReleaseDC(GetDesktopWindow(),hdc);
	windowName		=	_defaultWindowName;
	Left			=	Top		=	CW_USEDEFAULT;
	dwExStyle		=	dwStyle	=	0xffffffff;
	ddFOVY			=	45.0f;
	ddClipNear		=	0.125f;
	ddClipFar		=	1000.f;
//Internal variables
	DrawGLScene		=	_DrawGLScene;

//Set default WNDCLASSEX structure
	wcex.hInstance	=	GetModuleHandle(NULL);
	wcex.cbSize		=	sizeof(WNDCLASSEX);
	wcex.hCursor	=	NULL;
	wcex.hIcon		=	LoadIcon(NULL,IDI_APPLICATION);
	wcex.style		=	CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc=	_wndProc;

//Set default PIXELFORMATDESCRIPTOR structure
	pfd.nSize   	=	sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion    =   1;
	pfd.dwFlags     =	PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL |PFD_DOUBLEBUFFER;
	pfd.iPixelType  =	PFD_TYPE_RGBA;
	pfd.cColorBits  =   BitsPerPixel;
	pfd.cDepthBits  =   32;
	pfd.cStencilBits=   16;
	pfd.iLayerType  =	PFD_MAIN_PLANE;

//Set default DEVMODE structure
	dmScreenSettings.dmSize			=	sizeof(dmScreenSettings);
	dmScreenSettings.dmPelsWidth	=	GetSystemMetrics(SM_CXSCREEN);
	dmScreenSettings.dmPelsHeight	=	GetSystemMetrics(SM_CYSCREEN);
	dmScreenSettings.dmBitsPerPel	=	BitsPerPixel;
	dmScreenSettings.dmFields		=	DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
}//Main class constructor

GLwindow::~GLwindow(){
	if(fontBase && !N_GLWindows)glDeleteLists(fontBase,FONTCOUNT);
	if(hWindow) DestroyGLWindow();
	if(oldWndProc){
		SetWindowLong(hWindow, GWL_WNDPROC, (DWORD)oldWndProc);
	}
}//Main class destructor

int		GLwindow::GetExtFunc(){
	if(!isVBOsupported)
		if(strstr((char*)glGetString(GL_EXTENSIONS),"GL_ARB_vertex_buffer_object")){
			glGenBuffers	=	(void(APIENTRY*)(int,DWORD*))wglGetProcAddress("glGenBuffers");
			glBindBuffer	=	(void(APIENTRY*)(DWORD,DWORD))wglGetProcAddress("glBindBuffer");
			glBufferData	=	(void(APIENTRY*)(DWORD,int,void*,DWORD))wglGetProcAddress("glBufferData");
			glDeleteBuffers	=	(void(APIENTRY*)(int,DWORD*))wglGetProcAddress("glDeleteBuffers");
			glPointParameterf =	(void(APIENTRY*)(GLenum, GLfloat))wglGetProcAddress("glPointParameterf");
			glPointParameterfv= (void(APIENTRY*)(GLenum, const GLfloat *))wglGetProcAddress("glPointParameterfv");
			glMultiDrawArrays	= (void(APIENTRY*)(GLenum,int*,int*,int))wglGetProcAddress("glMultiDrawArrays");
			glMultiDrawElements	= (void(APIENTRY*)(GLenum,int *,GLenum,const void **,int))wglGetProcAddress("glMultiDrawElements");
			isVBOsupported	=	(glGenBuffers && glBindBuffer && glBufferData && glDeleteBuffers);
		}
	return	isVBOsupported;
}
bool	RunMsgLoop(DWORD N_cycles){
	MSG			msg;
	GLwindow	*pWin;
	DWORD		x = 0;
	bool		doWait;

	while(1){
		if (N_cycles)if(++x > N_cycles)return true;

		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
			if(msg.message == WM_QUIT){
				if(!N_GLWindows) break;
			}else{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}else{//Draw all active GL windows
			doWait	=	true;
			for(int y=0; y<N_GLWindows; y++){
				pWin = All_GLWindows[y];
				if(pWin->isActive==true && (!pWin->ticksPerFrame)){
                    wglMakeCurrent(pWin->hDC,pWin->hRC);
					if(pWin->DrawGLScene(pWin))SwapBuffers(pWin->hDC);
					else    _DrawGLScene(pWin);
					doWait	=	false;
				}
			}
			if(doWait)WaitMessage();
		}
	}

	return	false;
}//Message loop for all windows

WNDPROC	GLwindow::RegisterWMHandler(UINT msg, WNDPROC handleProc){
	int		x;
	WNDPROC	oldHandleProc;
	if(!N_WMHandles || !pWMHandleList){
		pWMHandleList	=	new WMHandle [MAXWMHANDLERS];
		N_WMHandles		=	0;
	}else for(x=0; x<N_WMHandles; x++){
		if(pWMHandleList[x].msg == msg){
			oldHandleProc				=	pWMHandleList[x].handleProc;
			pWMHandleList[x].handleProc	=	handleProc;
			return oldHandleProc;
		}
	}
	if(N_WMHandles < MAXWMHANDLERS){
		pWMHandleList[N_WMHandles].msg			=	msg;
		pWMHandleList[N_WMHandles++].handleProc	=	handleProc;
	}
	return	NULL;
}//Register a window message handle function for a specified MSG

LRESULT CALLBACK _wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	for( int x=0; x<N_GLWindows; x++ ) if( All_GLWindows[x]->hWindow==hWnd )
		return All_GLWindows[x]->wndProc(hWnd, uMsg, wParam, lParam);
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}//OOP connector for window procedure

LRESULT CALLBACK GLwindow::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	LONG	retval = 0;
	if(N_WMHandles)for(int x=0; x<N_WMHandles; x++)
		if(pWMHandleList[x].msg == uMsg){
			return	pWMHandleList[x].handleProc(hWnd,uMsg,wParam,lParam);
		}
	if(oldWndProc){
		//if( cbStack<4 ){
		//	cbStack ++;
			retval = CallWindowProc( oldWndProc,hWnd,uMsg,wParam,lParam );
		//	cbStack --;
		//}
	}
	switch(uMsg){
		case	WM_ACTIVATE:
			isActive = (HIWORD(wParam)==0);
			break;
		case	WM_CLOSE:
			PostQuitMessage(0);
			break;
		case	WM_SIZE:
			ResizeGLScene(LOWORD(lParam),HIWORD(lParam));
			break;
		default:
			return oldWndProc?retval:DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
	return	0;
}//Window procedure

void	CALLBACK _timerProc(HWND hWnd,UINT uMsg,UINT idEvent,DWORD dwTime){
	GLwindow	*pWin;
	if( !hWnd ){
		for(int x=0;x<N_GLWindows;++x)
			if( !All_GLWindows[x]->hWindow ){
				pWin = All_GLWindows[x];
				if(pWin->isActive){
					wglMakeCurrent(pWin->hDC,pWin->hRC);
					if(pWin->DrawGLScene(pWin))SwapBuffers(pWin->hDC);
					else	_DrawGLScene(pWin);
				}			
			}
	}else{
		pWin = (GLwindow*)GetWindowLong(hWnd,GWL_USERDATA);
		if(pWin->isActive){
			wglMakeCurrent(pWin->hDC,pWin->hRC);
			if(pWin->DrawGLScene(pWin))SwapBuffers(pWin->hDC);
			else	_DrawGLScene(pWin);
		}
	}
}//OOP connector for drawing procedure

bool	_DrawGLScene(GLwindow* pWin){
	glClear(GL_COLOR_BUFFER_BIT | (pWin->bUse3D?GL_DEPTH_BUFFER_BIT:0));
	glLoadIdentity();
	return true;
}//Default blank window drawing procedure

void	GLwindow::ResizeGLScene(int width,int height){
	double	Y	=	ddClipNear*tan(ddFOVY*PI/360.0);
	double	X	=	Y*width/height;
	Width		=   width;
	Height		=   height;
	wglMakeCurrent(hDC,hRC);
	if(height <= 0)height = 1;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-X,X,-Y,Y,ddClipNear,ddClipFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}//Resize window function

DWORD	GLwindow::CreateGLWindow(bool fullscreen){
	fullScreen		=	fullscreen;
	if(fullscreen){
		if (!Width && !Height){
			Width	=	dmScreenSettings.dmPelsWidth;
			Height	=	dmScreenSettings.dmPelsHeight;
		}else{
			dmScreenSettings.dmPelsWidth	=	Width;
			dmScreenSettings.dmPelsHeight	=	Height;
		}
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)
			!=	DISP_CHANGE_SUCCESSFUL)return 1;
		if(dwExStyle==0xffffffff)	dwExStyle	=	WS_EX_APPWINDOW;
		if(dwStyle  ==0xffffffff)	dwStyle		=	WS_POPUP;
	}else{
		if (!Width && !Height){
			Width	=	640;
			Height	=	480;
		}//Set default window dimension
		if(dwExStyle==0xffffffff)	dwExStyle	=	(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
		if(dwStyle  ==0xffffffff)	dwStyle		=	WS_OVERLAPPEDWINDOW;
	}

	if(!wcex.lpszClassName){//Generate class name and register class
		wcex.lpszClassName	=	className;
		int	x	=	0;
		do{
			if(x > 99)return 2;
			sprintf(className,"GLclass%2d",x++);
		}while(!RegisterClassEx(&wcex));
	}else{//Register class
		if(!RegisterClassEx(&wcex))return 3;
	}

	RECT	windowRect = {0,0,Width,Height};
	if(!AdjustWindowRectEx(&windowRect, dwStyle, wcex.lpszMenuName!=NULL, dwExStyle)){
		DestroyGLWindow();
		return	4;
	}

	if(hWindow)DestroyGLWindow();
	if((hWindow = CreateWindowEx(dwExStyle,className,windowName,dwStyle,Left,Top,
		windowRect.right-windowRect.left,windowRect.bottom-windowRect.top,
		hParentWindow,ChildWindowID,wcex.hInstance,NULL))==NULL){
		DestroyGLWindow();
		return	5;
	}

	if(!(hDC=GetDC(hWindow))){
		DestroyGLWindow();
		return	6;
	}

	GLuint	PixelFormat;
	if(!(PixelFormat=ChoosePixelFormat(hDC,&pfd))){
		DestroyGLWindow();
		return	7;
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd)){
		DestroyGLWindow();
		return	8;
	}
	DescribePixelFormat(hDC,PixelFormat,sizeof(pfd),&pfd);

	if (!(hRC=wglCreateContext(hDC))){
		DestroyGLWindow();
		return	9;
	}

	if(!wglMakeCurrent(hDC,hRC)){
		DestroyGLWindow();
		return	10;
	}

	ShowWindow(hWindow,SW_SHOW);
	SetForegroundWindow(hWindow);
	SetFocus(hWindow);
	ResizeGLScene(Width, Height);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	if(bUse3D){
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}else glDisable(GL_DEPTH_TEST);

	//Check for vertex buffer object support
	GetExtFunc();

	if(!SetFont(16,0,FW_DONTCARE,false,false,false))return	11;
	All_GLWindows[N_GLWindows++] = this;
	return	0;
}//GL window creation

DWORD	GLwindow::CreateGLWindow(HWND hwnd, bool fullWindow){
	RECT	rect;
	GLuint	PixelFormat;

	fullWindow?GetWindowRect(hwnd,&rect):GetClientRect(hwnd,&rect);
	Width			=	rect.right-rect.left;
	Height			=	rect.bottom-rect.top;
	hWindow			=	hwnd;
	hParentWindow	=	GetParent(hwnd);
	isActive		=	true;
	oldWndProc = (WNDPROC) SetWindowLong(hWindow, GWL_WNDPROC, (DWORD)_wndProc);
	if(!(hDC = fullWindow?GetWindowDC(hWindow):GetDC(hWindow)))return	6;
	if(!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))return 7;
	if(!SetPixelFormat(hDC,PixelFormat,&pfd))return	8;
	DescribePixelFormat(hDC,PixelFormat,sizeof(pfd),&pfd);
	if(!(hRC=wglCreateContext(hDC)))return	9;
	if(!wglMakeCurrent(hDC,hRC))return 10;

	ResizeGLScene(Width, Height);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//Check for vertex buffer object support
	GetExtFunc();

	if(!SetFont(16,0,FW_DONTCARE,false,false,false))return	11;
	All_GLWindows[N_GLWindows++] = this;
	return	0;
}//Create a GL context from a window

DWORD	GLwindow::CreateGLWindow( HDC hDeviceContext ){
	GLuint	PixelFormat;

	Width			=	GetDeviceCaps( hDeviceContext, HORZRES );
	Height			=	GetDeviceCaps( hDeviceContext, VERTRES );
	hParentWindow	=	hWindow	=	NULL;
	isActive		=	true;
	bExtDC			=	true;
	hDC				=	hDeviceContext;

	if(!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))return 7;
	if(!SetPixelFormat(hDC,PixelFormat,&pfd))return	8;
	DescribePixelFormat(hDC,PixelFormat,sizeof(pfd),&pfd);
	if(!(hRC=wglCreateContext(hDC)))return	9;
	if(!wglMakeCurrent(hDC,hRC))return 10;

	ResizeGLScene(Width, Height);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//Check for vertex buffer object support
	GetExtFunc();

	if(!SetFont(16,0,FW_DONTCARE,false,false,false))return	11;
	All_GLWindows[N_GLWindows++] = this;
	return	0;
}//Create a GL context from a device context

DWORD	GLwindow::DestroyGLWindow(){
	int		x;
	DWORD	retval = 0;

	for(x=0; All_GLWindows[x]!=this && x<N_GLWindows; x++);
	if(x<N_GLWindows)All_GLWindows[x] = All_GLWindows[--N_GLWindows];

	KillTimer(hWindow,DEFAULT_TIMER_ID);
	if(fullScreen){
		ChangeDisplaySettings(NULL,0);
		fullScreen  =   false;
	}

	if(hRC){
		if(!wglMakeCurrent(NULL,NULL)) retval |= 1;
		if(!wglDeleteContext(hRC))	retval |= 2;
		hRC	=	NULL;
	}
	if(!bExtDC) if(hDC){
		if(!ReleaseDC(hWindow,hDC)) retval |= 4;
		hDC	=	NULL;
	}
	if(!oldWndProc) if(IsWindow(hWindow)){
		if(!DestroyWindow(hWindow)) retval |= 8;
		hWindow	=	NULL;
	}
	UnregisterClass(wcex.lpszClassName,wcex.hInstance);
	return retval;
}//GL window destruction

GLwindow::Font2D*	GLwindow::BuildFont2D(int height, int width, int weight,
								bool italic,bool underline,bool strikeout,
						  		char *fontFamily){
	HFONT   newFont	=	CreateFont(height,width,0,0,weight,italic, underline,
		strikeout, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, FF_DONTCARE|DEFAULT_PITCH, fontFamily);
	if(!newFont)return NULL;
	return	BuildFont2D(newFont);
}

GLwindow::Font2D*	GLwindow::BuildFont2D(HFONT newFont){
	HFONT   oldFont =   (HFONT)SelectObject(hDC, newFont);
	if(oldFont==NULL || oldFont==(HFONT)GDI_ERROR)return NULL;
	int	newFontBase	=   glGenLists(FONTCOUNT);
	if(!newFontBase){
		SelectObject(hDC, oldFont);
		return  NULL;
	}
	if(!wglUseFontBitmaps(hDC, FONTSTART, FONTCOUNT, newFontBase)){
		glDeleteLists(newFontBase, FONTCOUNT);
		SelectObject(hDC,oldFont);
		return	NULL;
	}
	SelectObject(hDC, oldFont);
	Font2D  *font2D =   new Font2D(newFontBase,newFont);
	return  font2D;
}

GLwindow::Font3D*	GLwindow::BuildFont3D(HFONT newFont, float thickness, int fontType){
 	HFONT   oldFont =   (HFONT)SelectObject(hDC, newFont);
	if(oldFont==NULL || oldFont==(HFONT)GDI_ERROR)return NULL;
	int	newFontBase	=   glGenLists(FONTCOUNT);
	if(!newFontBase){
		SelectObject(hDC, oldFont);
		return  NULL;
	}
	Font3D  *font3D =   new Font3D(newFontBase,newFont);
	if(!wglUseFontOutlines(hDC, FONTSTART, FONTCOUNT, newFontBase, 0.0f,
		thickness, fontType, font3D->gmf)){
		delete  font3D;
		glDeleteLists(newFontBase, FONTCOUNT);
		SelectObject(hDC,oldFont);
		return	NULL;
	}
	SelectObject(hDC, oldFont);
	font3D->thickness   =   thickness*1.6f;//thickness ratio adjust constant
	return  font3D;
}//Set 3D font directly from HFONT

HFONT	GLwindow::SetFont(HFONT newFont){
	int 	newFontBase;
	if( !newFont ){
		SetFont(16,0,FW_DONTCARE,false,false,false);
		return	NULL;
	}
	HFONT   oldFont =   (HFONT)SelectObject(hDC, newFont);
	if(oldFont==NULL || oldFont==(HFONT)GDI_ERROR)return NULL;
	newFontBase    	=   glGenLists(FONTCOUNT);
	if(!newFontBase){
		SelectObject(hDC, oldFont);
		return  NULL;
	}
	if(!wglUseFontBitmaps(hDC, FONTSTART, FONTCOUNT, newFontBase)){
		glDeleteLists(newFontBase, FONTCOUNT);
		SelectObject(hDC, oldFont);
		return NULL;
	}
	if(fontBase)glDeleteLists(fontBase,FONTCOUNT);
	fontBase    =   newFontBase;
	return  oldFont;
}//Set 2D font directly from HFONT

bool	GLwindow::SetFont(int height, int width, int weight,
						  bool italic,bool underline,bool strikeout,
						  char *fontFamily){
	HFONT 	newFont	=	CreateFont(height,width,0,0,weight,italic, underline,
		strikeout, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, FF_DONTCARE|DEFAULT_PITCH, fontFamily);
	if(!newFont)return false;
	HFONT	oldFont	=	SetFont(newFont);
	if(!oldFont){
		DeleteObject(newFont);
		return false;
	}
	DeleteObject(oldFont);
	return  true;
}//Set font from specification discarding previous font

void	GLwindow::glPrintf(char *fmt, ...){
	char	text[256];
	va_list	ap;
	if(fmt == NULL)return;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);
	glPushAttrib(GL_LIST_BIT);
	glListBase(fontBase - FONTSTART);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}//print current font at current raster position
void	GLwindow::glPrintf(Font2D* font2D, char *fmt, ...){
	char	text[256];
	va_list	ap;
	if(fmt == NULL)return;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);
	glPushAttrib(GL_LIST_BIT);
	glListBase(font2D->base - FONTSTART);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}//print specified 2D font at current raster position
void	GLwindow::glPrintf(Font3D* font3D, char *fmt, ...){
    char	text[256];
	va_list	ap;
	if(fmt == NULL)return;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);
	glPushAttrib(GL_LIST_BIT);
	glListBase(font3D->base - FONTSTART);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}//print specified 3D font at current raster position

void    GLwindow::glXYPrintf(int X, int Y, WORD align, char* fmt,...){
/*Note: 'align' specifies where should the raster point be positioned in the 3D text box
  Its 2 lower order bytes specifies alignment along X,Y axis, i.e. align = 0xXXYY
  XX,YY	=	0   extreme left/bottom
			1   middle/center
    		2   extreme right/top
  By default, 'align=0' means the raster point is at the lower left corner of the text box.
*/
	//Convert to text
	char    text[256];
	if(!fmt)return;
	va_list	ap;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);
	SIZE    size = {0,0};
	if(align){
		if(!GetTextExtentPoint32(hDC, text, strlen(text), &size))return;
		switch(align&0x3){
			case 0: size.cy	=	0;break;
			case 1: size.cy >>= 1;break;
		}
		align >>= 8;
 		switch(align&0x3){
			case 0: size.cx	=	0;break;
			case 1: size.cx >>= 1;break;
		}
	}

	//1+	Preserve matrix mode and display list base
	glPushAttrib(GL_TRANSFORM_BIT | GL_LIST_BIT);
	//2+	Reset view matrix
	glMatrixMode(GL_MODELVIEW);
#ifndef	__DRAWFAST
	glPushMatrix();
#endif
 	glLoadIdentity();
	//3+	Set projection matrix
	glMatrixMode(GL_PROJECTION);
#ifndef	__DRAWFAST
	glPushMatrix();
#endif
	glLoadIdentity();
	glOrtho(0,Width,Height-1,0,-1,1);
	//0		Draw text
	glRasterPos2i(X-size.cx,Y+size.cy);
	glListBase(fontBase - FONTSTART);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	//3-	Restore projection matrix
#ifndef	__DRAWFAST
	glPopMatrix();
	//2-	Restore view matrix
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
#endif
	//1-	Restore view matrix selection and display list base
	glPopAttrib();
}//print current font at specified position (X,Y)

void    GLwindow::glXYPrintf(Font2D *font2D, int X, int Y, WORD align, char* fmt,...){
	//Convert to text
	char    text[256];
	va_list	ap;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);
	HFONT   oldFont = (HFONT)SelectObject(hDC,font2D->hFont);
	if(oldFont==NULL || oldFont==(HFONT)GDI_ERROR)return;
	int oldBase =	fontBase;
	fontBase	=	font2D->base;
	glXYPrintf(X, Y, align, text);
	fontBase    =   oldBase;
	SelectObject(hDC,oldFont);
}//print specified 2D font at specified position (X,Y)

void    GLwindow::glXYZPrintf(Font3D *font3D,int X,int Y,int Z,DWORD align,char* fmt,...){
/*Note: 'align' specifies where should the raster point be positioned in the 3D text box
  Its 3 lower order bytes specifies alignment along X,Y,Z axis, i.e. align = 0x00XXYYZZ
  XX,YY,ZZ =	0   extreme left
                1   middle
                2   extreme right
  By default, 'align=0' means the raster point is at the lower left front corner of the
  text box.
*/
	//Convert to text
	char	text[256];
	float   disp[3] = {0,0,0};//XYZ displacement

	if(fmt == NULL)return;
	va_list	ap;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	int     x, len=strlen(text);
	float   textBox[3]={0,0,0};//width, height, thickness of the text box
	if(align){
		for(x=0; x<len; x++){
			textBox[0]	+=	font3D->gmf[text[x]-FONTSTART].gmfCellIncX;
			textBox[1]	=	max(textBox[1],font3D->gmf[text[x]-FONTSTART].gmfBlackBoxY);
		}
		textBox[2]	=	textBox[1] * font3D->thickness;
		for(x=2; x>=0; x--,align>>=8){
			switch(align&0xff){
				case 1: disp[x] = textBox[x]/2;break;
				case 2: disp[x] = textBox[x];break;
			}
		}
	}
	glPushAttrib(GL_LIST_BIT);
	glListBase(font3D->base - FONTSTART);
	glTranslatef(-disp[0], -disp[1], disp[2]);
	glCallLists(len, GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}//print specified 3D font at specified position (X,Y,Z)

bool	GLwindow::SetFPS(int FPS){
	if(FPS){
		ticksPerFrame	=	1000/FPS;
		if(SetTimer(hWindow,DEFAULT_TIMER_ID,ticksPerFrame,_timerProc))
			return  true;
	}
	KillTimer(hWindow,DEFAULT_TIMER_ID);
	ticksPerFrame	=	0;
	return	false;
}//Restrict FPS to save CPU %

#endif