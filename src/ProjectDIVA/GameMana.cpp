#include <Math.h>
#include "GameMana.h"
#include "EffectMana.h"
#include "Gui.h"
#include "keyboard.h"
#include "UIScreen.h"
#include "VideoEngine.h"

#define STARLEVEL 100
GameUI gameui;

float SINGLE_LINE_INC;
float DOUBLE_LINE_INC;
float DOUBLE_LINE_TWIST_FACTOR;
float DOUBLE_LINE_SPACE_FACTOR;
float COMET_LINE_INC;
float STRIP_NOTE_SIZE;
float NOTE_SIZE;
float META_SIZE;
float ARROW_HEAD;

int nowDistance;
int *spin_angle_inc;
GUIStaticImage *white;
GUIStaticImage *black_top,*black_bottom,*chanceTimeLabel;
void (Unit::*UnitDrawLine)(const Point &a, const Point &b);
void (Unit::*UnitDraw_Bezier)(const Point &p0, const Point &p1, const Point &p2, const Point &p3, float l, float r, int flag);
GameData GAMEDATA;
int *UNIT_COLOR[8], *UNIT_BGCOLOR;

void GameData::Init()
{
	//note_pic = graphEngine.AddTexture("pic\\note.png");
	background = NULL;

	// load notes_new.png
	struct toLoad{
		Resource** ppRes;
		string res_name;
	};
		
	static toLoad toloads[]={
		{(Resource**)&pointer_rc,		"note_pointer"			},
		{(Resource**)&note_rc8,			"note_rect[8]"			},
		{(Resource**)&rhythm_rc8,		"rhythm_rect[8]"		},
		{(Resource**)&note_strip_rc8,	"note_rect_strip[8]"	},
		{(Resource**)&rhythm_strip_rc8,	"rhythm_rect_strip[8]"	},
		{(Resource**)&bar_in_rc8,		"bar_in[8]"				},
		{(Resource**)&bar_out_rc,		"bar_out"				},
		{(Resource**)&presh_rc8,		"presh_rect[8]"			}
	};
	for(int x=0, X=sizeof(toloads)/sizeof(toLoad); x<X; x++)
	{
		*toloads[x].ppRes = g_res.getResource(toloads[x].res_name);
	}

	// Load tail color
	static string tail_colors[]={
		"tail_color_CIRCLE[4]"	,
		"tail_color_RECT[4]"	,
		"tail_color_CROSS[4]"	,
		"tail_color_TRIANGLE[4]",
		"tail_color_RIGHT[4]"	,
		"tail_color_LEFT[4]"	,
		"tail_color_DOWN[4]"	,
		"tail_color_UP[4]"		,
	};
	for(int x=0; x<8; x++)
		UNIT_COLOR[x] = ((IntResource*)g_res.getResource(tail_colors[x]))->value;
	
	UNIT_BGCOLOR = ((IntResource*)g_res.getResource("tail_color_background[4]"))->value;

	// ui.png
	/*
	state_rect[0] = MakeRect(440,120,65,19);
	state_rect[1] = MakeRect(301,120,65,19);
	state_rect[2] = MakeRect(376,120,65,19);
	state_rect[3] = MakeRect(440,139,47,19);
	state_rect[4] = MakeRect(301,139,65,19);
	state_rect[5] = MakeRect(376,139,65,19);
	state_rect[6] = MakeRect(301,158,79,18);*/
	hp = gameini.HP/2, score = 0, combo_draw = combo = 0;
	cool_cnt = fine_cnt = safe_cnt = sad_cnt = worst_cnt = maxcombo = 0;
	memset(press_cnt, 0, sizeof(press_cnt));
	_chanceTimeScore = _chanceTimeCombo = chanceTime = 0;
	maxScoreCanGet = maxScoreTempCombo = maxScoreTempChanceTimeCombo = 0;
}

void GameData::SetNew()
{
	hp = gameini.HP/2, score = 0, combo_draw = combo = 0;
	cool_cnt = fine_cnt = safe_cnt = sad_cnt = worst_cnt = maxcombo = 0;
	memset(press_cnt, 0, sizeof(press_cnt));
	_chanceTimeScore = _chanceTimeCombo = chanceTime = 0;
	maxScoreCanGet = maxScoreTempCombo = maxScoreTempChanceTimeCombo = 0;
	background = NULL;//graphEngine.AddTexture("pic\\black.png");

}

void GameData::ModifyHP(int delta)
{
	if(delta>0&&hp<gameini.HP)
	{
		hp = min(hp+delta,gameini.HP);
		gameui.SetUIState(ADD);
	}
	if(delta<0&&hp>0)
	{
		hp = max(hp+delta,0);
		gameui.SetUIState(DEC);
	}
}

//static int ser=0;
Unit::Unit(Note _note,double standing, int _notePos, double _speed_factor)
{
//	serial = ser++;
	delayTime = 0;
	noteType = NOTETYPE_NORMAL;
	note = Point(gameini.ORIGIN_X+_note._x*gameini.DELTA_X+12,gameini.ORIGIN_Y+_note._y*gameini.DELTA_Y+12);
	rhythm = Point(_note._tailx,_note._taily);
	type = (UNIT)_note._type;
	speed_factor = _speed_factor;
	restTime = standing+gameini.delay*_speed_factor;
	totalTime = standing;
	notePos = _notePos;
	key = _note._key;
	angle = 0, preshed = false, first = true, rippled = false;
	//---------------------------bezier------------------------------
	/*while(true)
	{
	double x = rand()%gameini.DISTANCE+1, y = rand()%gameini.DISTANCE+1;
	double p1 = x*note.x*(x-note.x), p2 = note.x-x, p3 = y*note.x-note.y*x;
	double p4 = x*rhythm.x*(x-rhythm.x), p5 = rhythm.x-x, p6 = y*rhythm.x-rhythm.y*x;
	if(fabs(p1*p4)<EPS||fabs(p2*p4-p1*p5)<EPS)
	continue;
	_c = (p3*p4-p1*p6)/(p2*p4-p1*p5);
	_a = (p3*p4-_c*p2*p4)/(p1*p4);
	_b = (y-_c-_a*x*x)/x;
	break;
	}*/
}

Unit::Unit(Note _note, double standing, int _notePos, double _duration, double speed_factor)
{
	delayTime = 0, totalduration = _duration, duration = _duration+gameini.delay;
	noteType = NOTETYPE_STRIP;
	note = Point(gameini.ORIGIN_X+_note._x*gameini.DELTA_X+12,gameini.ORIGIN_Y+_note._y*gameini.DELTA_Y+12);
	rhythm = Point(_note._tailx,_note._taily);
	type = (UNIT)(_note._type-NOTENum);
	restTime = standing+gameini.delay*speed_factor;
	totalTime = standing;
	notePos = _notePos;
	key = _note._key;
	angle = 0, preshed = false, start = false, first = true, rippled = false;
}

bool Unit::finish()
{
	if(noteType==NOTETYPE_NORMAL) 
		return restTime<EPS; 
	else if(start&&fabs(duration)<EPS) 
		return true;
	else if(!start&&fabs(restTime)<EPS)
		return true;
	else
		return false;
}

void Unit::Update(double dwTime)
{
	//dwTime *= core._notemana.speed_factor;
	if(!preshed)
	{
		if(noteType==NOTETYPE_NORMAL)
		{
			restTime -= dwTime;
			if(restTime<0) restTime = 0;
		}
		else
		{
			restTime -= dwTime;
			if(restTime<gameini.delay)
				duration -= dwTime;
			if(restTime<0) restTime = 0;
			if(duration<0) duration = 0;
		}
		if(noteType==NOTETYPE_NORMAL)
		{
			double percent = (restTime-gameini.delay)<0?0:(restTime-gameini.delay)/totalTime;
			angle = -(PI+PI)*percent;
		}
		else
		{
			double percent = (((duration-gameini.delay)<0?0:(duration-gameini.delay))+((restTime-gameini.delay)<0?0:(restTime-gameini.delay)))/(totalTime+totalduration);
			angle = -(PI+PI)*percent;
		}
	}
	else
	{
		delayTime -= dwTime;
		if(delayTime<0)
			delayTime = 0;
	}
	if(!rippled)
	{
		if(restTime-gameini.delay<totalTime*0.875)
		{
			effectMana.AddEffectNoteIn(note.x,note.y);
			rippled = true;
		}
	}
}

void Unit::AddMetafile(float x, float y, int flag)
{
	RECT rect;
	if(flag==0)
	{
		graphEngine.Draw(GAMEDATA.bar_out_rc,x-8.0f,y-8.0f);
	}
	else if(flag==1)
	{
		graphEngine.Draw(&GAMEDATA.bar_in_rc8[type],x-7.3f,y-6.5f);
	}
}

void Unit::AddParticle(float x, float y, float size)
{
	pSystem.AddParticle(
		new ParticleTail(x,y,128,UNIT_COLOR[type],particle_square,size),
		(notePos<<4)+(type<<1)
	);
}

void Unit::AddCometParticle(float x, float y, float dx, float dy)
{
	// Random color interpolated with Unit color
	int rgb[3]={rand()&0xff,rand()&0xff,rand()&0xff}, *unit_color = UNIT_COLOR[type];
	//if(serial&1){
	if(0){
		int maxv = max(rgb[0],rgb[1]);
		maxv = max(maxv,rgb[2]);
		float r = (float)256/(maxv+1);
		rgb[0] = (rgb[0]+1)*r;
		rgb[1] = (rgb[1]+1)*r;
		rgb[2] = (rgb[2]+1)*r;
	}
	rgb[0] = (rgb[0]+unit_color[0])>>1;
	rgb[1] = (rgb[1]+unit_color[1])>>1;
	rgb[2] = (rgb[2]+unit_color[2])>>1;

	// Adjust disperse density
	SetLength(dx,dy,sqrt((float)rand()/RAND_MAX));
	//SetLength(dx,dy,(float)rand()/RAND_MAX);

	// Add particle
	//if(serial&1)
	if(GAMEDATA.chanceTime)
		pSystem.AddParticle(
			(new ParticleComet(	x,y,255,rgb,particle_star2,dx,dy,(float)rand()*(7.0f/RAND_MAX),type,0x600f,GAMEDATA.hp,0,0)),
			(notePos<<4)+(type<<1)+1
		);
	else
		pSystem.AddParticle(
			new ParticleComet(	x,y,255,rgb,particle_sphere,dx,dy,(float)rand()*(7.5f/RAND_MAX),type,0x200f,GAMEDATA.hp,0,0),
			(notePos<<4)+(type<<1)+1
		);
}

void Unit::AddStarParticle(float x, float y, float size, D3DCOLOR color)
{
	// Random color interpolated with Unit color
	int rgb[3]={rand()&0xff,rand()&0xff,rand()&0xff}, *unit_color = UNIT_COLOR[type];
	rgb[0] = (rgb[0]+unit_color[0])>>1;
	rgb[1] = (rgb[1]+unit_color[1])>>1;
	rgb[2] = (rgb[2]+unit_color[2])>>1;

	// Add particle
	pSystem.AddParticle(
		new ParticleComet(x,y,255,rgb,particle_star,0,0,size,type,(GAMEDATA.chanceTime)?0x2007:0x2002,GAMEDATA.hp,0,0),
		(notePos<<4)+(type<<1)+1
	);
}

void Unit::AddSpinParticle(short angle)
{
	pSystem.AddParticle(
		new ParticleComet(0,0,255,UNIT_COLOR[type],particle_5star,note.x,note.y,10,type,0x0079,GAMEDATA.hp,0,angle),
		(notePos<<4)+(type<<1)
	);
}

void Unit::DrawSingleLine(const Point &a, const Point &b)
{
	float	move_dir_x=b.x-a.x, move_dir_y=b.y-a.y;
	float	D = sqrt(move_dir_x*move_dir_x+move_dir_y*move_dir_y);
	if(D==0) return;
	float	x = a.x, y = a.y;
	float	x_inc = move_dir_x*SINGLE_LINE_INC/D;
	float	y_inc = move_dir_y*SINGLE_LINE_INC/D;

	for(int i=D/SINGLE_LINE_INC; i>=0; i--)
	{
		if(InsideDrawRange(x,y))
			AddParticle(x,y,4);
		x += x_inc;
		y += y_inc;
	}
}

void Unit::DrawDoubleLine(const Point &a, const Point &b)
{
	float	move_dir_x=b.x-a.x, move_dir_y=b.y-a.y;
	float	D = UnitLength(move_dir_x,move_dir_y);
	if(D==0) return;

	float	x = a.x, y = a.y;
	float	x_inc = move_dir_x*DOUBLE_LINE_INC;
	float	y_inc = move_dir_y*DOUBLE_LINE_INC;
	float	vert_dir_x = move_dir_y;
	float	vert_dir_y = -move_dir_x;
	float	_length_posi = length_drawn;

	for(int i=D/DOUBLE_LINE_INC; i>=0; i--)
	{
		float X,Y;
		{// draw 1st point
			X = x + vert_dir_x*DOUBLE_LINE_SPACE_FACTOR*cos(_length_posi*DOUBLE_LINE_TWIST_FACTOR);
			Y = y + vert_dir_y*DOUBLE_LINE_SPACE_FACTOR*sin(_length_posi*DOUBLE_LINE_TWIST_FACTOR);
			if(InsideDrawRange(X,Y))
				AddParticle(X,Y,1.6f);
		}
		{// draw 2nd point
			X = x - vert_dir_x*DOUBLE_LINE_SPACE_FACTOR*cos(_length_posi*DOUBLE_LINE_TWIST_FACTOR);
			Y = y - vert_dir_y*DOUBLE_LINE_SPACE_FACTOR*sin(_length_posi*DOUBLE_LINE_TWIST_FACTOR);
			if(InsideDrawRange(X,Y))
				AddParticle(X,Y,1.6f);
		}
		x += x_inc;
		y += y_inc;
		_length_posi += DOUBLE_LINE_INC;
	}
	length_drawn += D;
}

void Unit::DrawCometLine(const Point &a, const Point &b)
{
	float	move_dir_x=b.x-a.x, move_dir_y=b.y-a.y;
	float	D = UnitLength(move_dir_x,move_dir_y);
	if(D==0) return;

	float	x = a.x, y = a.y;
	float	x_inc = move_dir_x*COMET_LINE_INC;
	float	y_inc = move_dir_y*COMET_LINE_INC;

	for(int i=D/COMET_LINE_INC; i>=0; i--)
	{
		float	x_dir, y_dir;
		RandomDir(x_dir,y_dir);
		if(InsideDrawRange(x,y))
			AddCometParticle(x,y,x_dir-move_dir_x,y_dir-move_dir_y);
		x += x_inc;
		y += y_inc;
	}
}

void Unit::DrawBar(const Point &a, const Point &b, int flag)
{
	float	move_dir_x=b.x-a.x, move_dir_y=b.y-a.y;
	float	D = sqrt(move_dir_x*move_dir_x+move_dir_y*move_dir_y);
	if(D==0) return;
	float	x = a.x, y = a.y;
	float	x_inc = move_dir_x*SINGLE_LINE_INC/D;
	float	y_inc = move_dir_y*SINGLE_LINE_INC/D;

	for(float  i = 0; i < D; i+=SINGLE_LINE_INC)
	{
		//if(InsideDrawRangeEx(x,y))
			AddMetafile(x,y,flag);
		x += x_inc;
		y += y_inc;
	}
}

void Unit::Draw_Normal()
{
	double realTime = (restTime-gameini.delay)<0?0:(restTime-gameini.delay);

	//draw stationary note
	D3DXMATRIX mat,mat2,ptransform;
	base::Sprite->GetTransform(&ptransform);
	D3DXMatrixTransformation2D(&mat, NULL, 0.0f, NULL, &base::MakeDrawRect(D3DXVECTOR2(note.x,note.y)), angle, NULL);
	if(realTime<totalTime*0.875)	// normal size
	{
		graphEngine.Draw(	&GAMEDATA.note_rc8[type],
							MakefRect(note.x-NOTE_SIZE*0.5f,note.y-NOTE_SIZE*0.5f,NOTE_SIZE,NOTE_SIZE)
							/*,D3DCOLOR_ARGB(int((restTime/gameini.standing)*255),255,255,255)*/);
		base::Sprite->SetTransform(&mat);
		graphEngine.Draw(	GAMEDATA.pointer_rc,
							MakefRect(note.x-NOTE_SIZE*0.5f,note.y-NOTE_SIZE*(0.5f+ARROW_HEAD),NOTE_SIZE,NOTE_SIZE));
	}
	else							// just appear, enlarge size
	{
		float percent = 1+(gameini.NOTE_BLOWUP-1)*((realTime-totalTime*0.875)/(totalTime*0.125));
		float note_size = NOTE_SIZE*percent;
		graphEngine.Draw(	&GAMEDATA.note_rc8[type],
			MakefRect(note.x-note_size*0.5f,note.y-note_size*0.5f,note_size,note_size)
			/*,D3DCOLOR_ARGB(int((restTime/gameini.standing)*255),255,255,255)*/);
		base::Sprite->SetTransform(&mat);
		graphEngine.Draw(	GAMEDATA.pointer_rc,
			MakefRect(note.x-note_size*0.5f,note.y-note_size*(0.5f+ARROW_HEAD),note_size,note_size)
			/*,D3DCOLOR_ARGB(int((restTime/gameini.standing)*255),255,255,255)*/);
	}
	base::Sprite->SetTransform(&ptransform);

	//draw rhythm, flying note
	double percent = (restTime-gameini.delay)/totalTime;
	Point _rhythm;

	if(restTime<gameini.delay)	// shrink notes
	{
		_rhythm = note+(rhythm-note).unit()*(nowDistance*2/3);
		bezier_p1 = note+(_rhythm-note)*0.25+(_rhythm-note).normal()*0.15;
		bezier_p2 = note+(_rhythm-note)*0.75-(_rhythm-note).normal()*0.15;
		_rhythm = Bezier(note,bezier_p1,bezier_p2,_rhythm,percent);
		percent = restTime/gameini.delay;
		float note_size = NOTE_SIZE*percent;
		graphEngine.Draw( preshed?&GAMEDATA.presh_rc8[type]:&GAMEDATA.rhythm_rc8[type],
			MakefRect(_rhythm.x-note_size*0.5f,_rhythm.y-note_size*0.5f,note_size,note_size));
	}
	else
	{
		_rhythm = note+(rhythm-note).unit()*(nowDistance);
		bezier_p1 = note+(_rhythm-note)*0.25+(_rhythm-note).normal()*0.15;
		bezier_p2 = note+(_rhythm-note)*0.75-(_rhythm-note).normal()*0.15;
		_rhythm = Bezier(note,bezier_p1,bezier_p2,_rhythm,percent);
		if(InsideDrawRangeEx(_rhythm.x, _rhythm.y))
		{
			graphEngine.Draw( preshed?&GAMEDATA.presh_rc8[type]:&GAMEDATA.rhythm_rc8[type],
				MakefRect(_rhythm.x-NOTE_SIZE*0.5f,_rhythm.y-NOTE_SIZE*0.5f,NOTE_SIZE,NOTE_SIZE));

			// Draw line to connect to last position
			if(first)
			{
				first = false;
				length_drawn = (float)rand()*(6.2831853f/RAND_MAX/DOUBLE_LINE_TWIST_FACTOR);
			}else
				(this->*UnitDrawLine)(lastPoint,_rhythm);

			lastPoint = _rhythm;
		}
	}
}

void Unit::Draw_Strip(DWORD drawMask)
{
	float realTime = (restTime-gameini.delay)<0?0:(restTime-gameini.delay);

	if(drawMask&0x2)
	{
		//draw stationary note
		D3DXMATRIX mat,mat2,ptransform;
		base::Sprite->GetTransform(&ptransform);
		D3DXMatrixTransformation2D(&mat, NULL, 0.0f, NULL, &base::MakeDrawRect(D3DXVECTOR2(note.x,note.y)), angle, NULL);
		if(realTime<totalTime*0.875)	// normal size
		{
			graphEngine.Draw( &GAMEDATA.note_strip_rc8[type],
				MakefRect(note.x-STRIP_NOTE_SIZE*0.5f,note.y-STRIP_NOTE_SIZE*0.5f,STRIP_NOTE_SIZE,STRIP_NOTE_SIZE));
				//,D3DCOLOR_ARGB(int((restTime/gameini.standing)*255),255,255,255)
			base::Sprite->SetTransform(&mat);
			graphEngine.Draw( GAMEDATA.pointer_rc,
				MakefRect(note.x-STRIP_NOTE_SIZE*0.5f,note.y-STRIP_NOTE_SIZE*(0.5f+ARROW_HEAD),STRIP_NOTE_SIZE,STRIP_NOTE_SIZE));
				//,D3DCOLOR_ARGB(int((restTime/gameini.standing)*255),255,255,255)
		}
		else							// just appear, enlarge size
		{
			double percent = 1.0f+(gameini.NOTE_BLOWUP-1.0f)*((realTime-totalTime*0.875)/(totalTime*0.125));
			float note_size = STRIP_NOTE_SIZE*percent;
			graphEngine.Draw( &GAMEDATA.note_strip_rc8[type],
				MakefRect(note.x-note_size*0.5f,note.y-note_size*0.5f,note_size,note_size));
				//,D3DCOLOR_ARGB(int((restTime/gameini.standing)*255),255,255,255)
			base::Sprite->SetTransform(&mat);
			graphEngine.Draw( GAMEDATA.pointer_rc,
				MakefRect(note.x-note_size*0.5f,note.y-note_size*(0.5f+ARROW_HEAD),note_size,note_size));
				//,D3DCOLOR_ARGB(int((restTime/gameini.standing)*255),255,255,255)
		}
		base::Sprite->SetTransform(&ptransform);
	}

	//draw strip
	double percent = realTime/totalTime, realdura = duration-gameini.delay<0?0:(duration-gameini.delay);

	Point _rhythm = note+(rhythm-note).unit()*(nowDistance);
	Point _tail = Bezier(note,bezier_p1,bezier_p2,_rhythm,percent+realdura/totalTime);

	if(drawMask&1)
	{
		(this->*UnitDraw_Bezier)(note,bezier_p1,bezier_p2,_rhythm,percent,percent+realdura/totalTime,0);
		(this->*UnitDraw_Bezier)(note,bezier_p1,bezier_p2,_rhythm,percent,percent+realdura/totalTime,1);
	}

	if((drawMask&2)==0)
		return;

	_rhythm = note+(rhythm-note).unit()*(nowDistance);
	bezier_p1 = note+(_rhythm-note)*0.25+(_rhythm-note).normal()*0.15;
	bezier_p2 = note+(_rhythm-note)*0.75-(_rhythm-note).normal()*0.15;
	_rhythm = Bezier(note,bezier_p1,bezier_p2,_rhythm,percent);
	if(systemIni.antialiasing)
		if(InsideDrawRange(_rhythm.x, _rhythm.y))
		{
			AddMetafile(_rhythm.x,_rhythm.y,0);
			AddMetafile(_rhythm.x,_rhythm.y,1);
		}
	if( fabs(restTime)>EPS && !start )
	{
		// Draw head symbol
		if(InsideDrawRangeEx(_rhythm.x, _rhythm.y))
		{
			if(systemIni.antialiasing)
			{
				AddMetafile(_rhythm.x,_rhythm.y,0);
				AddMetafile(_rhythm.x,_rhythm.y,1);
			}
			graphEngine.Draw(preshed?&GAMEDATA.presh_rc8[type]:&GAMEDATA.rhythm_strip_rc8[type],
				MakefRect( _rhythm.x-STRIP_NOTE_SIZE*0.5f, _rhythm.y-STRIP_NOTE_SIZE*0.5f, STRIP_NOTE_SIZE, STRIP_NOTE_SIZE ) );

			if(systemIni.particle>=3)
			{
				// Add star particle
				if(first)
				{
					first = false;
					length_drawn = 0;
				}
				else
				{
					length_drawn += (_rhythm-lastPoint).mod();
					while(length_drawn>0){
						float rand_size = (float)rand()/RAND_MAX;
						// Chance time got 2 times the number of star particles
						length_drawn -= rand_size*(GAMEDATA.chanceTime)?1.5f:2.0f;
						if(InsideDrawRange(_rhythm.x,_rhythm.y))
							AddStarParticle( _rhythm.x+rand()*(12.0f/RAND_MAX)-6.0f,
							_rhythm.y+rand()*(12.0f/RAND_MAX)-6.0f, rand_size*10, 0 );
					}
				}

				// save current position
				lastPoint = _rhythm;
			}
		}
	}

	// Draw tail symbol
	if(InsideDrawRangeEx(_tail.x, _tail.y))
	{
		if(systemIni.antialiasing)
		{
			AddMetafile(_tail.x,_tail.y,0);
			AddMetafile(_tail.x,_tail.y,1);
		}
		graphEngine.Draw( preshed?&GAMEDATA.presh_rc8[type]:&GAMEDATA.rhythm_strip_rc8[type],
			MakefRect(_tail.x-STRIP_NOTE_SIZE*0.5f,_tail.y-STRIP_NOTE_SIZE*0.5f,STRIP_NOTE_SIZE,STRIP_NOTE_SIZE) );
	}
}

struct VertexXYZC{
	float x,y,z;
	D3DCOLOR color;
};
const DWORD g_FVF = D3DFVF_XYZ|D3DFVF_DIFFUSE;
const int g_stride = 16;
const int TOTAL = 64;
VertexXYZC vbuf[(TOTAL+1)*2], vbuf1[(TOTAL+1)*2], vbuf2[(TOTAL+1)*2];
const float max_dist = sqrtf((float)WIDTH*WIDTH+(float)HEIGHT*HEIGHT);
const float strip_widths[2][3]={{7.25f, 8.25f}, {5.5f, 6.5f}};

float BinarySearchPoint(float l, float r, const Point &p0, const Point &p1, const Point &p2, const Point &p3)
{
	// Assumption: r must be inside DrawRangeEx, l must be outside DrawRangeEx
	float mid = (l+r)*0.5f;
	Point middle = Bezier(p0,p1,p2,p3,mid);
	if(InsideDrawRangeEx(middle.x, middle.y)){
		if(InsideDrawRange(middle.x, middle.y)){
			return BinarySearchPoint(l,mid,p0,p1,p2,p3);
		}else{
			return mid;
		}
	}else{
		return BinarySearchPoint(mid,r,p0,p1,p2,p3);
	}
}

void Unit::Draw_Bezier1(const Point &p0, const Point &p1, const Point &p2, const Point &p3, float l, float r, int flag)
{
	{	// binary search for the 1st point which is inside DrawRangeEx but outside DrawRange
		Point S = Bezier(p0,p1,p2,p3,r);
		Point E = Bezier(p0,p1,p2,p3,l);
		if(!InsideDrawRangeEx(S.x, S.y)){		// S is outside
			if(!InsideDrawRangeEx(E.x, E.y))	// both are outside, no need to draw
				return;
			else	// only E is inside
				r = BinarySearchPoint(r,l,p0,p1,p2,p3);
		}/*else{									// S is inside
			if(!InsideDrawRangeEx(E.x, E.y))	// only S is inside
				l = BinarySearchPoint(l,r,p0,p1,p2,p3);
			// both are inside, just continue
		}*/
	}

	float dist = (Bezier(p0,p1,p2,p3,r)-Bezier(p0,p1,p2,p3,l)).mod();
	if(dist>max_dist)
		dist=max_dist;
	int total = TOTAL*dist/max_dist;
	float t=l, inc = (r-l)/total;
	Point p=Bezier(p0,p1,p2,p3,l),q;
	for( float t=l; t < r; t+=inc )
	{
		Point q = Bezier(p0,p1,p2,p3,t);
		DrawBar(p,q,flag);
		p = q;
	}
	q = Bezier(p0,p1,p2,p3,r);
	DrawBar(p,q,flag);
}

void Unit::Draw_Bezier2(const Point &p0, const Point &p1, const Point &p2, const Point &p3, float l, float r, int flag)
{
	{	// binary search for the 1st point which is inside DrawRangeEx but outside DrawRange
		Point S = Bezier(p0,p1,p2,p3,r);
		Point E = Bezier(p0,p1,p2,p3,l);
		if(!InsideDrawRangeEx(S.x, S.y)){		// S is outside
			if(!InsideDrawRangeEx(E.x, E.y))	// both are outside, no need to draw
				return;
			else	// only E is inside
				r = BinarySearchPoint(r,l,p0,p1,p2,p3);
		}/*else{									// S is inside
			if(!InsideDrawRangeEx(E.x, E.y))	// only S is inside
				l = BinarySearchPoint(l,r,p0,p1,p2,p3);
			// both are inside, just continue
		}*/
	}

	// Render the strip
	int *pColor = flag?UNIT_COLOR[type]:UNIT_BGCOLOR;
	D3DCOLOR color_start = D3DCOLOR_ARGB(pColor[3],pColor[0],pColor[1],pColor[2]);
	D3DCOLOR color_end = D3DCOLOR_ARGB(pColor[3]>>4,pColor[0],pColor[1],pColor[2]);
	float width_inner = strip_widths[flag][0];
	float width_outer = strip_widths[flag][1];

	float dist = (Bezier(p0,p1,p2,p3,r)-Bezier(p0,p1,p2,p3,l)).mod();
	if(dist>max_dist)
		dist = max_dist;
	int total = TOTAL*dist/max_dist;
	if(!total)
		return;
	float t=l, inc = (r-l)/total;
	for( int x=0; x<=total; x++,t+=inc)
	{
		Point center = Bezier(p0,p1,p2,p3,t);
		Point dir = BezierDir(p0,p1,p2,p3,t).normal().unit();

		{	// center
			Point left = center-dir*width_inner;
			Point right = center+dir*width_inner;
			VertexXYZC L = {left.x, left.y, 0, color_start};
			VertexXYZC R = {right.x, right.y, 0, color_start};
			vbuf[x<<1] = L;
			vbuf[(x<<1)+1] = R;
		}

		{	// left side
			Point left = center-dir*width_inner;
			Point right = center-dir*width_outer;
			VertexXYZC L = {left.x, left.y, 0, color_start};
			VertexXYZC R = {right.x, right.y, 0, color_end};
			vbuf1[x<<1] = L;
			vbuf1[(x<<1)+1] = R;
		}

		{	// right side
			Point left = center+dir*width_inner;
			Point right = center+dir*width_outer;
			VertexXYZC L = {left.x, left.y, 0, color_start};
			VertexXYZC R = {right.x, right.y, 0, color_end};
			vbuf2[x<<1] = L;
			vbuf2[(x<<1)+1] = R;
		}
	}
	graphEngine.DrawPrimitiveUP(vbuf, g_FVF, g_stride, D3DPT_TRIANGLESTRIP, total*2);
	graphEngine.DrawPrimitiveUP(vbuf1, g_FVF, g_stride, D3DPT_TRIANGLESTRIP, total*2);
	graphEngine.DrawPrimitiveUP(vbuf2, g_FVF, g_stride, D3DPT_TRIANGLESTRIP, total*2);
}

void Unit::Draw(DWORD drawMask)
{
	if(noteType==NOTETYPE_NORMAL)
		Draw_Normal();
	else
		Draw_Strip(drawMask);
}

void Unit::Clear()
{
	if(systemIni.particle>=3)
		pSystem.SetState(GAMEDATA.chanceTime?0xc000:0x8000,((notePos<<4)+(type<<1)+1));
	else
		pSystem.Clear((notePos<<4)+(type<<1));
}

void Unit::Presh(PLAYSTATE id)
{
	if(noteType==NOTETYPE_NORMAL)
	{
		preshed = true;
		delayTime = 0;
	}
	else
	{
		if(!start)
			start = true;
		else
		{
			preshed = true;
			delayTime = 0;
		}
	}

	if(id!=WORST)
	{
		if(noteType==NOTETYPE_NORMAL)
		{
			effectMana.AddEffectNotePress(note.x,note.y,id);
			numberMana.SetCombo(GAMEDATA.combo,note.x,note.y);
		}
		else
		{
			effectMana.AddEffectNotePress(note.x,note.y,id);
			numberMana.SetCombo(GAMEDATA.combo,note.x,note.y);
		}
		if(key!=NOWAV)
		{
			string filename = _notemap.GetWav(key);
			if(filename!="")
				soundEngine->PlayMusic(filename);
		}
	}
}

void NoteMana::Run(LpFrame _note)
{
	if(_note==NULL)
		return;
	if(_note->_IsSetBPM)
		singleTime = _notemap.Cal_Single(_note->_BPM);
	double duration = _note->getTimePosition()-nowTime;
	if(duration<0)
		duration = 0;
	for(int i = 0; i < _note->_noteNum; i++)
	{
		if(_note->_note[i]._type<NOTENum)
			unit.push_back(Unit(_note->_note[i],duration,_note->_note[i].notePos,1));
		else
			unit.push_back(Unit(_note->_note[i],duration,_note->_note[i].notePos,_note->_note[i].duration*singleTime,1));
	}
}

void NoteMana::SetTimePosi(int framePos)
{
	for(int i=framePos; i>=0; i--)
		if(_notemap._timeTable._frame[i]._IsSetBPM){
			singleTime = _notemap.Cal_Single(_notemap._timeTable._frame[i]._BPM);
			break;
		}
	nowTime = _notemap._timeTable._frame[framePos].timePosition;
}

void NoteMana::SetNew()
{
	singleTime = nowTime = 0;
	speed_factor = tail_speed_factor = speed_factor_int = 1;
	Clear();
}
void NoteMana::Draw(int phase)
{
	if(systemIni.antialiasing){
		if(phase==1)
			for(auto ptr = unit.begin(); ptr != unit.end(); ptr++)
			{
				if(ptr->noteType==NOTETYPE_STRIP)
					ptr->Draw(1);
			}
		else
			for(auto ptr = unit.begin(); ptr != unit.end(); ptr++)
				ptr->Draw(2);
	}else{
		if(phase==1)
			for(auto ptr = unit.begin(); ptr != unit.end(); ptr++)
			{
				if(ptr->noteType==NOTETYPE_STRIP)
					ptr->Draw(0x3);
			}
		else
			for(auto ptr = unit.begin(); ptr != unit.end(); ptr++)
			{
				if(ptr->noteType==NOTETYPE_NORMAL)
					ptr->Draw();
			}
	}
}

void NoteMana::Add(int note_standing)
{
	while(true)
	{
		LpFrame frame = _notemap.GetNotes();
		if(frame==NULL)
			break;
		if(frame->getTimePosition()-nowTime>note_standing*singleTime)
			break;
		Run(frame);
		_notemap.NextNotes();
	}
}
void NoteMana::Pause(bool state)
{
	if(state==State_Paused)
		Update(base::currTime-base::lastTime);
	//else base::lastTime = (GetCurrentCount()+base::currTime)>>1;
}

void NoteMana::Update( LONGLONG dwTime )
{
	// Process the key_event queue first
	while(!key_queue.empty())
	{
		key_event &event = key_queue.pop();
		OnKeyEvent(event.keyInfo&0x7fffffff,(event.keyInfo&0x80000000)==0,event.dwTime-lastTime);
	}

	dwTime*=speed_factor;
	nowTime += dwTime;

	// update all units in 1 go
	for(auto ptr = unit.begin(); ptr != unit.end(); ptr++)
		ptr->Update(dwTime);

	if(GAMEDATA.gamemode==GAME_MODE_PV)
		return;

	bool findNew = true;
	int hit_count=0;

	while(findNew)
	{
		findNew = false;
		for(auto ptr = unit.begin(); ptr != unit.end(); ptr++)
		{
			// skip pressed but not yet deleted notes
			if(ptr->preshed)
			{
				if(fabs(ptr->delayTime)<EPS)
				{
					ptr->Clear();
					unit.erase(ptr);
				}
				continue;
			}

			if(ptr->noteType==NOTETYPE_NORMAL)		// normal note
			{
				if(GAMEDATA.gamemode==GAME_MODE_AUTO && (ptr->restTime-gameini.delay)<=gameini.eps)
				{		// auto press in auto mode
					UINT type = ptr->type;
					if( type>=0 && type<UNIT_KEYMAX )
						hit_count += OnKeyEvent(type,true,1);
					findNew = true;
				}
				else if(fabs(ptr->restTime)<EPS)			// timeout, note is missed
				{
					GAMEDATA.ModifyHP(gameini.HP_DEC), GAMEDATA.combo_draw = GAMEDATA.combo = 0, GAMEDATA.worst_cnt++, numberMana.SetCombo(0,ptr->note.x,ptr->note.y), gameui.SetState(WORST);
					GAMEDATA.maxScoreTempCombo++;
					GAMEDATA.maxScoreCanGet += gameini.SCORE_COOL + ((GAMEDATA.maxScoreTempCombo>50?50:GAMEDATA.maxScoreTempCombo)/10)*50;
					if(GAMEDATA.chanceTime)			// reset chance time combo
					{
						GAMEDATA._chanceTimeCombo = 0;
						numberMana.SetExtraScore((GAMEDATA._chanceTimeCombo>50?50:GAMEDATA._chanceTimeCombo)*100,ptr->note.x,ptr->note.y);
						GAMEDATA.maxScoreTempChanceTimeCombo++;
						GAMEDATA.maxScoreCanGet += (GAMEDATA.maxScoreTempChanceTimeCombo>50?50:GAMEDATA.maxScoreTempChanceTimeCombo)*100;
					}
					ptr->Clear();
					unit.erase(ptr);
					findNew = true;
				}
				break;
			}
			else		// strip note
			{
				if(GAMEDATA.gamemode==GAME_MODE_AUTO)
				{
					if(!ptr->start && (ptr->restTime-gameini.delay)<gameini.eps)
					{		// simulate press
						UINT type = ptr->type;
						if( type>=0 && type<UNIT_KEYMAX )
							hit_count += OnKeyEvent(type,true,1);
						findNew = true;
						break;
					}
					else if(ptr->start && (ptr->duration-gameini.delay)<gameini.eps)
					{		// simulate release
						UINT type = ptr->type;
						if( type>=0 && type<UNIT_KEYMAX )
							OnKeyEvent(type,false,1);
						findNew = true;
						break;
					}
				}
				if(ptr->finish())		// missed
				{
					GAMEDATA.ModifyHP(gameini.HP_DEC), GAMEDATA.combo_draw = GAMEDATA.combo = 0, GAMEDATA.worst_cnt++, numberMana.SetCombo(0,ptr->note.x,ptr->note.y), gameui.SetState(WORST);
					GAMEDATA.maxScoreTempCombo++;
					GAMEDATA.maxScoreCanGet += gameini.SCORE_COOL + ((GAMEDATA.maxScoreTempCombo>50?50:GAMEDATA.maxScoreTempCombo)/10)*50;
					if(GAMEDATA.chanceTime)
					{
						GAMEDATA._chanceTimeCombo = 0;
						numberMana.SetExtraScore((GAMEDATA._chanceTimeCombo>50?50:GAMEDATA._chanceTimeCombo)*100,ptr->note.x,ptr->note.y);
						GAMEDATA.maxScoreTempChanceTimeCombo++;
						GAMEDATA.maxScoreCanGet += (GAMEDATA.maxScoreTempChanceTimeCombo>50?50:GAMEDATA.maxScoreTempChanceTimeCombo)*100;
					}
					ptr->Presh(WORST);
					ptr->Presh(WORST);
					//ptr->Clear();
					//unit.erase(ptr);
					findNew = true;
					break;
				}
			}
		}
	}

	if(!gameui.combo_star){
		if(GAMEDATA.combo>=STARLEVEL)
			gameui.SetComboStar();
	}else{
		if(GAMEDATA.combo<STARLEVEL){
			gameui.ClearComboStar();
			core.ChangePlaySpeed(0,1);
		}
	}

	if(hit_count)
		soundEngine->PlayHit(hit_count);
}

int NoteMana::PushKeyEvent(UINT vkCode, bool bKeyDown, LONGLONG time_stamp)
{
	char type = key_map[vkCode];
	if(type>=0){	// if key matches note key
		// get time stamp
		NoteMana::key_event new_event = {time_stamp-base::delayCompenTime, type|(bKeyDown?0:0x80000000)};

		// play sound
		if(bKeyDown)
			soundEngine->PlayHit(1);

		// save time stamp and return immediately
		key_queue.push_back(new_event);
		return 1;
	}
	return 0;
}

int NoteMana::OnKeyEvent(UINT type, bool bKeyDown, LONGLONG dwTime)
{
	if(!nowTime) return 0;

	if(bKeyDown)	// keyPress event
	{
		auto best_ptr = unit.end();
		double best_ptr_time = 1e20f;
		bool bMatched = false;
		for(auto ptr = unit.begin(); ptr!=unit.end(); ++ptr)
		{	// find best matching note
			if( ptr->preshed || (ptr->noteType==NOTETYPE_STRIP && ptr->start) )	// skip pressed note or started strip note
				continue;

			double restTime = ptr->restTime-dwTime-gameini.delay;
			if(restTime > gameini.delay)	// no more notes in keystroke time frame
				if(ptr->noteType!=NOTETYPE_STRIP)			// strip notes may lay outside time frame
				{
					ptr = unit.end();
					break;
				}

			// if there are notes in time frame which match the keystroke,
			// consider the closest matched one as correct,
			// otherwise, consider the closest unmatched one as incorrect
			restTime = fabs(restTime);
			if(ptr->type==type){
				if(bMatched){
					if(restTime<best_ptr_time){
						best_ptr_time = restTime;
						best_ptr = ptr;
					}
				}else{
					bMatched = true;
					best_ptr_time = restTime;
					best_ptr = ptr;
				}
			}else if(systemIni.judgement){
				if(!bMatched){
					if(restTime<best_ptr_time && ptr->noteType!=NOTETYPE_STRIP){
						best_ptr_time = restTime;
						best_ptr = ptr;
					}
				}
			}
		}
		if(best_ptr!=unit.end())
		{
			if(best_ptr->type==type)
			{	// pressed correctly
				best_ptr_time /= speed_factor;
				if(best_ptr_time<=gameini.COOL || GAMEDATA.gamemode==GAME_MODE_AUTO)
				{
					//effectMana.AddEffectResetFlame(ptr->type);
					GAMEDATA.ModifyHP(gameini.HP_ADD), GAMEDATA.cool_cnt++, GAMEDATA.combo++, GAMEDATA.combo_draw++, GAMEDATA.score += gameini.SCORE_COOL + ((GAMEDATA.combo>50?50:GAMEDATA.combo)/10)*50, best_ptr->Presh(COOL), gameui.SetState(COOL);
					GAMEDATA.maxScoreTempCombo++;
					GAMEDATA.maxScoreCanGet += gameini.SCORE_COOL + ((GAMEDATA.maxScoreTempCombo>50?50:GAMEDATA.maxScoreTempCombo)/10)*50;
					if(GAMEDATA.chanceTime)
					{
						GAMEDATA._chanceTimeCombo++;
						int inc = min(GAMEDATA._chanceTimeCombo, 50)*100;
						GAMEDATA.score += inc;
						GAMEDATA._chanceTimeScore += inc;
						numberMana.SetExtraScore(inc, best_ptr->note.x, best_ptr->note.y);
						GAMEDATA.maxScoreTempChanceTimeCombo++;
						GAMEDATA.maxScoreCanGet += (GAMEDATA.maxScoreTempChanceTimeCombo>50?50:GAMEDATA.maxScoreTempChanceTimeCombo)*100;
					}
					if(GAMEDATA.chanceTime && systemIni.particle>=3)
						for(short angle=spin_angle_inc[0]; angle<360; angle+=spin_angle_inc[0])//
							best_ptr->AddSpinParticle(angle);
				}
				else if(best_ptr_time<=gameini.FINE)
				{
					GAMEDATA.ModifyHP(gameini.HP_ADD), GAMEDATA.fine_cnt++, GAMEDATA.combo++, GAMEDATA.combo_draw++, GAMEDATA.score += gameini.SCORE_FINE + ((GAMEDATA.combo>50?50:GAMEDATA.combo)/10)*50, best_ptr->Presh(FINE), gameui.SetState(FINE);
					GAMEDATA.maxScoreTempCombo++;
					GAMEDATA.maxScoreCanGet += gameini.SCORE_COOL + ((GAMEDATA.maxScoreTempCombo>50?50:GAMEDATA.maxScoreTempCombo)/10)*50;
					if(GAMEDATA.chanceTime)
					{
						GAMEDATA._chanceTimeCombo++;
						int inc = min(GAMEDATA._chanceTimeCombo, 50)*100;
						GAMEDATA.score += inc;
						GAMEDATA._chanceTimeScore += inc;
						numberMana.SetExtraScore(inc, best_ptr->note.x, best_ptr->note.y);
						GAMEDATA.maxScoreTempChanceTimeCombo++;
						GAMEDATA.maxScoreCanGet += (GAMEDATA.maxScoreTempChanceTimeCombo>50?50:GAMEDATA.maxScoreTempChanceTimeCombo)*100;
					}
					if(GAMEDATA.chanceTime && systemIni.particle>=3)
						for(short angle=spin_angle_inc[1]; angle<360; angle+=spin_angle_inc[1])
							best_ptr->AddSpinParticle(angle);
				}
				else if(best_ptr_time<=gameini.SAFE)
				{
					GAMEDATA.ModifyHP(gameini.HP_SAFE), GAMEDATA.score += gameini.SCORE_SAFE, GAMEDATA.safe_cnt++, GAMEDATA.combo_draw=GAMEDATA.combo=0, best_ptr->Presh(SAFE), gameui.SetState(SAFE);
					GAMEDATA.maxScoreTempCombo++;
					GAMEDATA.maxScoreCanGet += gameini.SCORE_COOL + ((GAMEDATA.maxScoreTempCombo>50?50:GAMEDATA.maxScoreTempCombo)/10)*50;
					if(GAMEDATA.chanceTime)
					{
						GAMEDATA._chanceTimeCombo=0;
						numberMana.SetExtraScore((GAMEDATA._chanceTimeCombo>50?50:GAMEDATA._chanceTimeCombo)*100,best_ptr->note.x,best_ptr->note.y);
						GAMEDATA.maxScoreTempChanceTimeCombo++;
						GAMEDATA.maxScoreCanGet += (GAMEDATA.maxScoreTempChanceTimeCombo>50?50:GAMEDATA.maxScoreTempChanceTimeCombo)*100;
					}
				}
				else if(best_ptr_time<=gameini.SAD)
				{
					//effectMana.AddEffectResetFlame(ptr->type);
					GAMEDATA.ModifyHP(gameini.HP_SAD), GAMEDATA.score += gameini.SCORE_SAD, GAMEDATA.sad_cnt++, GAMEDATA.combo_draw=GAMEDATA.combo=0, best_ptr->Presh(SAD), gameui.SetState(SAD);
					GAMEDATA.maxScoreTempCombo++;
					GAMEDATA.maxScoreCanGet += gameini.SCORE_COOL + ((GAMEDATA.maxScoreTempCombo>50?50:GAMEDATA.maxScoreTempCombo)/10)*50;
					if(GAMEDATA.chanceTime)
					{
						GAMEDATA._chanceTimeCombo=0;
						numberMana.SetExtraScore((GAMEDATA._chanceTimeCombo>50?50:GAMEDATA._chanceTimeCombo)*100,best_ptr->note.x,best_ptr->note.y);
						GAMEDATA.maxScoreTempChanceTimeCombo++;
						GAMEDATA.maxScoreCanGet += (GAMEDATA.maxScoreTempChanceTimeCombo>50?50:GAMEDATA.maxScoreTempChanceTimeCombo)*100;
					}
				}
				GAMEDATA.maxcombo = max(GAMEDATA.maxcombo,GAMEDATA.combo);
			}
			else
			{	// pressed wrongly
				GAMEDATA.ModifyHP(gameini.HP_DEC), GAMEDATA.combo_draw = GAMEDATA.combo = 0, GAMEDATA.worst_cnt++, numberMana.SetCombo(0,best_ptr->note.x,best_ptr->note.y),best_ptr->Presh(WORST),gameui.SetState(WORST);
				GAMEDATA.maxScoreTempCombo++;
				GAMEDATA.maxScoreCanGet += gameini.SCORE_COOL + ((GAMEDATA.maxScoreTempCombo>50?50:GAMEDATA.maxScoreTempCombo)/10)*50;
				if(GAMEDATA.chanceTime)
				{
					GAMEDATA._chanceTimeCombo=0;
					numberMana.SetExtraScore((GAMEDATA._chanceTimeCombo>50?50:GAMEDATA._chanceTimeCombo)*100,best_ptr->note.x,best_ptr->note.y);
					GAMEDATA.maxScoreTempChanceTimeCombo++;
					GAMEDATA.maxScoreCanGet += (GAMEDATA.maxScoreTempChanceTimeCombo>50?50:GAMEDATA.maxScoreTempChanceTimeCombo)*100;
				}
				GAMEDATA.ModifyHP(gameini.HP_DEC);
			}
		}else if(!unit.empty()){	// penalize for pressing extra keys when there are units on screen
			GAMEDATA.ModifyHP(gameini.HP_DEC), GAMEDATA.combo_draw = GAMEDATA.combo = 0, GAMEDATA.worst_cnt++, numberMana.SetCombo(0,best_ptr->note.x,best_ptr->note.y),best_ptr->Presh(WORST),gameui.SetState(WORST);
			if(GAMEDATA.chanceTime)
			{
				GAMEDATA._chanceTimeCombo=0;
				numberMana.SetExtraScore((GAMEDATA._chanceTimeCombo>50?50:GAMEDATA._chanceTimeCombo)*100,best_ptr->note.x,best_ptr->note.y);
			}
		}
	}
	else	// keyRelease event
	{	
		auto ptr = unit.begin();

		while(ptr!=unit.end())
		{
			if(ptr->noteType==NOTETYPE_STRIP&&ptr->type==type&&ptr->start&&!ptr->preshed)
				break;
			ptr++;
		}
		if(ptr!=unit.end())
		{
			double restTime = fabs(ptr->duration-dwTime-gameini.delay)/speed_factor;
			soundEngine->PlayHit(1);

			if(restTime<=gameini.COOL || GAMEDATA.gamemode==GAME_MODE_AUTO)
			{
				GAMEDATA.ModifyHP(gameini.HP_ADD), GAMEDATA.cool_cnt++, GAMEDATA.combo++, GAMEDATA.combo_draw++, GAMEDATA.score += gameini.SCORE_COOL + ((GAMEDATA.combo>50?50:GAMEDATA.combo)/10)*50, ptr->Presh(COOL), gameui.SetState(COOL);
				GAMEDATA.maxScoreTempCombo++;
				GAMEDATA.maxScoreCanGet += gameini.SCORE_COOL + ((GAMEDATA.maxScoreTempCombo>50?50:GAMEDATA.maxScoreTempCombo)/10)*50;
				if(GAMEDATA.chanceTime)
				{
					GAMEDATA._chanceTimeCombo++;
					int inc = min(GAMEDATA._chanceTimeCombo, 50)*100;
					GAMEDATA.score += inc;
					GAMEDATA._chanceTimeScore += inc;
					numberMana.SetExtraScore(inc, ptr->note.x, ptr->note.y);
					GAMEDATA.maxScoreTempChanceTimeCombo++;
					GAMEDATA.maxScoreCanGet += (GAMEDATA.maxScoreTempChanceTimeCombo>50?50:GAMEDATA.maxScoreTempChanceTimeCombo)*100;
				}
				if(GAMEDATA.chanceTime && systemIni.particle>=3)
					for(short angle=spin_angle_inc[0]; angle<=360; angle+=spin_angle_inc[0])
						ptr->AddSpinParticle(angle);
			}
			else if(restTime<=gameini.FINE)
			{
				GAMEDATA.ModifyHP(gameini.HP_ADD), GAMEDATA.fine_cnt++, GAMEDATA.combo++, GAMEDATA.combo_draw++, GAMEDATA.score += gameini.SCORE_FINE + ((GAMEDATA.combo>50?50:GAMEDATA.combo)/10)*50, ptr->Presh(FINE), gameui.SetState(FINE);
				GAMEDATA.maxScoreTempCombo++;
				GAMEDATA.maxScoreCanGet += gameini.SCORE_COOL + ((GAMEDATA.maxScoreTempCombo>50?50:GAMEDATA.maxScoreTempCombo)/10)*50;
				if(GAMEDATA.chanceTime)
				{
					GAMEDATA._chanceTimeCombo++;
					int inc = min(GAMEDATA._chanceTimeCombo, 50)*100;
					GAMEDATA.score += inc;
					GAMEDATA._chanceTimeScore += inc;
					numberMana.SetExtraScore(inc, ptr->note.x, ptr->note.y);
					GAMEDATA.maxScoreTempChanceTimeCombo++;
					GAMEDATA.maxScoreCanGet += (GAMEDATA.maxScoreTempChanceTimeCombo>50?50:GAMEDATA.maxScoreTempChanceTimeCombo)*100;
				}
				if(GAMEDATA.chanceTime && systemIni.particle>=3)
					for(short angle=spin_angle_inc[1]; angle<360; angle+=spin_angle_inc[1])
						ptr->AddSpinParticle(angle);
			}
			else if(restTime<=gameini.SAFE)
			{
				GAMEDATA.ModifyHP(gameini.HP_SAFE), GAMEDATA.score += gameini.SCORE_SAFE, GAMEDATA.safe_cnt++, GAMEDATA.combo_draw=GAMEDATA.combo=0, ptr->Presh(SAFE), gameui.SetState(SAFE);
				GAMEDATA.maxScoreTempCombo++;
				GAMEDATA.maxScoreCanGet += gameini.SCORE_COOL + ((GAMEDATA.maxScoreTempCombo>50?50:GAMEDATA.maxScoreTempCombo)/10)*50;
				if(GAMEDATA.chanceTime)
				{
					GAMEDATA._chanceTimeCombo=0;
					numberMana.SetExtraScore((GAMEDATA._chanceTimeCombo>50?50:GAMEDATA._chanceTimeCombo)*100,ptr->note.x,ptr->note.y);
					GAMEDATA.maxScoreTempChanceTimeCombo++;
					GAMEDATA.maxScoreCanGet += (GAMEDATA.maxScoreTempChanceTimeCombo>50?50:GAMEDATA.maxScoreTempChanceTimeCombo)*100;
				}
			}
			else if(restTime<=gameini.SAD)
			{
				GAMEDATA.ModifyHP(gameini.HP_SAD), GAMEDATA.score += gameini.SCORE_SAD, GAMEDATA.sad_cnt++, GAMEDATA.combo_draw=GAMEDATA.combo=0, ptr->Presh(SAD), gameui.SetState(SAD);
				GAMEDATA.maxScoreTempCombo++;
				GAMEDATA.maxScoreCanGet += gameini.SCORE_COOL + ((GAMEDATA.maxScoreTempCombo>50?50:GAMEDATA.maxScoreTempCombo)/10)*50;
				if(GAMEDATA.chanceTime)
				{
					GAMEDATA._chanceTimeCombo=0;
					numberMana.SetExtraScore((GAMEDATA._chanceTimeCombo>50?50:GAMEDATA._chanceTimeCombo)*100,ptr->note.x,ptr->note.y);
					GAMEDATA.maxScoreTempChanceTimeCombo++;
					GAMEDATA.maxScoreCanGet += (GAMEDATA.maxScoreTempChanceTimeCombo>50?50:GAMEDATA.maxScoreTempChanceTimeCombo)*100;
				}
			}
			else
			{	// press wrongly or miss
				GAMEDATA.ModifyHP(gameini.HP_DEC), GAMEDATA.combo_draw = GAMEDATA.combo = 0, GAMEDATA.worst_cnt++, numberMana.SetCombo(0,ptr->note.x,ptr->note.y),ptr->Presh(WORST),gameui.SetState(WORST);
				GAMEDATA.maxScoreTempCombo++;
				GAMEDATA.maxScoreCanGet += gameini.SCORE_COOL + ((GAMEDATA.maxScoreTempCombo>50?50:GAMEDATA.maxScoreTempCombo)/10)*50;
				if(GAMEDATA.chanceTime)
				{
					GAMEDATA._chanceTimeCombo = 0;
					numberMana.SetExtraScore((GAMEDATA._chanceTimeCombo>50?50:GAMEDATA._chanceTimeCombo)*100,ptr->note.x,ptr->note.y);
					GAMEDATA.maxScoreTempChanceTimeCombo++;
					GAMEDATA.maxScoreCanGet += (GAMEDATA.maxScoreTempChanceTimeCombo>50?50:GAMEDATA.maxScoreTempChanceTimeCombo)*100;
				}
			}
			GAMEDATA.maxcombo = max(GAMEDATA.maxcombo,GAMEDATA.combo);
		}
	}
	return 1;
}
//------------------------------------------------------------------------------------

void GameUI::Init()
{
	state = NOSTATE;
	uistate = NORMAL;
	ui = graphEngine.AddTexture("pic\\ui.png");

	// load ui.png
	struct toLoad{
		Resource** ppRes;
		string res_name;
	};

	static toLoad toloads[]={
		{(Resource**)&top_bar_rc,				"top_bar"				},
		{(Resource**)&bottom_bar_rc,			"bottom_bar"			},
		{(Resource**)&top_bar_red_rc,			"top_bar_red"			},
		{(Resource**)&bottom_bar_red_rc,		"bottom_bar_red"		},
		{(Resource**)&rhythm_logo_rc,			"rhythm_logo"			},
		{(Resource**)&combo_gauge_rc,			"combo_gauge[13]"		},
		{(Resource**)&combo_shift_rc,			"combo_shift[2]"		},
		{(Resource**)&progress_bar_front_rc,	"progress_bar_front"	},
		{(Resource**)&progress_bar_back_rc,		"progress_bar_back"		},
		{(Resource**)&progress_bar_pointer_rc,	"progress_bar_pointer"	},
		{(Resource**)&hp_gauge_rc,				"hp_gauge[20]"			},
		{(Resource**)&hp_rhyme_rc,				"hp_rhyme"				},
		{(Resource**)&hp_UPs_rc,				"hp_UPs[5]"				},
		{(Resource**)&hp_ExQn_rc,				"hp_ExQn"				},
		{(Resource**)&hp_Ex_rc,					"hp_Ex"					},

		{(Resource**)&state0_rc,				"press_state0_rc"		},
		{(Resource**)&state1_rc,				"press_state1_rc"		},
		{(Resource**)&state2_rc,				"press_state2_rc"		},
		{(Resource**)&state3_rc,				"press_state3_rc"		},
		{(Resource**)&state4_rc,				"press_state4_rc"		},
		{(Resource**)&state5_rc,				"press_state5_rc"		},
		{(Resource**)&state6_rc,				"press_state6_rc"		},
	};
	for(int x=0, X=sizeof(toloads)/sizeof(toLoad); x<X; x++)
	{
		*toloads[x].ppRes = g_res.getResource(toloads[x].res_name);
	}
	press_state[0] = state0_rc;
	press_state[1] = state1_rc;
	press_state[2] = state2_rc;
	press_state[3] = state3_rc;
	press_state[4] = state4_rc;
	press_state[5] = state5_rc;
	press_state[6] = state6_rc;

	spin_angle_inc = ((IntResource*)g_res.getResource("spin_angle_inc[2]"))->value;
	SINGLE_LINE_INC = *((FloatResource*)g_res.getResource("single_line_inc"))->value;
	DOUBLE_LINE_INC = *((FloatResource*)g_res.getResource("double_line_inc"))->value;
	DOUBLE_LINE_TWIST_FACTOR = *((FloatResource*)g_res.getResource("double_line_twist_factor"))->value;
	DOUBLE_LINE_SPACE_FACTOR = *((FloatResource*)g_res.getResource("double_line_space_factor"))->value;
	COMET_LINE_INC = *((FloatResource*)g_res.getResource("comet_line_inc"))->value;
	STRIP_NOTE_SIZE = *((FloatResource*)g_res.getResource("strip_note_size"))->value;
	NOTE_SIZE = *((FloatResource*)g_res.getResource("note_size"))->value;
	META_SIZE = *((FloatResource*)g_res.getResource("meta_size"))->value;
	ARROW_HEAD = *((FloatResource*)g_res.getResource("arrow_head"))->value;
}

void GameUI::SetNew()
{
	state = NOSTATE;
	uistate = NORMAL;
	delayTime = uidelay = 0;
	deltaTop = top_bar_rc->Height;
	deltaBottom = bottom_bar_rc->Height;
	ClearComboStar();
	MoveIn(30);
}

void GameUI::SetComboStar()
{
	if(!g_res.getResource("combo_star"))
		return;
	ClearComboStar();
	combo_star = new GUIStaticImage("combo_star");
	combo_star->efvct.AddEffect(new GUIEF_Alpha(white->efvct.a,2,255));
	auto X = (combo_star->drawRect.left+combo_star->drawRect.right)/2;
	auto Y = (combo_star->drawRect.top+combo_star->drawRect.bottom)/2;
	auto width = combo_star->drawRect.right-combo_star->drawRect.left;
	RECT beginRect = {X, Y, X+1, Y+1};
	RECT middleRect = {X-(width<<2), Y-(width<<2), X+(width<<2), Y+(width<<2)};
	RECT finalRect = combo_star->drawRect;
	combo_star->drawRect = beginRect;
	combo_star->efvct.AddEffect(new GUIEF_Stretch(&combo_star->drawRect, beginRect, middleRect, 16, 0));
	combo_star->efvct.AddEffect(new GUIEF_Stretch(&combo_star->drawRect, middleRect, finalRect, 16, 16));
}

void GameUI::ClearComboStar()
{
	if(combo_star)
		delete combo_star;
	combo_star = NULL;
}

void GameUI::Update(double dwTime)
{
	black_top->Update();
	black_bottom->Update();
	if(GAMEDATA.chanceTime)
		chanceTimeLabel->Update();
	if(fabs(delayTime)>EPS)
	{
		delayTime -= dwTime;
		if(delayTime<0)
			delayTime = 0;
	}
	else
		state = NOSTATE;
	if(fabs(uidelay)>EPS)
	{
		uidelay -= dwTime;
		if(uidelay<0)
			uidelay = 0;
	}
	else
		uistate = NORMAL;
	spark++;

	nowTime++;
	if(moveState==1)
	{
		deltaTop = top_bar_rc->Height*double(moveTime-nowTime)/moveTime;
		deltaBottom = bottom_bar_rc->Height*double(moveTime-nowTime)/moveTime;
	}
	else if(moveState==2)
	{
		deltaTop = top_bar_rc->Height*double(nowTime)/moveTime;
		deltaBottom = bottom_bar_rc->Height*double(nowTime)/moveTime;
	}
	if(nowTime==moveTime)
		moveState = 0;
	if(combo_star)
		combo_star->Update();
}

void *prepare_combo_gauge(){
	static VertexXYZC buf[]={
		{10,10,10,0xffff0000},
		{10,300,10,0xff00ff00},
		{50,300,10,0xff0000ff},
	};
	return buf;
}

void GameUI::Draw(int game_mode)
{
	if(game_mode!=GAME_MODE_PV){
		// determine hp number: 0 (empty: nothing to draw), 1 ~ 15 (15 drawable levels)
		const int hpnum = GAMEDATA.hp*15.0f/gameini.HP+0.5f;
		// determine hp index: source index in ui.png
		int hp_index = hpnum-1;
		// sign indicator on top of hp gauge
		//RECT sign_rect = MakeRect(0,0,0,0);
		ImageResource *sign_rect_rc = NULL;

		// draw top and bottom panel bar
		if(hpnum<=3 && ((spark>>5)&1))
		{
			graphEngine.Draw(top_bar_red_rc,0,-deltaTop);
			graphEngine.Draw(bottom_bar_red_rc,0,deltaBottom);
			if(hpnum) hp_index += 17;
			//sign_rect = MakeRect(116,271,36,36);
			sign_rect_rc = hp_Ex_rc;
		}
		else
		{
			graphEngine.Draw(top_bar_rc,0,-deltaTop);
			graphEngine.Draw(bottom_bar_rc,0,deltaBottom);
		}

		if(hpnum>=15)
		{	// highest 1 levels
			//sign_rect = MakeRect(40,271,36,36);
			sign_rect_rc = hp_rhyme_rc;
		}

		// Draw hp gauge if non-empty
		if(hp_index>=0){
			if(core.cheatDie)
				graphEngine.Draw1(&hp_gauge_rc[hp_index],0,deltaBottom,0xe0111111);
			else
				graphEngine.Draw(&hp_gauge_rc[hp_index],0,deltaBottom);
		}

		// rolling up or down arrow
		switch(uistate)
		{
		case ADD:
			//graphEngine.Draw(ui,MakeRect(spark/5%5*38,155,38,38),9,HEIGHT-bottom_bar_rc->Height+10+deltaBottom);
			graphEngine.Draw(&hp_UPs_rc[spark/5%5],0,deltaBottom);
			break;
		case DEC:
			if(!(hpnum<=3 && ((spark>>5)&1)))
			{
				//graphEngine.Draw(ui,MakeRect(76,271,36,36),8,HEIGHT-bottom_bar_rc->Height+10+deltaBottom);
				graphEngine.Draw(hp_ExQn_rc,0,deltaBottom);
				break;
			}
		case NORMAL:
			//graphEngine.Draw(ui,sign_rect,11,HEIGHT-bottom_bar_rc->Height+10+deltaBottom);
			if(sign_rect_rc)
				graphEngine.Draw(sign_rect_rc,0,deltaBottom);
			break;
		}

		{	// draw combo gauge
			int combo_index = 12-GAMEDATA.combo_draw*12/100;
			if(combo_index<0) combo_index = 0;
			graphEngine.Draw(&combo_gauge_rc[combo_index],0,deltaBottom);
		}

		// draw state (good/safe/worst)
		if(state!=NOSTATE)
		{
			graphEngine.Draw(press_state[state],0,deltaBottom);
			//graphEngine.Draw(ui,GAMEDATA.state_rect[state],WIDTH-90,HEIGHT-20+deltaBottom);
		}

		numberMana.DrawScore(GAMEDATA.score);

		black_top->Draw();
		black_bottom->Draw();

		if(GAMEDATA.chanceTime)
			chanceTimeLabel->Draw();

		// Draw shift button
		if(GAMEDATA.combo_draw>=STARLEVEL)
			graphEngine.Draw(&combo_shift_rc[(spark>>3)&1?1:0]);

		// Draw combo star
		if(combo_star)
			combo_star->Draw(0, deltaBottom);
	}

	// Compute progress
	float cur_progress = _notemap.GetProgress();
	if(cur_progress>1) cur_progress=1;

	// Draw progress bar
	// 1. background
	graphEngine.Draw(progress_bar_back_rc);

	// 2. scrolling foreground
	RECT src_rect = progress_bar_front_rc->srcRect;
	fRECT dst_rect = progress_bar_front_rc->dstRect;
	LONG width_2 = (src_rect.right-src_rect.left)>>1;
	float inc = (nowTime<<1)%width_2;

	dst_rect.right = dst_rect.left+cur_progress*(dst_rect.right-dst_rect.left)+0.5f;

	src_rect.right	= src_rect.left+cur_progress*width_2+0.5f;
	src_rect.left	+= (width_2-inc);
	src_rect.right	+= (width_2-inc);

	graphEngine.Draw(progress_bar_front_rc, src_rect, dst_rect);

	if(game_mode!=GAME_MODE_PV){
		// 3. progress pointer
		graphEngine.DrawCenter(progress_bar_pointer_rc, dst_rect.right, (dst_rect.bottom+dst_rect.top)*0.5f);

		graphEngine.Draw(rhythm_logo_rc);
		graphEngine.SetFont(systemIni.font,12,false,FW_BOLD);
		{
			string map_name = _notemap.GetMapName();
			if(!map_name.empty())
				graphEngine.DrawText(map_name+" - "+StringTable(HardType[_notemap.GetLevel()-1]),22,3);
		}
	}
}

void GameUI::SetState(PLAYSTATE _state)
{
	state = _state;
	if(state!=NOSTATE)
		delayTime = gameini.playstate_delay;
}

void GameUI::SetUIState(UISTATE _state)
{
	uistate = _state;
	if(uistate!=NORMAL)
		uidelay = gameini.uistate_delay;
}

void GameUI::MoveAway(int time)
{
	moveState = 2;
	moveTime = time, nowTime = 0;
}

void GameUI::MoveIn(int time)
{
	moveState = 1;
	moveTime = time, nowTime = 0;
}

//------------------------------------------------------------------------------------
void GameCore::Init()
{
	graphEngine.InitEngine(base::Device,base::Sprite);
	videoEngine.Init();
	GAMEDATA.Init();
	gameui.Init();
	gameini.Init();
	_notemana.Init();
	effectMana.Init();
	numberMana.Init();
	pSystem.Init();

	white = new GUIStaticImage("white");
	black_top = new GUIStaticImage("black_top");
	(*black_top->efvct.a)=128;
	black_bottom = new GUIStaticImage("black_bottom");
	(*black_bottom->efvct.a)=128;
	chanceTimeLabel = new GUIStaticImage("chanceTimeLabel");
	chanceTimeLabel->efvct.AddEffect(new Spark(chanceTimeLabel->efvct.a,3,true,-1,255,255,128,255));
}

GameCore::~GameCore()
{
	SAFE_DELETE(menu);
	SAFE_DELETE(white);
	SAFE_DELETE(black_top);
	SAFE_DELETE(black_bottom);
	graphEngine.ReleaseTex();
}

void GameCore::LoadCalibration(string filename)
{
	map_calibration.clear();
	ifstream fin(filename);
	if(fin){
		int level;
		double offset;
		while(fin>>level>>offset)
			map_calibration[level] = offset;
	}
	if(map_calibration.find(0)==map_calibration.end())
		map_calibration[0]=0.0;
}

void GameCore::SaveCalibration()
{
	ofstream fout(calib_filename);
	for each(auto it in map_calibration)
		fout << it.first << ' ' << it.second << endl;
}

void GameCore::SetNew(string filename,string baseDirec,int gamemode)
{
	//dInputKeyboard.start();
	calib_filename = baseDirec+"calibration.inf";
	GAMEDATA.SetNew();
	gameui.SetNew();
	GAMEDATA.gamemode = gamemode;
	videoEngine.SetNew();
	_notemap.SetNew(filename,baseDirec);
	//singleTime = _notemap.Cal_Single(_notemap.GetBPM());
	nowDistance = gameini.DISTANCE*gameini.BPM/_notemap.GetBPM();

	LoadCalibration(calib_filename);

	noteTime = 0;
	totalTime = 0;
	prev_seektrack_percent = 0;
	_notemap.Resource_read();
	effectMana.SetNew();
	numberMana.SetNew();
	_notemana.SetNew();
	finish = paused = false, visible = true;
	(*black_top->efvct.drawRect)=MakeRect(0,-gameui.top_bar_rc->Height,gameui.top_bar_rc->Width,gameui.top_bar_rc->Height);
	(*black_bottom->efvct.drawRect)=MakeRect(0,HEIGHT,gameui.top_bar_rc->Width,gameui.top_bar_rc->Height);
	soundEngine->FreeChannel(0);
	cheatDie = false;
	ChangeTimeOffsets(0,"");
}

void GameCore::Run(LpFrame _nowFrame, int mask)
{
	if(_nowFrame==NULL)
		return;
	if(mask&4)
		if(_nowFrame->_IsSetBPM)
		{
			//singleTime = _notemap.Cal_Single(_nowFrame->_BPM);
			nowDistance = gameini.DISTANCE*gameini.BPM/_nowFrame->_BPM;
		}
	string filename;
	if(mask&2)
		if(_nowFrame->_resource!=NOSOURCE)
		{
			filename = _notemap.GetResource(_nowFrame->_resource);
			if(filename!="")
			{
				if(CheckResource(filename)==GRAPH)
					GAMEDATA.background = graphEngine.AddTexture(filename,GameCoreRes);
				else
				{
					videoEngine.Play(filename, true);
					GAMEDATA.background = NULL;
				}
			}
		}
	if(mask&1)
		for(int i = 0; i < BGSNum; i++)
		{
			if(_nowFrame->_BGM[i]>=0){
				soundEngine->PlayMusic(_nowFrame->_BGM[i], 0, false, pow(2.0, map_calibration[0]));
				if(systemIni.KeepPitch)
					soundEngine->SetTempo(_notemana.speed_factor,0,true);
				else
					soundEngine->SetSampleRate(_notemana.speed_factor,0,true);
			}
		}
	if(mask&8)
	{
		if(!GAMEDATA.chanceTime&&_nowFrame->notePos==_notemap._chanceTimeStart)
		{
			GAMEDATA.chanceTime = true;
			(*black_top->efvct.drawRect) = MakeRect(0,0,480,0);
			(*black_bottom->efvct.drawRect) = MakeRect(0,272,480,0);
			black_top->efvct.AddEffect(new GUIEF_Stretch(black_top->efvct.drawRect,20,MakeRect(0,0,gameui.top_bar_rc->Width,gameui.top_bar_rc->Height),0));
			black_bottom->efvct.AddEffect(new GUIEF_Stretch(black_bottom->efvct.drawRect,20,MakeRect(0,HEIGHT-gameui.top_bar_rc->Height,gameui.top_bar_rc->Width,gameui.top_bar_rc->Height),0));
			effectMana.AddEffectChanceTimeStart();
			gameui.MoveAway(30);
		}
		if(GAMEDATA.chanceTime&&_nowFrame->notePos==_notemap._chanceTimeEnd+1)
		{
			GAMEDATA.chanceTime = false;
			black_top->efvct.ReleaseEffects();
			black_bottom->efvct.ReleaseEffects();
			black_top->efvct.AddEffect(new GUIEF_Stretch(black_top->efvct.drawRect,20,MakeRect(0,-(gameui.top_bar_rc->Height),gameui.top_bar_rc->Width,gameui.top_bar_rc->Height),0));
			black_bottom->efvct.AddEffect(new GUIEF_Stretch(black_bottom->efvct.drawRect,20,MakeRect(0,HEIGHT,gameui.top_bar_rc->Width,gameui.top_bar_rc->Height),0));
			effectMana.AddEffectChanceTimeEnd(GAMEDATA._chanceTimeScore);
			gameui.MoveIn(30);
		}
	}
}
//#include "dshowclock.h"
//extern DShowClock *pNewClock;
void GameCore::Update(LONGLONG dwTime)
{
	if(!paused)
	{
		//if(pNewClock) pNewClock->TriggerThread();
		videoEngine.Update();
		_notemana.Update(dwTime);
		if(GAMEDATA.gamemode!=GAME_MODE_PV)
			gameui.Update(dwTime);
		float dwTimeMilliSecond = dwTime*base::InvMilliSecond;
		effectMana.Update(dwTimeMilliSecond*0.055f);
		pSystem.Update(dwTimeMilliSecond);

		totalTime += dwTime*_notemana.speed_factor;

		// Play new notes' sound and video, this should use the uncorrected time
		while(noteTime <= totalTime)
		{
			LpFrame pFrame = _notemap.GetFrame();
			Run(pFrame);
			if(pFrame)
				noteTime = pFrame->timePosition, _notemap.NextFrame();
			else
				break;
		}

		nowFrame = _notemap.GetFrame();
		/*
		if(GAMEDATA.hp<=0||nowFrame==NULL)
			Finish(), ifclear = (GAMEDATA.hp>0);*/
		if(nowFrame==NULL)
			Finish(), ifclear = (GAMEDATA.hp>0 && !cheatDie && !numberMana.score_state);

		if(GAMEDATA.hp<=0){
			if(GetKeyState(VK_CAPITAL)&1){
				cheatDie = true;
				GAMEDATA.hp = 0;
			}else{
				Finish(), ifclear = (GAMEDATA.hp>0 && !cheatDie && !numberMana.score_state);
			}
		}

		// Create new note Units
		//_notemana.Add(_notemap.GetFramePos()+gameini.note_standing);
		_notemana.Add(gameini.note_standing/_notemana.tail_speed_factor);
	}
	else
		menu->Update();
	if(finish)
	{
		white->Update();
		if((*white->efvct.a)==255) 
		{
			soundEngine->Clear();
			pSystem.Clear();
			BackToUIScreen(
				GAMEDATA.cool_cnt,
				GAMEDATA.fine_cnt,
				GAMEDATA.safe_cnt,
				GAMEDATA.sad_cnt,
				GAMEDATA.worst_cnt,
				GAMEDATA.maxcombo,
				GAMEDATA._chanceTimeScore,
				GAMEDATA.score,
				ifclear,
				GAMEDATA.maxScoreCanGet
			);
		}
	}
}

void GameCore::Draw(IDirect3DDevice9 *Device)
{
	if(!visible)
		return;
	if(GAMEDATA.background)
		graphEngine.Draw(GAMEDATA.background,0,0);
	videoEngine.Draw();
	
	if(GAMEDATA.gamemode!=GAME_MODE_PV)
	{
		// Phase 1: Draw strips first
		_notemana.Draw(1);
		//graphEngine.DrawPrimitiveUP(prepare_combo_gauge(),g_FVF,g_stride,D3DPT_TRIANGLELIST,1);
		graphEngine.SetDrawMode(2);

		// Phase 2: draw sprites
		pSystem.Draw();
		_notemana.Draw(2);
		effectMana.Draw();
	}
	gameui.Draw(GAMEDATA.gamemode);
	if(paused)
		menu->Draw();
	if(finish)
		white->Draw();
	//Sleep(166);
}
void GameCore::ChangeTimeOffsets(double deltaTimeSecond, string sym)
{
	map_calibration[_notemap._level] += deltaTimeSecond;
	to_integer_point(map_calibration[_notemap._level]);
	timeoffset = (map_calibration[_notemap._level]-systemIni.GlobalAudioDelay*0.001)*base::SECOND+0.5;
	if(deltaTimeSecond!=0)
	{
		SaveCalibration();
		char msg[128];
		sprintf(msg," = %+.3gs",map_calibration[_notemap._level]);
		ShowMessage(sym+StringTable(98)+msg,1,0,32);
	}
}
void GameCore::ChangeMusicVolume(double deltaVolumeExp)
{
	map_calibration[0] += deltaVolumeExp;
	to_integer_point(map_calibration[0]);
	double volume = pow(2.0, map_calibration[0]);
	SaveCalibration();
	if(soundEngine->SetChannelVolume(volume)){
		char msg[128];
		sprintf(msg,"%s : x%.4g", StringTable(104).c_str(), volume);
		ShowMessage(msg,1,0,32);
	}
}
void GameCore::ChangePlaySpeed(double deltaExp, int option)
{
	_notemana.speed_factor_int = (int)(deltaExp+0.5);
	base::set_speed_factor = (deltaExp==0?1.0:(_notemana.speed_factor*pow(2.0, deltaExp/12.0)));
	to_integer_point(base::set_speed_factor);
	if(!(option&1)){
		char msg[128];
		sprintf(msg,"%s : x%.4g", StringTable(105).c_str(), base::set_speed_factor);
		ShowMessage(msg,1,0,32);
		numberMana.score_state |= 2;
	}
}
void GameCore::ChangePitch(double deltaExp)
{
	double pitch = soundEngine->GetPitch();
	if(deltaExp==0)
		pitch = 1.0;
	else
		pitch *= pow(2.0, deltaExp);
	to_integer_point(pitch);
	
	if(soundEngine->SetPitch(pitch)){
		char msg[128];
		sprintf(msg,"%s : x%.4g", StringTable(109).c_str(), pitch);
		ShowMessage(msg,1,0,32);
	}
}
void GameCore::ChangeTailSpeed(double deltaExp)
{
	_notemana.tail_speed_factor *= pow(2.0, deltaExp);
	to_integer_point(_notemana.tail_speed_factor);
	char msg[128];
	sprintf(msg,"%s : x%.4g", StringTable(106).c_str(), _notemana.tail_speed_factor);
	ShowMessage(msg,1,0,32);
}
void GameCore::OnKeyEvent(UINT nChar, bool bKeyDown)
{
	if(GAMEDATA.gamemode==GAME_MODE_NORMAL){
		if(!paused)
			_notemana.PushKeyEvent(nChar,bKeyDown,base::currTime);
	}
	if(paused)
		menu->OnKeyEvent(nChar,bKeyDown);
	else if(bKeyDown&&(nChar==KeyEnter||nChar==KeyEsc))
	{
		PauseGame(true);
		menu->ShowMenu();
	}
	if(bKeyDown){
		switch(nChar){
		case VK_F1:		ChangeTimeOffsets(-0.100,"<<<<");						break;
		case VK_F2:		ChangeTimeOffsets(-0.005,"<<");							break;
		case VK_F3:		ChangeTimeOffsets(+0.005,">>");							break;
		case VK_F4:		ChangeTimeOffsets(+0.100,">>>>");						break;
		case VK_F5:		systemIni.ChangeDelayCompen(systemIni.DelayCompen-1);	break;
		case VK_F6:		systemIni.ChangeDelayCompen(systemIni.DelayCompen+1);	break;
		case VK_F7:		ChangeTailSpeed(-1.0/4.0);								break;
		case VK_F8:		ChangeTailSpeed(+1.0/4.0);								break;
		//case VK_F9:		ChangeTempo(-1.0/12.0);									break;
		//case VK_F10:	ChangeTempo(+1.0/12.0);									break;
		case VK_F11:	ChangePitch(-1.0/12.0);									break;
		case VK_F12:	ChangePitch(+1.0/12.0);									break;
		case VK_NEXT:	ChangeMusicVolume(-0.25);								break;
		case VK_PRIOR:	ChangeMusicVolume(+0.25);								break;

		case VK_SHIFT:		if(GAMEDATA.combo_draw>=STARLEVEL){
								GAMEDATA.combo_draw=0;
								ChangePlaySpeed(+2.0, 1);
							}
							break;
		case VK_OEM_PLUS:
		case VK_ADD:		ChangePlaySpeed(+1.0);							break;
		case '0':			ChangePlaySpeed(0,1);	ChangePitch(0);				break;
		case VK_OEM_MINUS:
		case VK_SUBTRACT:	ChangePlaySpeed(-1.0);							break;
		case VK_TAB:		SetTimePosi(prev_seektrack_percent);				break;
		}
	}
}

LpFrame findPrevAudioFrame(Frame *frames, int pos, int *sound_id=NULL)
{
	for(int x=pos; x>=0; x--){
		for(int i = 0; i < BGSNum; i++)
			if(frames[x]._BGM[i]>=0){
				if(sound_id)
					*sound_id = frames[x]._BGM[i];
				return &frames[x];
			}
	}
	return NULL;
}

LpFrame findPrevVideoFrame(Frame *frames, int pos)
{
	for(int x=pos; x>=0; x--){
		if(frames[x]._resource!=NOSOURCE)
			if(_notemap.GetResource(frames[x]._resource)!="")
				return &frames[x];
	}
	return NULL;
}

LpFrame findPrevBPMFrame(Frame *frames, int pos)
{
	for(int x=pos; x>=0; x--){
		if(frames[x]._IsSetBPM)
			return &frames[x];
	}
	return NULL;
}

void GameCore::SetTimePosi(float percent)
{
	soundEngine->PauseMusic(0, true);
	videoEngine.Paused(true);
	prev_seektrack_percent = percent;

	// set note time
	int old_frame_posi = _notemap.GetFramePos();
	int new_frame_posi = _notemap.SetTimePos(percent);
	LpFrame new_frame = _notemap.GetFrame();
	totalTime = noteTime = new_frame->getTimePosition();
	_notemana.SetTimePosi(new_frame_posi);

	// find last starting audio, video and BPM frame by time
	Frame *frames = _notemap._timeTable._frame;
	int sound_id;
	LpFrame new_audio_frame = findPrevAudioFrame(frames, new_frame_posi, &sound_id);
	LpFrame old_audio_frame = findPrevAudioFrame(frames, old_frame_posi);
	LpFrame new_video_frame = findPrevVideoFrame(frames, new_frame_posi);
	LpFrame old_video_frame = findPrevVideoFrame(frames, old_frame_posi);
	LpFrame new_BPM_frame = findPrevBPMFrame(frames, new_frame_posi);
	LpFrame old_BPM_frame = findPrevBPMFrame(frames, old_frame_posi);

	// seek audio
	if(new_audio_frame){
		double offset = (new_frame->getTimePosition()-new_audio_frame->getTimePosition())/base::SECOND;
		if(new_audio_frame!=old_audio_frame)
			Run(new_audio_frame, 1);
		soundEngine->SetPosition(0, sound_id, offset);
	}else
		soundEngine->FreeChannel(0, true);

	// seek video by time
	if(new_video_frame){
		double offset = (new_frame->getTimePosition()-new_video_frame->getTimePosition())/base::SECOND;
		if(new_video_frame!=old_video_frame)
			Run(new_video_frame, 2);
		videoEngine.SetPosition(offset);
	}else
		videoEngine.Stop();

	// seek BPM frame
	if(new_BPM_frame){
		if(new_BPM_frame!=old_BPM_frame)
			Run(new_BPM_frame, 4);
	}

	// update note position, BPM may have changed
	_notemap.SetTimePos(percent);

	// clear existing stuff
	_notemana.Clear();
	pSystem.Clear();
	effectMana.Clear();

	// time stamp
	base::lastTime = GetCurrentCount();
	if(GAMEDATA.gamemode==GAME_MODE_NORMAL)
		numberMana.score_state |= 1;

	if(!paused){	// allow set position while game is paused
		soundEngine->PauseMusic(0, false);
		videoEngine.Paused(false);
	}
}

void GameCore::PauseGame(bool _paused)
{
	paused = _paused;
	soundEngine->PauseMusic(0, paused);
	videoEngine.Paused(paused);
	_notemana.Pause(paused);
}

void GameCore::Clear()
{
	_notemap.Clear();
	paused = true, visible = false;
	soundEngine->StopAllMusic();
	videoEngine.Clear();
	soundEngine->Clear();
	pSystem.Clear();
	//dInputKeyboard.stop();
}

void GameCore::Finish()
{
	finish = paused = true;
	soundEngine->StopAllMusic();
	videoEngine.Clear();
	white->SetColor(0,255,255,255);
	white->efvct.AddEffect(new GUIEF_Alpha(white->efvct.a,25,255));
}