#ifndef __PSystemH__
#define __PSystemH__

#include "defines.h"
#include "Base.h"
#include <map>
#include <list>
#include <vector>

using namespace std;


class ParticleBase
{
protected:
	float	_age;
	float	_x,_y,_size;
	Texture *_drawTex;

public:
	ParticleBase(float x,float y,Texture *drawTex):_age(0),_x(x),_y(y),_drawTex(drawTex){}
	virtual void Draw() = NULL;
	virtual bool Update(float) = NULL;	// return false if die
	virtual bool Die() = NULL;
};

class ParticleTail : public ParticleBase
{
private:
	int _a,_r,_g,_b;

public:
	static RECT srcRect;
	static D3DXVECTOR3 center;

	ParticleTail(int x,int y,int a,int *rgb,Texture* drawTex,int size)
		:ParticleBase(x,y,drawTex),_a(a),_r(rgb[0]),_g(rgb[1]),_b(rgb[2]){
			_size = size;
	}

	void Draw();
	bool Update(float);	// return false if die
	bool Die();
};

class ParticleComet : public ParticleBase
{
private:
	int		_a,_r,_g,_b;
	float	dx,dy,ox,oy;

public:
	static RECT srcRect;
	static D3DXVECTOR3 center;

	BYTE	hp, mp, type, preserve;
	short	angle, state;

	ParticleComet(float x,float y,int a,int *rgb,Texture* drawTex,
		float _dx, float _dy, float size, short _type, short _state, BYTE _hp, BYTE _mp, short _angle)
		:ParticleBase(x,y,drawTex),_a(a),_r(rgb[0]),_g(rgb[1]),_b(rgb[2]),
		dx(_dx),dy(_dy),ox(x),oy(y),hp(_hp),mp(_mp),angle(_angle){
			type = _type;
			state = _state;
			_size = size;
	}

	void Draw();
	bool Update(float);	// return false if die
	bool Die();		// return true if die
};

/*
class ParticleStar : public ParticleBase
{
private:
	//state:0~6->normal, 1->normal
	int		_a,_r,_g,_b;
	float	dx,dy,size;

public:
	static RECT srcRect;
	static D3DXVECTOR3 center;
	ParticleStar(int x,int y,int a,int *rgb,Texture* drawTex, float _dx, float _dy, float _size, int _type, short _state)
		:ParticleBase(x,y,drawTex),_a(a),_r(rgb[0]),_g(rgb[1]),_b(rgb[2]),dx(_dx),dy(_dy),size(_size){
			type = _type;
			state = _state;
	}

	void Draw();
	bool Update();	// return false if die
	bool Die();		// return true if die
};
*/

class PSystem
{
private:
	int max_num_particles;
	float min_time_interval;
	float FPS_prune_threashold;
	map < int, vector<ParticleBase*> > m_particle_groups;

public:
	PSystem(){max_num_particles=0; min_time_interval=1000.0f;}
	~PSystem(){Clear();}

	void Init();
	void Draw();
	void Update(float);

	void AddParticle(ParticleBase* p, int group);

	void Clear();
	void Clear(int group);

	void SetState(int state_mask);
	void SetState(int state_mask, int group);
};

extern Texture *particle_star;
extern Texture *particle_5star;
extern Texture *particle_star2;
extern Texture *particle_sphere;
extern Texture *particle_square;
extern PSystem pSystem;

/*/

class ParticleNode
{
public:
	ParticleNode(){prev=next=NULL;data=NULL;}
	~ParticleNode(){SAFE_DELETE(data);}
	ParticleNode(ParticleNode *pprev,ParticleNode *pnext,ParticleBase *pdata):prev(pprev),next(pnext),data(pdata){}
	ParticleBase *data;
	ParticleNode *prev,*next;
};

class PSystem
{
	PSystem(){}
	~PSystem(){Clear();}

	//ParticleNode *head,*tail;
	map<int,ParticleNode*> head;
	map<int,ParticleNode*> tail;

public:
	static PSystem& Instance(){static PSystem instance;return instance;}

	void Draw();
	void Update();

	void AddParticle(ParticleBase* p,int group=0);

	void Clear();
	void Clear(int group);
};
*/

#endif