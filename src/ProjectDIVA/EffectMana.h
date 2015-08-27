#pragma once

#include <vector>
#include "Base.h"
#include "GameMana.h"
#include "PSystem.h"
#include "resource.hpp"

using namespace std;

struct Effect_params
{
	float start_frame, stop_frame;
	float *size_table, *alpha_table;
};

class Effect_Base
{
protected:
	float nowframe;
	int desframe;
	int _x,_y;
	ImageResource* _image_rc;
	Texture *_drawTex;

public:
	Effect_Base(int _nowframe,int _desframe,int x,int y, ImageResource* image_rc)
		:nowframe(_nowframe),desframe(_desframe),_x(x),_y(y),_image_rc(image_rc){}

	Effect_Base(int _nowframe,int _desframe,int x,int y, Texture* image_rc)
		:nowframe(_nowframe),desframe(_desframe),_x(x),_y(y),_drawTex(image_rc){}

	virtual void Draw(){}
	virtual bool Update(float){return false;}		// return false means die
	virtual bool Die(){return nowframe>=desframe;}
};

class Effect_NoteIn : public Effect_Base
{
public:
	Effect_NoteIn(int x,int y,ImageResource* rc):Effect_Base(0,30,x,y,rc){}

	void Draw(){}
	bool Update(float){return false;}
};

class Effect_SingleFrame : public Effect_Base
{
private:
	float *size_table, *alpha_table;

public:
	DWORD color;
	Effect_SingleFrame(int x,int y,Effect_params &params,ImageResource* rc)
		:Effect_Base(params.start_frame, params.stop_frame,x,y,rc)
	{
		color = rc->color;
		size_table = params.size_table;
		alpha_table = params.alpha_table;
	}

	void Draw();
	bool Update(float);
};

class Effect_MultipleFrame : public Effect_Base
{
public:
	Effect_MultipleFrame(int x,int y,int nFrames,ImageResource* rc,int startFrame=0)
		:Effect_Base(startFrame,nFrames,x,y,rc){}

	void Draw();
	bool Update(float);
};


class Effect_Combo : public Effect_Base
{
	
public:
	int _combo;
	Effect_Combo(int x,int y,int combo,Texture* drawTex):Effect_Base(0,40,x,y,drawTex),_combo(combo){}

	void Draw();
	bool Update(float);
};

class Effect_ExtraScore : public Effect_Base
{
	int _extraScore;
public:
	Effect_ExtraScore(int x,int y,int extraScore,Texture* drawTex):Effect_Base(0,40,x,y,drawTex),_extraScore(extraScore){}

	void Draw();
	bool Update(float);
};

class Effect_ChanceTimeEnd : public Effect_Base
{
	int _chanceTimeScore;
public:
	Effect_ChanceTimeEnd(int chanceTimeScore,Texture* drawTex):Effect_Base(0,180,0,0,drawTex),_chanceTimeScore(chanceTimeScore){}

	void Draw();
	bool Update(float);
};

class Effect_ChanceTimeStart : public Effect_Base
{
public:
	Effect_ChanceTimeStart(Texture* drawTex):Effect_Base(0,180,0,0,drawTex){}

	void Draw();
	bool Update(float);
};

enum NumberStyle{
	SCORE,
	COMBO,
	EXTRASCORE,
	CHANCE
};

enum NumberDwFlags{
	DWFLAG_LEFT,
	DWFLAG_RIGHT,
	MID
};


class NumberMana
{
	Effect_ExtraScore *extraScoreDraw;
	Texture			*numTex;
	D3DCOLOR		score_color;
	ImageResource	*score_dash;
	RECT			score_rect;
	int				score_inc, score_fill;
	
public:
	Effect_Combo	*comboDraw;
	int				_nowDrawScore,_nowScore;
	int				score_state;	// 0: normal, 1: seek-tracked, 2: speed-changed

	NumberMana(){}
	~NumberMana(){}

	void Init();
	void Clear();
	void SetNew();
	void Draw(int number, const RECT &drawRect,NumberStyle style,NumberDwFlags flag,D3DCOLOR color,int inc,int fill=0);
	void DrawScore(int score);
	void DrawCombo();
	
	void Update(float dwTimeMilliSecond);

	void SetCombo(int combo,int x,int y);
	void SetExtraScore(int extraScore,int x,int y);
};
extern NumberMana numberMana;
//#define numberMana NumberMana::Instance()

class EffectMana
{
	ImageResource *effect_note_in_rc;
	ImageResource *effect_note_press_burst1;
	ImageResource *effect_note_press_burst2;
	ImageResource *effect_note_press_burst_cool;
	ImageResource *effect_note_press_rhyme1;
	ImageResource *effect_note_press_rhyme2;
	ImageResource *effect_note_press_aura;

	FloatResource *effect_aura_params;
	FloatResource *effect_rhyme_params;
	FloatResource *effect_ripple_params;

public:
	Effect_params aura_param, rhyme_param, ripple_param;

	vector<Effect_Base*> effects;
	Texture *numTex;
	
public:
	EffectMana(){}
	~EffectMana(){Clear();}

	//static EffectMana& Instance() {static EffectMana instance;return instance;}
	
	void Init();
	void SetNew();

	void Clear();

	void AddEffectNoteIn(int x,int y,int *color=NULL);
	void AddEffectNotePress(int x,int y,PLAYSTATE id);
	void AddEffectChanceTimeStart(){effects.push_back((Effect_Base*)(new Effect_ChanceTimeStart(numTex)));}
	void AddEffectChanceTimeEnd(int chanceTimeScore){effects.push_back((Effect_Base*)(new Effect_ChanceTimeEnd(chanceTimeScore,numTex)));}

	void Draw();

	void DrawCombo(int combo);

	void Update(float dwTimeMilliSecond);

};

extern EffectMana effectMana;
//#define effectMana effectMana
