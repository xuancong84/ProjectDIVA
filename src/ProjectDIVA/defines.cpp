#include "defines.h"
#include "UIScreen.h"
#include "keyboard.h"
#include "GameMana.h"
#include "resource.hpp"
#include <string.h>
#include <sstream>

#pragma warning (disable : 4996)

SystemIni systemIni;

// This is the standard 16:9 aspect ratio, 480x272 is not exactly 16:9
// Window sizes: 0-480*270 1-720*405 2-1024*576 3-Fullscreen 4-Custom
const int g_windowSizesN = 3;
int g_windowSizes[g_windowSizesN+1][2] = {{480,270},{720,405},{1024,576},{0,0}};
RECT g_defaultRect = {0,0,480,272};

// Language specific texts
int getMaxSkin();
vector <string> g_StringTable;

wstring str2wstr(string &s)
{
	wchar_t *buf = new wchar_t[ s.size() ];
	size_t num_chars = mbstowcs( buf, s.c_str(), s.size() );
	wstring ws( buf, num_chars );
	delete[] buf;
	return ws;
}

void null(){}

void ErrorExit(string error_infor,int exit_code)
{
	MessageBoxW(NULL, Ansi2WChar(error_infor).data(), L"DIVA", MB_OK|MB_ICONEXCLAMATION);
	exit(exit_code);
}

LONGLONG GetCurrentCount(void)
{
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	return tmp.QuadPart;
}
RECT MakeRect(int px,int py,int width,int height)
{
	RECT ret;
	ret.top=py;
	ret.left=px;
	ret.right=px+width;
	ret.bottom=py+height;
	return ret;
}
fRECT MakefRect(int px,int py,int width,int height)
{
	fRECT ret;
	ret.top=py;
	ret.left=px;
	ret.right=px+width;
	ret.bottom=py+height;
	return ret;
}
RECT MakeRect(const fRECT &rect)
{
	RECT ret;
	ret.top = rect.top+0.5f;
	ret.left = rect.left+0.5f;
	ret.right = rect.right+0.5f;
	ret.bottom = rect.bottom+0.5f;
	return ret;
}
fRECT MakefRect(const RECT &rect)
{
	fRECT ret;
	ret.top = rect.top;
	ret.left = rect.left;
	ret.right = rect.right;
	ret.bottom = rect.bottom;
	return ret;
}
bool InsideRect(float x, float y, const fRECT &rect)
{
	return x>=rect.left && x<rect.right && y>=rect.top && y<rect.bottom;
}
bool InsideRect(int x, int y, const RECT &rect)
{
	return x>=rect.left && x<rect.right && y>=rect.top && y<rect.bottom;
}
void SetRectPos(int px,int py,RECT &src)
{
	RECT ret;
	ret.left = px;
	ret.top = py;
	ret.right = px + src.right - src.left;
	ret.bottom = py + src.bottom - src.top;
	src = ret;
}

void RandomDir(float &dx, float &dy){
	float angle = (float)rand()/RAND_MAX*6.2831853f;
	dx = cos(angle);
	dy = sin(angle);
}
float UnitLength(float &dx, float &dy){
	float length = sqrt(dx*dx+dy*dy);
	if(length>0){
		dx /= length;
		dy /= length;
	}
	return length;
}
void SetLength(float &dx, float &dy, float set_length){
	float length = sqrt(dx*dx+dy*dy);
	if(length>0){
		length = set_length/length;
		dx *= length;
		dy *= length;
	}
}

DWORD RandomColor(){
	BYTE rgb[4]={rand()&0xff,rand()&0xff,rand()&0xff,rand()&0xff};
	return *(DWORD*)rgb;
}

bool utf8_check_is_valid(const char * str)
{
	int c,i,ix,n,j;
	for (i=0, ix=strlen(str); i < ix; i++)
	{
		c = (unsigned char) str[i];
		//if (c==0x09 || c==0x0a || c==0x0d || (0x20 <= c && c <= 0x7e) ) n = 0; // is_printable_ascii
		if (0x00 <= c && c <= 0x7f) n=0; // 0bbbbbbb
		else if ((c & 0xE0) == 0xC0) n=1; // 110bbbbb
		else if ( c==0xed && i<(ix-1) && ((unsigned char)str[i+1] & 0xa0)==0xa0) return false; //U+d800 to U+dfff
		else if ((c & 0xF0) == 0xE0) n=2; // 1110bbbb
		else if ((c & 0xF8) == 0xF0) n=3; // 11110bbb
		else if ((c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
		else if ((c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
		else return false;
		for (j=0; j<n && i<ix; j++) { // n bytes matching 10bbbbbb follow ?
			if ((++i == ix) || (( (unsigned char)str[i] & 0xC0) != 0x80))
				return false;
		}
	}
	return true;
}
wstring Ansi2WChar(const LPCSTR pszSrc, UINT codepage)
{
	if(codepage==0xffff)
		codepage = utf8_check_is_valid(pszSrc)?CP_UTF8:CP_ACP;
	int nSize = MultiByteToWideChar(codepage, 0, (LPCSTR)pszSrc, -1, NULL, 0);
	if(nSize <= 0) return L"";
	WCHAR *pwszDst = new WCHAR[nSize+1];
	if(!pwszDst) return L"";
	MultiByteToWideChar(codepage, 0,(LPCSTR)pszSrc, -1, pwszDst, nSize);
	pwszDst[nSize] = 0;
	if( pwszDst[0] == 0xFEFF) // skip Oxfeff
		for(int i = 0; i < nSize; i ++) 
			pwszDst[i] = pwszDst[i+1]; 
	wstring wcharString(pwszDst);
	delete pwszDst;
	return wcharString;
}
wstring Ansi2WChar(const string& s, UINT codepage){ return Ansi2WChar(s.data(), codepage);}

wchar_t *CodePageToUnicode(int codePage, const char *src)
{
	if (!src) return 0;
	int srcLen = strlen(src);
	if (!srcLen)
	{
		wchar_t *w = new wchar_t[1];
		w[0] = 0;
		return w;
	}

	int requiredSize = MultiByteToWideChar(codePage, 0, src,srcLen,0,0);
	if (!requiredSize)
		return 0;

	wchar_t *w = new wchar_t[requiredSize+1];
	w[requiredSize] = 0;

	int retval = MultiByteToWideChar(codePage,	0,	src,srcLen,w,requiredSize);
	if (!retval)
	{
		delete [] w;
		return 0;
	}

	return w;
}

char *UnicodeToCodePage(int codePage, const wchar_t *src)
{
	if (!src) return 0;
	int srcLen = wcslen(src);
	if (!srcLen)
	{
		char *x = new char[1];
		x[0] = '\0';
		return x;
	}

	int requiredSize = WideCharToMultiByte(codePage,0,src,srcLen,0,0,0,0);
	if (!requiredSize)
		return 0;

	char *x = new char[requiredSize+1];
	x[requiredSize] = 0;

	int retval = WideCharToMultiByte(codePage,	0,	src,srcLen,x,requiredSize,0,0);
	if (!retval)
	{
		delete [] x;
		return 0;
	}

	return x;
}

string Ansi2UTF8(string s_ansi){
	wchar_t *wText = CodePageToUnicode(CP_ACP, s_ansi.data());
	char *utf8Text = UnicodeToCodePage(65001,wText);
	string s_utf8(utf8Text);
	delete [] wText,utf8Text;
	return s_utf8;
}
string join(vector <string> tokens, string delimiter){
	string ret;
	for(auto it=tokens.begin(); it!=tokens.end(); ++it){
		if(it!=tokens.begin())
			ret+=delimiter;
		ret+=*it;
	}
	return ret;
}
vector <string> split(const string &s, string delimiter){
	vector <string> ret;
	int posi=0;
	for(int fposi=0; (fposi=s.find(delimiter, posi))!=string::npos; posi=fposi+delimiter.size()){
		ret.push_back(s.substr(posi,fposi-posi));
	}
	if(posi<s.size())
		ret.push_back(s.substr(posi));
	return ret;
}
string ReplaceString(string subject, const string& search,
	const string& replace) {
		size_t pos = 0;
		while((pos = subject.find(search, pos)) != string::npos) {
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
		return subject;
}
string& ReplaceStringInPlace(string& subject, const string& search,
	const string& replace) {
		size_t pos = 0;
		while((pos = subject.find(search, pos)) != string::npos) {
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
		return subject;
}
string EscapeStringInPlace(string &s_in){
	ReplaceStringInPlace(s_in, "\\n", "\n");
	ReplaceStringInPlace(s_in, "\\r", "\r");
	return ReplaceStringInPlace(s_in, "\\\"", "\"");
}
int load_utf8_text(vector <string> &v_string, const string filename){
	v_string.clear();
	ifstream fin(filename);
	string line;
	if(fin){
		while(getline(fin, line))
			v_string.push_back(EscapeStringInPlace(line));
		return v_string.size();
	}
	return 0;
}

BOOL SetWindowTitle(HWND hwnd, const char *text){
	wchar_t *buf = CodePageToUnicode(CP_UTF8, text);
	BOOL retval = SetWindowTextW(hwnd, buf);
	delete [] buf;
	return retval;
}

void checkTail0(char *s,int maxl)
{
	int l=0;
	while(l<maxl)
	{
		if(s[l]==10)
		{
			s[l]=0;
			break;
		}
		l++;
	}
}

void wcheckTail0(wchar_t *s,int maxl)
{
	int l=0;
	while(l<maxl)
	{
		if(s[l]==10)
		{
			s[l]=0;
			break;
		}
		l++;
	}
}

bool isImage(string filename)
{
	string ret;
	for(int i=filename.length()-1;i>=0;i--)
	{
		if(filename[i]=='.')
			break;

		ret=char((filename[i]>='A'&&filename[i]<='Z')?(filename[i]-('A'-'a')):(filename[i]))+ret;
	}
	
	if(ret=="jpg"||ret=="png"||ret=="bmp"||ret=="dds"||ret=="tga")
		return true;
	else
		return false;
}

bool operator==(const RECT &rect1,const RECT &rect2)
{
	return (rect1.left==rect2.left&&rect1.bottom==rect2.bottom&&rect1.right==rect2.right&&rect1.top==rect2.top);
}


string GetLocale(){
	char loc_system[LOCALE_NAME_MAX_LENGTH], loc_user[LOCALE_NAME_MAX_LENGTH];
	if(!GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO639LANGNAME, loc_system, LOCALE_NAME_MAX_LENGTH))
		return "en";
	if(!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, loc_user, LOCALE_NAME_MAX_LENGTH))
		return loc_system;
	if(loc_system!=string("en"))
		return loc_user;
	if(loc_user!=string("en"))
		return loc_system;
	return "en";
}

int getMaxSkin(){
	for(int x=1; true; x++){
		ostringstream oss;
		oss << "gamedata" << x << ".txt";
		ifstream fin(oss.str().c_str());
		if(!fin) return x-1;
	}
}
int unused;

int SystemIni::getAntiAliasingInfo()
{
	multisampling.clear();
	multisamplingN.clear();
	IDirect3D9 *d3d9 = NULL;
	d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	if(!d3d9) return 0;
	for(int x=0; x<=16; x++){
		DWORD nQuality;
		if(SUCCEEDED(d3d9->CheckDeviceMultiSampleType(
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				D3DFMT_A8R8G8B8,
				base::d3dpp.Windowed,
				(D3DMULTISAMPLE_TYPE)x,
				&nQuality))){
			for(int y=0; y<nQuality; y++)
				multisampling.push_back(make_pair((D3DMULTISAMPLE_TYPE)x,y));
			multisamplingN.push_back(make_pair((D3DMULTISAMPLE_TYPE)x,nQuality));
		}
		
	}
	d3d9->Release();
	options[13].max_val = multisampling.size();
	if(options[13].pValue)
		if(*options[13].pValue>multisampling.size())
			*options[13].pValue=multisampling.size();
	return multisampling.size();
}

int CALLBACK EnumFontsProc(LOGFONT *lplf, TEXTMETRIC *lptm, DWORD dwType, LPARAM lParam){
	if(lplf->lfFaceName[0]!='@'){
		systemIni.fonts.push_back(lplf->lfFaceName);
		systemIni.fonts_utf8.push_back(Ansi2UTF8(lplf->lfFaceName));
	}
	return 1;
}

Option *options = NULL;
const int cfg_menu_option_idN = 17;
int cfg_menu_option_id[cfg_menu_option_idN]={5,0,1,4,6,10,11,7,8,9,12,13,14,18,17,23,15};	// Add into 2nd last posi

void SystemIni::Init()
{
	// List locales
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile("locale\\*.txt", &fd);
	locales.clear();
	do{
		string fn = fd.cFileName;
		locales.push_back(fn.substr(0, fn.size()-4));
	}while(FindNextFile(hFind, &fd));
	FindClose(hFind);
	int p_en	= find(locales.begin(), locales.end(), "en")-locales.begin();
	int p_user	= find(locales.begin(), locales.end(), GetLocale())-locales.begin();

	// List fonts
	EnumFonts(GetDC(base::hwnd), NULL, (FONTENUMPROC)&EnumFontsProc, (LPARAM)this);
	int p_font	= find(fonts.begin(), fonts.end(), "Time New Roman")-fonts.begin();

	// Load symbols
	static char symbols[] = "↑ ↓ ← → △ × □ ○ ★ ☆";
	istringstream iss(symbols);
	g_button_symbols.clear();
	string s;
	while(iss>>s)
		g_button_symbols.push_back(Ansi2UTF8(s));

	// Config menu item information
	static Option _options[optionsN]={
		{"GameKeyType", 0, 0, 4, &GameKeyType, &SystemIni::ChangeGameKeyType, &SystemIni::EnterSetKey},
		{"WindowSize", 1, 0, g_windowSizesN, &winSizeType, &SystemIni::ChangeWindowSize, &SystemIni::ChangeWindowSize},
		{"WindowWidth", g_windowSizes[1][0], -1, -1, &winWidth},
		{"WindowHeight", g_windowSizes[1][1], -1, -1, &winHeight},
		{"SoundVolume", 5, 0, 10, &soundVolume, &SystemIni::ChangeSndVolume,&SystemIni::EnterSndVolume},
		{"Language", p_user<locales.size()?p_user:p_en, 0, locales.size()-1, &language, &SystemIni::ChangeLanguage},
		{"Particle", 3, 0, 3, &particle, &SystemIni::ChangeParticleSystem},
		{"KeyHook", 2, 0, 2, &keyHook, &SystemIni::ChangeKeyHook},
		{"AutoPause", 1, 0, 2, &autoPause, &SystemIni::ChangeAutoPause},
		{"FPS", 0, 0, 2, &showFPS, &SystemIni::ChangeFPSstatus},
		{"LowVideo", 0, 0, 9, &lowVideo, &SystemIni::ChangeOption},			// 10
		{"DataSet", getMaxSkin(), 0, getMaxSkin(), &dataSet, &SystemIni::ChangeDataSet},
		{"Font", p_font<fonts.size()?p_font:0, 0, fonts.size()-1, &fontIndex, &SystemIni::ChangeFont},
		{"Antialiasing", 1, 0, 1, &antialiasing, &SystemIni::ChangeAntialiasing},
		{"Judgement", 0, 0, 1, &judgement, &SystemIni::ChangeOption},
		{"Reset", 0, 0, 0, &unused, NULL, &SystemIni::EnterReset},
		{"GlobalAudioDelay", 0, -1000, 1000, &GlobalAudioDelay},
		{"PitchShiftDSP", 0, 0, 1, &KeepPitch, &SystemIni::ChangeOption},
		{"DelayCompen", 0, 0, 100, &DelayCompen, &SystemIni::ChangeDelayCompen},
		{"GameLevel", 1, 0, GAME_LEVEL_MAX-1, &game_level, &SystemIni::ChangeOption},
		{"AudioDSPBufferSize", 1024, 256, 2048, &AudioDSPBufferSize},		// 20
		{"AudioDSPBufferCount", 2, 2, 4, &AudioDSPBufferCount},
		{"FMOD_DSP_FFT_WINDOW", 1024, 64, 65536, &FMOD_DSP_FFT_WINDOW},
		{"SoundSystem", 0, 0, 1, &sound_system, &SystemIni::ChangeSoundSystem},
		{"BASS_ATTRIB_TEMPO_OPTION_SEQUENCE_MS", 20, 41, 164, &BASS_ATTRIB_TEMPO_OPTION_SEQUENCE_MS},
		{"BASS_ATTRIB_TEMPO_OPTION_SEEKWINDOW_MS", 10, 14, 56, &BASS_ATTRIB_TEMPO_OPTION_SEEKWINDOW_MS},
		{"BASS_ATTRIB_TEMPO_OPTION_OVERLAP_MS", 10, 4, 16, &BASS_ATTRIB_TEMPO_OPTION_OVERLAP_MS},
	};
	if(!options){
		options = _options;
		SetDefaultValue();
	}
	if(!LoadIniFile())
	{
		SaveIniFile();
	}
	getAntiAliasingInfo();
	ApplyAllSettings();
}

void SystemIni::ApplyAllSettings(){
	ChangeGameKeyType(GameKeyType,false);
	ChangeSndVolume(soundVolume,false);
	ChangeLanguage(language,false);
	ChangeParticleSystem(particle,false);
	ChangeKeyHook(keyHook,false);
	ChangeFont(fontIndex,false);
	ChangeAntialiasing(antialiasing,false);
	ChangeDelayCompen(DelayCompen,false);
	ChangeSoundSystem(sound_system, false);
}

void SystemIni::ChangeToDefault()
{
	SetDefaultValue();
	ApplyAllSettings();
	ChangeWindowSize(winSizeType,false);	// this one is special
	SaveIniFile();
}
void SystemIni::ChangeOption(int state, bool bSave)
{
	if(bSave) SaveIniFile();
}
void SystemIni::ChangeSoundSystem(int system, bool bSave)
{
	bool isFirstTime = true;
	if(soundEngine){
		soundEngine->Free();
		delete soundEngine;
		soundEngine = NULL;
		isFirstTime = false;
	}
	if(system==0)
		soundEngine = new FmodSoundEngine();
	else
		soundEngine = new BassSoundEngine();
	soundEngine->Init();
	if(!isFirstTime)
		soundEngine->PlayMusic("gamedata\\mainmenu.mp3",0,true);
	if(bSave) SaveIniFile();
}
void SystemIni::ChangeDataSet(int data_set, bool bSave)
{
	ostringstream oss;
	oss << "gamedata" << data_set << ".txt";
	g_res = ResourceLoader(oss.str().c_str());
	if(bSave)
	{
		SaveIniFile();
		GAMEDATA.Init();
		gameui.Init();
	}
}
void SystemIni::ChangeFont(int font_id, bool bSave)
{
	font = fonts[font_id];
	font_utf8 = fonts_utf8[font_id];
	if(bSave) SaveIniFile();
}
void SystemIni::ChangeDelayCompen(int delay, bool bSave)
{
	Option &option = options[18];
	if(delay<option.min_val)
		delay=option.min_val;
	if(delay>option.max_val)
		delay=option.max_val;
	DelayCompen = delay;
	base::delayCompenTime = (DelayCompen*0.001*base::SECOND+0.5);
	if(bSave) SaveIniFile();
	if(ScreenState==SCREEN_GAMEMAIN){
		char msg[128];
		sprintf(msg," = %d ms",delay);
		ShowMessage(StringTable(99)+msg,1,0,32);
	}
}
void SystemIni::ChangeFPSstatus(int state, bool bSave)
{
	if(state!=1) SetWindowTitle(base::hwnd, base::Name);
	if(bSave) SaveIniFile();
}
void SystemIni::ChangeKeyHook(int iHook, bool bSave)
{
	switch(iHook){
	case 0:
		if( base::keyboard_thread_id )
			PostThreadMessage(base::keyboard_thread_id, WM_QUIT, 0, 0);
		Sleep(10);
		break;
	case 1:
		if(dInputKeyboard.status==1)
			dInputKeyboard.stop();
		CreateThread(NULL,0,base::KeyboardThreadFunc,NULL,0,&base::keyboard_thread_id);
		break;
	case 2:
		if( base::keyboard_thread_id )
			PostThreadMessage(base::keyboard_thread_id, WM_QUIT, 0, 0);
		Sleep(10);
		dInputKeyboard.start();
		break;
	}
	if(bSave) SaveIniFile();
}

BYTE default_key_maps[key_mapsN][8]={
	{'D', 'A', 'S', 'W', KeyRight, KeyLeft, KeyDown, KeyUp},
	{KeyRight, KeyLeft, KeyDown, KeyUp, 'D', 'A', 'S', 'W'},
	{'F', 'S', 'D', 'E', 'L', 'J', 'K', 'I'},
	{'L', 'J', 'K', 'I', 'F', 'S', 'D', 'E'},
};
char current_key_map[256];	// maps virtual-key code to UNIT_TYPE (0~7), global, currently in use
char *key_map = current_key_map;

void SystemIni::ChangeGameKeyType(int type, bool bSave)
{
	if( type<key_mapsN ){
		key_map = current_key_map;
		ApplyKeyMap(key_map, default_key_maps[type]);
	}else{
		key_map = custom_key_map;
	}
	if(bSave) SaveIniFile();
}
void SystemIni::ChangeSndVolume(int volume, bool bSave)
{
	soundVolume = volume;
	sndVolume = double(soundVolume)/10.0;
	if(bSave) SaveIniFile();
}
void SystemIni::EnterSndVolume(int volume, bool bSave)
{
	soundEngine->PlayHit(1);
}
void SystemIni::ChangeWindowSize(int sizeType, bool bSave)
{
	winWidth = g_windowSizes[sizeType][0];
	winHeight = g_windowSizes[sizeType][1];
	windowed = (sizeType!=g_windowSizesN);
	winSizeType = sizeType;
	base::ResizeWindow(winWidth, winHeight);
	if(bSave) SaveIniFile();
}
void SystemIni::ChangeAntialiasing(int sizeType, bool bSave)
{
	int index = sizeType?sizeType-1:0;
	UnitDraw_Bezier = (sizeType?&(Unit::Draw_Bezier2):&(Unit::Draw_Bezier1));
	d3dpp.MultiSampleType = multisampling[index].first;
	d3dpp.MultiSampleQuality = multisampling[index].second;
	if(bSave){
		if(base::Device->Reset( &d3dpp ) != D3D_OK)
			while(!base::D3DResetDevice());
		SaveIniFile();
	}
}
void (Unit::*drawLineFuncs[4])(const Point &, const Point &) = {
	&(Unit::DrawNullLine),
	&(Unit::DrawSingleLine),
	&(Unit::DrawDoubleLine),
	&(Unit::DrawCometLine)
};
void SystemIni::ChangeParticleSystem(int _particle, bool bSave){
	UnitDrawLine = drawLineFuncs[systemIni.particle];
	particle = _particle;
	if(bSave) SaveIniFile();
}
void SystemIni::ChangeLanguage(int _language, bool bSave){
	language = _language;
	load_utf8_text(g_StringTable, "locale\\"+locales[language]+".txt");
	if(bSave) SaveIniFile();
}
vector <string>& SystemIni::GetLanguageNames()
{
	static vector <string> langs;
	if(langs.empty())
		for(auto it=locales.begin(); it!=locales.end(); ++it){
			string first_line;
			getline(ifstream("locale\\"+(*it)+".txt"), first_line);
			langs.push_back(first_line);
		}
	return langs;
}

void SystemIni::ChangeAutoPause(int pause_method, bool bSave)
{
	autoPause = pause_method;
	if(bSave) SaveIniFile();
}
void SystemIni::EnterReset(int unused, bool bSave)
{
	soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
	ChangeToDefault();
	base::ResizeWindow(systemIni.winWidth,systemIni.winHeight);
	if(bSave) SaveIniFile();
}

vector <string> g_button_symbols;	//	"↑", "↓", "←", "→", "△", "×", "□", "○"

GUIMessageBox *lock_msgbox = NULL, *old_lock_msgbox = NULL;
string printKey(UINT nChar){
	ostringstream oss;
	switch(nChar){
	case VK_UP:			oss << g_button_symbols[0]; break;
	case VK_DOWN:		oss << g_button_symbols[1]; break;
	case VK_LEFT:		oss << g_button_symbols[2]; break;
	case VK_RIGHT:		oss << g_button_symbols[3]; break;
	case VK_HOME:		oss << "HOME"; break;
	case VK_END:		oss << "END"; break;
	case VK_PRIOR:		oss << "PageUp"; break;
	case VK_NEXT:		oss << "PageDown"; break;
	case VK_SPACE:		oss << StringTable(89); break;
	case VK_OEM_COMMA:	oss << ","; break;
	case VK_OEM_PERIOD:	oss << "."; break;
	case VK_OEM_MINUS:	oss << "-"; break;
	case VK_OEM_PLUS:	oss << "="; break;
	case VK_OEM_2:		oss << "/"; break;
	case VK_OEM_4:		oss << "["; break;
	case VK_OEM_5:		oss << "\\"; break;
	case VK_OEM_6:		oss << "]"; break;
	case VK_OEM_1:		oss << ";"; break;
	case VK_OEM_7:		oss << "'"; break;
	case VK_NUMPAD0:	oss << "NumPad0"; break;
	case VK_NUMPAD1:	oss << "NumPad1"; break;
	case VK_NUMPAD2:	oss << "NumPad2"; break;
	case VK_NUMPAD3:	oss << "NumPad3"; break;
	case VK_NUMPAD4:	oss << "NumPad4"; break;
	case VK_NUMPAD5:	oss << "NumPad5"; break;
	case VK_NUMPAD6:	oss << "NumPad6"; break;
	case VK_NUMPAD7:	oss << "NumPad7"; break;
	case VK_NUMPAD8:	oss << "NumPad8"; break;
	case VK_NUMPAD9:	oss << "NumPad9"; break;
	case VK_MULTIPLY:	oss << "NumPad*"; break;
	case VK_ADD:      	oss << "NumPad+"; break;
	case VK_SEPARATOR:	oss << "NumPad;"; break;
	case VK_SUBTRACT:	oss << "NumPad-"; break;
	case VK_DECIMAL: 	oss << "NumPad."; break;
	case VK_DIVIDE:		oss << "NumPad/"; break;
	default:
		if(nChar>='A'&&nChar<='Z') oss << (char)nChar;
		else if(nChar>='0'&&nChar<='9') oss << (char)nChar;
		else oss<<"#"<<nChar;
	}
	return oss.str();
}
string printKeys(int key, int maxlen){
	int found = 0;
	string out;
	for(int i=0; i<256; i++){
		if(key_map[i]==key){
			if(found)
				out += StringTable(90);
			found ++;
			out += printKey(i);
		}
	}
	if(found>=7)
		found++;
	return out;
}
void CloseSetKeyMsgbox(){
	if(old_lock_msgbox) delete old_lock_msgbox;
	old_lock_msgbox = lock_msgbox;
	lock_msgbox = NULL;
	menu_configlb_RefreshDetail();
	RefreshConfigDetail();
}
void SetKeyHandler( UINT nChar, bool bKeyDown ){
	if( nChar == VK_ESCAPE ){					// quit menu
		if(bKeyDown){
			lock_msgbox->p_OnMenuSelectBegin = pausedMenu_OnSelectedIndexChanged;
			lock_msgbox->OnKeyEvent(KeyEnter,true);
			return;
		}
	}else if( nChar == VK_BACK ){				// back to previous key
		if(bKeyDown)
			if(lock_msgbox->selectedIndex>0)
				lock_msgbox->OnKeyEvent(VK_UP, true);
	}else if( nChar == VK_RETURN ){				// save key map
		if(bKeyDown){
			lock_msgbox->p_OnMenuSelectBegin = pausedMenu_OnMenuSelectBegin;
			systemIni.SaveKeyMap("keyboard.ini");
			lock_msgbox->OnKeyEvent(KeyEnter,true);
		}
	}else if(lock_msgbox->selectedIndex<8){		// normal key
		if(bKeyDown){
			int x=0, index = lock_msgbox->selectedIndex, unit_id=7-index;
			for(int i=0; i<256; i++)
				if(Keys[i]) x++;
			if(x==1){	// pressing down the 1st key, clear all keys associated with current unit
				for(int i=0; i<256; i++)
					if(key_map[i]==unit_id)
						key_map[i] = -1;
			}
			key_map[nChar] = unit_id;
			lock_msgbox->_text[index]=g_button_symbols[index]+" : "+printKeys(unit_id,40);
		}else{
			int x;
			for(x=0; x<256; x++)
				if(Keys[x]) break;
			if(x==256)	// if all keys are released
				lock_msgbox->OnKeyEvent(VK_DOWN, true);
		}
	}
	if(lock_msgbox)
		lock_msgbox->_option[8]=(lock_msgbox->selectedIndex==8)?86:85;
}
void SystemIni::EnterSetKey(int current_keymap_id, bool bSave)
{
	if(current_keymap_id!=key_mapsN)
		return;
	soundEngine->PlaySound("gamedata\\Select.wav",systemIni.sndVolume);
	GUIMessageBox *set_key_menu = new GUIMessageBox();
	set_key_menu->PrepareMenu(84,400);
	for(int x=0; x<8; x++)
		set_key_menu->AddItem(g_button_symbols[x]+" : "+printKeys(7-x,20));
	set_key_menu->AddItem(85);
	set_key_menu->KeyHandler = SetKeyHandler;
	set_key_menu->p_OnMenuSelectOver = NULL;
	set_key_menu->p_OnMenuReleaseOver = CloseSetKeyMsgbox;
	set_key_menu->p_OnSelectedIndexChanged = pausedMenu_OnSelectedIndexChanged;
	set_key_menu->p_OnMenuSelectBegin = pausedMenu_OnMenuSelectBegin;
	set_key_menu->ShowMenu();
	for(int x=0; x<256; x++)
		Keys[x] = false;

	lock_msgbox = set_key_menu;
}
void SystemIni::SaveIniFile()
{
	ofstream fout("config.ini");
	if(fout){
		fout << "[CONFIG]" << endl;
		for( int x=0; x<optionsN; x++ ){
			if(options[x].option_name=="Language")
				fout << options[x].option_name << " = " << locales[language] << endl;
			else if(options[x].option_name=="Font")
				fout << options[x].option_name << " = " << fonts_utf8[fontIndex] << endl;
			else if(options[x].pValue!=NULL)
				fout << options[x].option_name << " = " << *options[x].pValue << endl;
		}
		if(SongInfoManager::songPath != DEFAULT_SONG_PATH)
			fout << "songPath = " << SongInfoManager::songPath << endl;
		fout.close();
	}
}


#include <fstream>
#include <sstream>

void SystemIni::SetDefaultValue(){
	for( int x=0; x<optionsN; x++ )
		if(options[x].pValue!=NULL)
			*options[x].pValue = options[x].default_value;
}

void SystemIni::ApplyKeyMap(char *key_map, BYTE *km){
	memset(key_map,-1,256);
	for(int x=0; x<8; x++)
		key_map[km[x]] = x;
}

void SystemIni::LoadKeyMap(char *filename){
	ifstream fkb(filename);
	if(fkb){
		for(int x=0; x<256; x++){
			int i;
			if(!(fkb>>i)) break;
			custom_key_map[x] = i;
		}
		if(!fkb){	// invalid keymap file, load default key map 0
			fkb.close();
			ApplyKeyMap(custom_key_map, default_key_maps[0]);
		}
	}else
		ApplyKeyMap(custom_key_map, default_key_maps[0]);
}

void SystemIni::SaveKeyMap(char *filename){
	ofstream fout(filename);
	if(fout){
		for(int x=0; x<256; x++)
			fout << (int)custom_key_map[x] << endl;
		fout.close();
	}
}

bool SystemIni::LoadIniFile()
{
	LoadKeyMap("keyboard.ini");
	ifstream fin("config.ini");
	int n_options_set = 0, n_options_need = 0;
	if(fin){
		// build option name to Option map
		map <string,Option*> option_map;
		for(int x=0; x<optionsN; x++)
			if(options[x].pValue!=NULL)
			{
				option_map[options[x].option_name] = &options[x];
				n_options_need++;
			}

		string line;
		while(getline(fin,line)){
			// read options from file
			istringstream iss(line);
			string _name, _null, _data;
			if(iss>>_name>>_null>>_data)
			{
				int _val;
				if(_name=="Language" && _null[0]=='='){
					language = find(locales.begin(), locales.end(), _data)-locales.begin();
					language = (language>=locales.size()?options[5].default_value:language);
				}else if(_name=="Font" && _null[0]=='='){
					auto its = split(line, " ");
					string font_name = join(vector<string>(its.begin()+2, its.end()), " ");
					fontIndex = find(fonts_utf8.begin(), fonts_utf8.end(), font_name)-fonts_utf8.begin();
					fontIndex = (fontIndex>=fonts.size()?options[12].default_value:fontIndex);
				}else if(_name=="songPath" && _null[0]=='='){
					string path = line.substr(line.find("=")+1);
					while(path.length()>0){
						if(path[0]==' ') path=path.substr(1);
						else break;
					}
					SongInfoManager::songPath = path;
					SongInfoManager::currPath = path;
				}else if(istringstream(_data)>>_val){
					Option *option = option_map[_name];
					if(option!=NULL)
					{
						*(option->pValue) = _val;
						n_options_set ++;
					}
				}
			}
		}
	}

	// Find window size type, support custom window size
	if( winSizeType <= g_windowSizesN ){
		winWidth = g_windowSizes[winSizeType][0];
		winHeight = g_windowSizes[winSizeType][1];
	}
	windowed = (winSizeType!=g_windowSizesN);

	return (n_options_set>=n_options_need);
}


string ToString(int num)
{
	char buffer[20];
	itoa(num,buffer,10);
	return string(buffer);
}

string ToString(double num,const char *_Format)
{
	char buffer[20];
	sprintf(buffer,_Format,num);
	return string(buffer);
}


Point Bezier(const Point &p0, const Point &p1, const Point &p2, const float &t)
{
	float tt = 1-t;
	return p0*tt*tt+p1*2*t*tt+p2*t*t;
}

Point Bezier(const Point &p0, const Point &p1, const Point &p2, const Point &p3, const float &t)
{
	float tt = 1-t;
	return p0*tt*tt*tt+p1*3*t*tt*tt+p2*3*t*t*tt+p3*t*t*t;
}

Point BezierDir(const Point &p0, const Point &p1, const Point &p2, const Point &p3, const float &t)
{
	float tt = 1-t;
	return (p1-p0)*tt*tt*3 + (p2-p1)*t*tt*6 + (p3-p2)*t*t*3;
}

void to_integer_point(double &val)
{
	LONGLONG val_i = val+0.5;
	if(abs(val-val_i)<1e-4)
		val = val_i;
}

void to_integer_point(float &val)
{
	LONGLONG val_i = val+0.5;
	if(abs(val-val_i)<1e-4f)
		val = val_i;
}


