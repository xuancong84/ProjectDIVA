#ifndef __DefineH__
#define __DefineH__

#define __D3DRM_H__

#include <d3dx9.h>
#include <d3d9.h>
#include <windows.h>
#include <mmsystem.h>
#include <atlbase.h>
#include <stdio.h>
#include <d3d9types.h>
#include <tchar.h>
#include <windowsx.h> 
#include <string>
#include <queue>
#include <time.h>
#include <dshow.h>
#include <vmr9.h>
#include "..\DShowClass\qedit.h"
#include "..\DShowClass\streams.h"
#include <map>
#include <string>
#include <fstream>
using namespace std;

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"strmiids.lib")
#pragma comment(lib,"quartz.lib")

#pragma comment(linker, "/NODEFAULTLIB:atlthunk.lib")

//#define MouseEventValue bLeftButtonDown,bRightButtonDown,bMiddleButtonDown,bSideButton1Down,bSideButton2Down,nMouseWheelDelta,xPos,yPos
#define KeyEventValue nChar, bKeyDown
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#define SAFE_RELEASE_ARRAY(p) { if(p) { delete[] (p); (p)=NULL; }}
#define PI 3.1415926f
#define EPS 1e-7
#define WIDTH 480
#define HEIGHT 272

//keyboard
#define KeyUp 38
#define KeyDown 40
#define KeyLeft 37
#define KeyRight 39
#define KeyEnter 13
#define KeySpace 32
#define KeyEsc 27

#define GameCoreRes 2

//GAMEMODE
#define GAME_MODE_NORMAL	0
#define GAME_MODE_AUTO	1
#define GAME_MODE_PV	2
#define key_mapsN	4
#define	optionsN	27
#define MAX_STRING_INDEX	256

#define  InsideDrawRange(x,y) ( x>=0 && y>=0 && x<=480 && y<=272 )
#define  InsideDrawRangeEx(x,y) ( x>-20 && y>-20 && x<500 && y<292 )

class GUIMessageBox;
struct fRECT{
	float left;
	float top;
	float right;
	float bottom;
	fRECT &operator += (fRECT &rhs)
	{
		this->left	+= rhs.left;
		this->top	+= rhs.top;
		this->right	+= rhs.right;
		this->bottom+= rhs.bottom;
		return *this;
	}
};

void null();
void ErrorExit(string error_infor,int exit_code);
RECT MakeRect(int px,int py,int width,int height);
RECT MakeRect(const fRECT &rect);
fRECT MakefRect(const RECT &rect);
fRECT MakefRect(int px,int py,int width,int height);
bool InsideRect(float x, float y, const fRECT &rect);
bool InsideRect(int x, int y, const RECT &rect);
void SetLength(float &dx, float &dy, float set_length);
float UnitLength(float &dx, float &dy);
void RandomDir(float &dx, float &dy);
DWORD RandomColor();
void SetRectPos(int px,int py, RECT &src);
LONGLONG GetCurrentCount(void);
bool utf8_check_is_valid(const LPCSTR str);
wstring Ansi2WChar(const string& szSrc, UINT codepage=0xffff);
wstring Ansi2WChar(const LPCSTR pszSrc, UINT codepage=0xffff);
void checkTail0(char *s,int maxl);
void wcheckTail0(wchar_t *s,int maxl);
typedef IDirect3DTexture9 Tex,Texture;
bool isImage(string filename);
bool operator==(const RECT &rect1,const RECT &rect2);
string ToString(int num);
string ToString(double num,const char *_Format);
string printKey(UINT nChar);
string printKeys(int key, int maxlen);

extern const int g_windowSizesN;
extern int g_windowSizes[][2];
extern vector<string> g_StringTable;
extern RECT g_defaultRect;
extern const int cfg_menu_option_idN;
extern int cfg_menu_option_id[];
extern vector <string> g_button_symbols;
extern char *key_map;
extern GUIMessageBox *lock_msgbox;

class Point
{
public:
	float x,y;
	Point():x(0),y(0) {}
	Point(const float &_x, const float &_y):x(_x),y(_y) {}
	float mod() {return sqrt(x*x+y*y);}
	Point unit() {float tmp = mod(); return Point(x/tmp,y/tmp);}
	Point normal() {return Point(y,-x);}
	friend Point operator+(const Point &a, const Point &b) {return Point(a.x+b.x,a.y+b.y);}
	friend Point operator-(const Point &a, const Point &b) {return Point(a.x-b.x,a.y-b.y);}
	friend Point operator*(const Point &a, const float &b) {return Point(a.x*b,a.y*b);}
	friend float operator*(const Point &a, const Point &b) {return a.x*b.x+a.y*b.y;}
	friend float cross(const Point &a, const Point &b) {return a.x*b.y-b.x*a.y;}
};

Point Bezier(const Point &p0, const Point &p1, const Point &p2, const float &t);
Point Bezier(const Point &p0, const Point &p1, const Point &p2, const Point &p3, const float &t);
Point BezierDir(const Point &p0, const Point &p1, const Point &p2, const Point &p3, const float &t);

class SystemIni
{
public:
	SystemIni(){}
	~SystemIni(){}

	char custom_key_map[256];	// load from the file, may not be in use
	vector <string> locales;	// list of all locales in ./locale/*.txt
	vector <string> fonts, fonts_utf8;
	string font, font_utf8;

	void Init();

	float sndVolume;
	int winWidth,winHeight;
	bool windowed;
	int soundVolume;
	int GameKeyType; //1-PC 2-PSP 3-O2JAMPC 4-O2JAMPSP
	int winSizeType; //1-480*272 2-720*408 3-960*544 4-desktop_resolution
	int language;
	int particle;
	int keyHook;
	int autoPause;
	int showFPS;	// 0:OFF 1:ON 2:ON(OSD)
	int lowVideo;	// 0:OFF 1:ON
	int dataSet;
	int fontIndex;
	int antialiasing;
	int judgement;
	int GlobalAudioDelay, KeepPitch;
	int AudioDSPBufferSize, AudioDSPBufferCount;
	int DelayCompen;	// in milliseconds
	int game_level;
	int sound_system;	// 0: FMOD, 1: BASS
	int FMOD_DSP_FFT_WINDOW;
	int BASS_ATTRIB_TEMPO_OPTION_SEQUENCE_MS;
	int BASS_ATTRIB_TEMPO_OPTION_SEEKWINDOW_MS;
	int BASS_ATTRIB_TEMPO_OPTION_OVERLAP_MS;

	// anti-aliasing
	vector <pair<D3DMULTISAMPLE_TYPE,int>> multisampling;
	vector <pair<D3DMULTISAMPLE_TYPE,int>> multisamplingN;
	int getAntiAliasingInfo();

	// Change options
	void ChangeGameKeyType(int type, bool bSave=true);
	void ChangeParticleSystem(int type, bool bSave=true);
	void ChangeLanguage(int type, bool bSave=true);
	void ChangeSndVolume(int volume, bool bSave=true);
	void ChangeWindowSize(int sizeType, bool bSave=true);
	void ChangeKeyHook(int bHook, bool bSave=true);
	void ChangeFPSstatus(int state, bool bSave=true);
	void ChangeOption(int state, bool bSave=true);
	void ChangeAutoPause(int pause_method, bool bSave=true);
	void ChangeDataSet(int data_set, bool bSave=true);
	void ChangeFont(int font_id, bool bSave=true);
	void ChangeAntialiasing(int aa_id, bool bSave=true);
	void ChangeDelayCompen(int delay, bool bSave=true);
	void ChangeSoundSystem(int system, bool bSave=true);
	void ChangeToDefault();

	// keyboard
	void EnterReset(int pValue, bool bSave=true);
	void EnterSetKey(int pValue, bool bSave=true);
	void EnterSndVolume(int pValue, bool bSave=true);

	// internal
	void SetDefaultValue();
	void ApplyAllSettings();

	// save/load options
	void SaveIniFile();
	bool LoadIniFile();

	// Keyboard
	void LoadKeyMap(char *filename);
	void SaveKeyMap(char *filename);
	void ApplyKeyMap(char*,BYTE*);

	// Others
	vector <string>& GetLanguageNames();
};

struct Option{
	string	option_name;
	int		default_value;
	int		min_val, max_val;
	int		*pValue;
	void	(SystemIni::*ChangeSettings)(int, bool);
	void	(SystemIni::*EnterSettings)(int, bool);
};
extern Option *options;
extern SystemIni systemIni;

//#define systemIni SystemIni::GetInstance()
#define StringTable(p) ((p)<g_StringTable.size()?g_StringTable[p]:"")
wstring str2wstr(string &s);
string Ansi2UTF8(string s_ansi);
wchar_t *CodePageToUnicode(int codePage, const char *src);
char *UnicodeToCodePage(int codePage, const wchar_t *src);
BOOL SetWindowTitle(HWND hwnd, const char *);
string join(vector <string> tokens, string delimiter);
vector <string> split(const string &s, string delimiter);

void to_integer_point(double &val);
void to_integer_point(float &val);

#endif