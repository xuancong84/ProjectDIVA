#ifndef __GameManaH__
#define __GameManaH__

#include <queue>
#include "defines.h"
#include "notemap.h"
#include "GameIni.h"
#include "GraphEngine.h"
#include "VideoEngine.h"
#include "Gui.h"

//#define GAMEDATA GameData::Instance()

enum PLAYSTATE{SAFE,COOL,FINE,SAD,_COOL,_FINE,WORST,NOSTATE};
enum UISTATE{ADD,DEC,NORMAL};

class GameData
{
	static const int TYPENUM = 8;
	ImageResource *pointer_rc;
	ImageResource *note_rc8;
	ImageResource *rhythm_rc8;
	ImageResource *note_strip_rc8;
	ImageResource *rhythm_strip_rc8;
	ImageResource *bar_in_rc8;
	ImageResource *bar_out_rc;
	ImageResource *presh_rc8;

	//RECT state_rect[7];
	Texture *background;
	friend class Unit;
	friend class NoteMana;
	friend class GameCore;
	friend class GameUI;
	int score, maxScoreCanGet, maxScoreTempCombo, maxScoreTempChanceTimeCombo;
	int cool_cnt,fine_cnt,safe_cnt,sad_cnt,worst_cnt,maxcombo;
	int press_cnt[PRESS_STATE_MAX];
	int _chanceTimeScore,_chanceTimeCombo; 
	bool chanceTime;

public:
	int gamemode, combo, combo_draw, hp;

	//static GameData& Instance(void) {static GameData instance; return instance;}
	GameData() {}
	~GameData() {}
	void Init();
	void SetNew();
	void Release();
	void ModifyHP(int delta);
};
// Frequently accessed class use global definition
extern GameData GAMEDATA;

class Unit
{
	int serial;
	//static const int STRIP_DELAY = 1;
	int notePos;
	bool preshed,first,rippled;
	UNIT type;
	double angle;
	Point note;
	Point rhythm;
	Point bezier_p1,bezier_p2,lastPoint;
	int key;
	double restTime;
	double delayTime;
	double totalTime;
	double speed_factor;
	//double _a,_b,_c;
	friend class NoteMana;
	double endtime;
	//STRIP
	bool start;
	float length_drawn;
	double totalduration,duration;
	NOTETYPE noteType;

public:
	Unit():type(UNIT_CIRCLE),noteType(NOTETYPE_NORMAL),restTime(0),angle(0),preshed(false),rippled(false) {}
	Unit(Note _note, double standing, int _notePos, double speed_factor);
	Unit(Note _note, double standing, int _notePos, double duration, double speed_factor);
	void Update(double dwTime);
	void Draw_Normal();
	void Draw_Strip(DWORD drawMask);
	bool finish();
	void Draw_Bezier1(const Point &p0, const Point &p1, const Point &p2, const Point &p3, float l = 0, float r = 1, int flag = 0);
	void Draw_Bezier2(const Point &p0, const Point &p1, const Point &p2, const Point &p3, float l = 0, float r = 1, int flag = 0);
	//void Draw_Frame(const Point &p0, const Point &p1, const Point &p2, D3DCOLOR color = D3DXCOLOR(255,255,255,255));
	void Draw(DWORD drawMask=0x3);
	void Clear();
	void Presh(PLAYSTATE id);
	void DrawNullLine(const Point &a, const Point &b){return;}
	void DrawSingleLine(const Point &a, const Point &b);
	void DrawDoubleLine(const Point &a, const Point &b);
	void DrawCometLine(const Point &a, const Point &b);
	void DrawBar(const Point &a, const Point &b, int flag = 0);
	void AddMetafile(float x, float y, int flag = 0);
	void AddParticle(float x, float y, float size);
	void AddCometParticle(float x, float y, float dx, float dy);
	void AddStarParticle(float x, float y, float size, D3DCOLOR color);
	void AddSpinParticle(short angle);
};

extern void (Unit::*UnitDraw_Bezier)(const Point &p0, const Point &p1, const Point &p2, const Point &p3, float l, float r, int flag);
extern void (Unit::*UnitDrawLine)(const Point &a, const Point &b);

#include "TSvector.hpp"
#include "CircularQueue.hpp"

class NoteMana
{
	CircularQueue <Unit> unit;
	
public:
	double singleTime;
	struct key_event
	{
		LONGLONG dwTime;
		UINT keyInfo;
	};
	CircularQueue <key_event> key_queue;

	LONGLONG nowTime;
	double tempo;
	double speed_factor, tail_speed_factor;
	int speed_factor_int;
	NoteMana():key_queue(CircularQueue<key_event>(63)),unit(CircularQueue<Unit>(63)) {}//
	void SetNew();
	void Init() {}
	void Clear() {unit.clear();}
	//---------------------------------------------
	void Add(int pos);
	void Run(LpFrame _note);
	void Draw(int phase);
	void Update(LONGLONG);
	void Pause(bool state);
	void SetTimePosi(int framePos);
	//---------------------------------------------
	int OnKeyEvent(UINT nChar, bool bKeyDown, LONGLONG dwTime=0);
	int PushKeyEvent(UINT nChar, bool bKeyDown, LONGLONG dwTime);
	//---------------------------------------------
	//void Pop() {if(!unit.empty()) unit.pop();}
	int GetSize() {return unit.size();}
	int GetScore() {return GAMEDATA.score;}
};

//#define gameui GameUI::Instance()
class GameUI
{
	PLAYSTATE state;
	UISTATE uistate;
	double delayTime, uidelay;
	int spark;
	int moveTime, nowTime;
	int moveState; //0 normal 1 in 2 out
	int deltaTop, deltaBottom;
	Texture *ui;
	ImageResource *top_bar_rc, *bottom_bar_rc;
	ImageResource *top_bar_red_rc, *bottom_bar_red_rc;
	ImageResource *rhythm_logo_rc, *hp_gauge_rc, *hp_rhyme_rc, *hp_UPs_rc, *hp_ExQn_rc, *hp_Ex_rc, *combo_gauge_rc, *combo_shift_rc;
	ImageResource *progress_bar_front_rc, *progress_bar_back_rc;
	ImageResource *progress_bar_pointer_rc;
	ImageResource *state0_rc,*state1_rc,*state2_rc,*state3_rc,*state4_rc,*state5_rc,*state6_rc;
	ImageResource *press_state[7];

	friend class GameCore;

public:
	//static GameUI& Instance(void) {static GameUI instance; return instance;}
	GUIStaticImage *combo_star;
	GameUI() {combo_star=NULL;}
	~GameUI() {}
	void SetComboStar();
	void ClearComboStar();
	void Init();
	void Draw(int game_mode);
	void Update(double dwTime);
	void SetNew();
	void SetState(PLAYSTATE _state);
	void SetUIState(UISTATE _state);
	void MoveAway(int time);
	void MoveIn(int time);
};

extern GameUI gameui;

class GameCore
{
	double noteTime;
	double prev_seektrack_percent;
	LONGLONG totalTime;
	LpFrame nowFrame;
	bool finish;
	bool visible;
	bool ifclear;
	string calib_filename;

public:
	GUIMessageBox *menu;
	RESOURCE resource;
	NoteMana _notemana;
	LONGLONG timeoffset;
	map <int,double> map_calibration;
	bool cheatDie;
	bool paused;
	GameCore() {}
	~GameCore();
	void Init();
	void SetNew(string filename,string baseDirec,int gamemode = GAME_MODE_NORMAL);
	void SetPausedMenu(GUIMessageBox *_menu) {menu = _menu;}
	void LoadCalibration(string filename);
	void SaveCalibration();
	void ChangeTimeOffsets(double deltaTimeSecond, string sym);
	void ChangeMusicVolume(double deltaRatio);
	void ChangePlaySpeed(double deltaRatio, int option=0);
	void ChangePitch(double val);
	void ChangeTailSpeed(double deltaRatio);
	//--------------------------------
	void Run(LpFrame _nowFrame, int mask=0xffffffff);
	void Update(LONGLONG dwTime);
	void Draw(IDirect3DDevice9 *Device);
	void Finish();
	void Clear();
	void SetTimePosi(float percent);	// seek to position, 0<percent<1
	//--------------------------------
	void OnKeyEvent(UINT nChar, bool bKeyDown);
	//--------------------------------
	void PauseGame(bool _paused);
	bool IsFinish() {return (nowFrame==NULL);}
	int GetScore() {return _notemana.GetScore();}
	int GetNoteSize() {return _notemana.GetSize();}
	int GetCombo() {return GAMEDATA.combo;}
};
extern GameCore core;
extern int *UNIT_COLOR[8], *UNIT_BGCOLOR;

#endif

