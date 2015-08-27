#include "PSystem.h"
#include "Base.h"
#include "GameMana.h"

using namespace std;

RECT ParticleComet::srcRect = {0,0,0,0};
D3DXVECTOR3 ParticleComet::center = D3DXVECTOR3(0,0,0);
RECT ParticleTail::srcRect = {0,0,0,0};
D3DXVECTOR3 ParticleTail::center = D3DXVECTOR3(0,0,0);
Texture *particle_star = NULL;
Texture *particle_5star = NULL;
Texture *particle_star2 = NULL;
Texture *particle_sphere = NULL;
Texture *particle_square = NULL;
PSystem pSystem;
float	spin_angle_speed, spin_radius_speed, Q_fade;
int		S_fade, F_fade;
float	tail_spread1, tail_spread2;

// Comet-shaped particle
void ParticleComet::Draw()
{
	graphEngine.Draw(	_drawTex, srcRect, MakefRect(_x,_y,_size,_size),
						D3DCOLOR_ARGB(_a,_r,_g,_b), &center);
}
/*	ParticleComet.state : WORD (16-bit)
	bit 0: 0->normal, 1->random color every frame
	bit 1: 0->normal, 1->random alpha (brightness)
	bit 2: 0->normal, 1->random size
	bit 3: 0->normal, 1->normalize randomized color
	bit 4: 0->normal, 1->spinning
	bit 5: 1->Q fade
	bit 6: 1->random alpha for Q fade 
	bit 14,13: 00->stationary, 01:slow spread, 10:fast spread, 11->fast linear
	bit 15: 0->normal, 1->fade faster
*/
bool ParticleComet::Update(float dwTimeMilliSecond)
{
	// increment age count
	_age += dwTimeMilliSecond;

	// random color
	if( state & 1 )
	{	// change color every frame
		int *unit_color = UNIT_COLOR[type];
		int R=rand()&0xff+1, G=rand()&0xff+1, B=rand()&0xff+1;
		if( state & 0x0008 ){		// increase brightness
			int maxv = max(R,G);
			maxv = max(maxv,B);
			float r = 256.0f/(maxv+1);
			_r = ((int)(R*r)+unit_color[0])>>1;
			_g = ((int)(G*r)+unit_color[1])>>1;
			_b = ((int)(B*r)+unit_color[2])>>1;
		}else{
			_r = (R+unit_color[0])>>1;
			_g = (G+unit_color[1])>>1;
			_b = (B+unit_color[2])>>1;
		}
		int M = (_r+_g+_b)*0.333333f;
		float a=(float)hp/gameini.HP, b=1.0f-a;
		_r = _r*a+M*b;
		_g = _g*a+M*b;
		_b = _b*a+M*b;
	}

	// random alpha
	if( state & 2 )
		_a = (rand()&0xff);

	// random size
	if( state & 4 )
		_size = (float)rand()*(8.0f/RAND_MAX);

	// update spin
	if( state & 0x0010 )
	{
		float fRadius = sqrt(_age*spin_radius_speed);
		float fAngle = angle*0.0174533f + _age*spin_angle_speed;
		_x = dx+fRadius*cos(fAngle);
		_y = dy+fRadius*sin(fAngle);
	}

	// update motion
	if( state & 0x6000 )
	{
		short motion = (state>>13)&3;
		float mul;
		if(motion==1) mul = sqrt(tail_spread1*_age);
		else mul = tail_spread2*_age;
		_x = ox+dx*mul;
		_y = oy+dy*mul;
	}

	// Q fade
	if( state & 0x0020 )
	{
		_a = 255+_age*_age*Q_fade;
		if(_a<0) _a=0;
		else if(state&0x0040) _a = ((768+_a)>>2)*(float)rand()/RAND_MAX;
	}
	else
	{
		// update fading
		_a += (state&0x8000)?F_fade:S_fade;
		if(_a<0) _a=0;
	}


	// spinning particle, check whether outside screen range
	//if( state & 0x0010 )
	//	return _age<(551.7f/0.3f);

	return (_a!=0);
}
bool ParticleComet::Die()
{
	return (!_a);
}

// Single strip particle
void ParticleTail::Draw()
{
	graphEngine.Draw(_drawTex,srcRect,MakefRect(_x,_y,_size,_size),D3DCOLOR_ARGB(_a,_r,_g,_b),&center);
}
bool ParticleTail::Update(float dwTimeMilliSecond)
{
	_age += dwTimeMilliSecond;
	_a -= (_age>145?4:2);
	if(_a<0)_a=0;
	return (_a!=0);
}
bool ParticleTail::Die()
{
	return (!_a);
}


void PSystem::Draw()
{
	//drawSP();
	//return;
	IDirect3DStateBlock9 *pRender_state;
	if(systemIni.particle>=3){
		base::Device->CreateStateBlock(D3DSBT_ALL,&pRender_state );

		base::Sprite->Flush();
		base::Device->SetRenderState( D3DRS_LIGHTING,  FALSE );
		base::Device->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
		base::Device->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
		base::Device->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		base::Device->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		//base::Device->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );
	}
	if(systemIni.particle>0){
		for each (auto &group in m_particle_groups){
			for each (auto &element in group.second){
				element->Draw();
			}
		}
	}
	if(systemIni.particle>=3){
		base::Sprite->Flush();
		pRender_state->Apply();
		pRender_state->Release();
	}
}

void PSystem::Update(float dwTimeMilliSecond)
{
	// Do particle pruning
	if(dwTimeMilliSecond<min_time_interval)
		min_time_interval = dwTimeMilliSecond;
	// collect stats
	vector <pair<int,int>> particles_per_unit;
	int N = 0;
	for(auto group_it = m_particle_groups.begin(); group_it!=m_particle_groups.end(); ++group_it){
		if(group_it->first&0x1){
			int n = group_it->second.size();
			particles_per_unit.push_back(make_pair(group_it->first,n));
			N+=n;
		}
	}
	if(dwTimeMilliSecond<min_time_interval*FPS_prune_threashold){	// decide not to prune
		if(N>max_num_particles) max_num_particles = N;
	}else{	// decide to prune
		for(;N>max_num_particles;N--){
			int n = rand()%N, i = 0;
			while(n>=particles_per_unit[i].second){
				n-=particles_per_unit[i].second;
				i++;
			}
			auto &group = m_particle_groups[particles_per_unit[i].first];
			delete group[n];
			group[n] = group[--particles_per_unit[i].second];
		}
		for each(auto &p in particles_per_unit){
			if(p.second==0)
				m_particle_groups.erase(p.first);
			else
				m_particle_groups[p.first].resize(p.second);
		}
	}

	// update all particles
	for (auto group_it = m_particle_groups.begin(); group_it!=m_particle_groups.end();){
		auto &group = group_it->second;
		for ( int n=0, N=group.size(); n<N; n++ ){
			if(!group[n]->Update(dwTimeMilliSecond)){
				delete group[n];
				group[n] = group[--N];
				group.resize(N);
				n--;
			}
		}
		if(group.empty())
		{
			group_it = m_particle_groups.erase(group_it);
			continue;
		}
		++group_it;
	}
}

void PSystem::Clear()
{
	for each (auto group in m_particle_groups){
		for each (auto element in group.second){
			delete element;
		}
	}
	m_particle_groups.clear();
}

void PSystem::Clear(int group)
{
	auto &particle_group = m_particle_groups[group];
	for each (auto i in particle_group){
		delete i;
	}
	m_particle_groups.erase(group);
}

void PSystem::SetState(int state_mask)
{
	for each (auto &group in m_particle_groups)
	{
		for each (auto &element in group.second)
			((ParticleComet*)element)->state |= state_mask;
	}
}

void PSystem::SetState(int state_mask, int group)
{
	auto &particle_group = m_particle_groups[group];
	for each (auto &element in particle_group){
		((ParticleComet*)element)->state |= state_mask;
	}
}

void PSystem::AddParticle(ParticleBase *p,int group)
{
	auto &particle_group = m_particle_groups[group];
	if(particle_group.empty()) particle_group.reserve(256);
	particle_group.push_back(p);
}

void PSystem::Init()
{
	D3DSURFACE_DESC desc;
	particle_star = graphEngine.AddTexture("pic\\particle_star.png");
	particle_star2 = graphEngine.AddTexture("pic\\particle_star2.png");
	particle_5star = graphEngine.AddTexture("pic\\particle_5star.png");
	particle_sphere = graphEngine.AddTexture("pic\\particle_sphere.png");
	particle_sphere->GetLevelDesc(0,&desc);
	RECT rect = {0,0,desc.Width,desc.Height};
	ParticleComet::srcRect = rect;
	ParticleComet::center = D3DXVECTOR3(desc.Width*0.5f,desc.Height*0.5f,0);

	particle_square = graphEngine.AddTexture("pic\\block.png");
	particle_square->GetLevelDesc(0,&desc);
	RECT rect2 = {0,0,desc.Width,desc.Height};
	ParticleTail::srcRect = rect2;
	ParticleTail::center = D3DXVECTOR3(desc.Width*0.5f,desc.Height*0.5f,0);

	spin_angle_speed = *((FloatResource*)g_res.getResource("spin_angle_speed"))->value;
	spin_radius_speed = *((FloatResource*)g_res.getResource("spin_radius_speed"))->value;
	Q_fade = *((FloatResource*)g_res.getResource("Q_fade"))->value;
	S_fade = *((IntResource*)g_res.getResource("S_fade"))->value;
	F_fade = *((IntResource*)g_res.getResource("F_fade"))->value;
	tail_spread1 = *((FloatResource*)g_res.getResource("tail_spread1"))->value;
	tail_spread2 = *((FloatResource*)g_res.getResource("tail_spread2"))->value;
	FPS_prune_threashold = *((FloatResource*)g_res.getResource("FPS_prune_threashold"))->value;
}
