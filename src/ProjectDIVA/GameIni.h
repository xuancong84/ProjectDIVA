#ifndef GAMEINI_H
#define GAMEINI_H

#include "base.h"
#include "GraphEngine.h"
#include "SoundEngine.h"


enum PRESS_STATE{PRESS_COOL, PRESS_FINE, PRESS_SAFE, PRESS_SAD, PRESS_WORST, PRESS_STATE_MAX};
enum GAME_LEVEL{LEVEL_PRACTICE, LEVEL_NORMAL, LEVEL_CHALLENGE, LEVEL_DEATHMATCH, GAME_LEVEL_MAX};

class GameIni
{
public:
	GameIni(void){}
	~GameIni(void) {}
	double delay,playstate_delay,uistate_delay;
	static const int note_standing = 192;
	static const int ORIGIN_X = 25, ORIGIN_Y = 52;
	static const int DELTA_X = 12, DELTA_Y = 12;
	static const int DISTANCE = 500;
	static const int BPM = 120;
	double COOL, FINE, SAFE, SAD;
	static const int SCORE_COOL = 400, SCORE_FINE = 300, SCORE_SAFE = 200, SCORE_SAD = 100;
	static const int HP = 100;
	static const int SINGLE_HP = 6;
	static const int HP_DEC = -2, HP_ADD = 1, HP_SAFE = 0, HP_SAD = -1;
	static const int HP_CHANGE[GAME_LEVEL_MAX][PRESS_STATE_MAX];
	static const double NOTE_BLOWUP;
	double eps;

	void Init();

	//static GameIni& Instance(void) {static GameIni instance; return instance;}
};
extern GameIni gameini;

extern void test();
#endif