#pragma once

#define _USE_MATH_DEFINES

// Main Rubic cube data
#include <vector>
#include <set>
#include <map>
#include <wtypes.h>
#include <d3dx9.h>
#include <d3d9.h>
#include <math.h>

using namespace std;

#define DEFAULTORDER	7

// Graphic data
#define GAPSIDERATIO	0.0

// Standard RGB colors
#define	RED		0x00ff0000
#define	GREEN	0x0000ff00
#define	BLUE	0x000000ff
#define	YELLOW	0x00ffff00
#define	ORANGE	0x0000ffff
#define	PURPLE	0x00ff00ff
#define	WHITE	0x00ffffff

#define	RED1	0x00010000
#define	GREEN1	0x00000100
#define	BLUE1	0x00000001
#define	YELLOW1	0x00010100
#define	ORANGE1	0x00000101
#define	PURPLE1	0x00010001
#define	WHITE1	0x00010101

// Definition of 4 orientations
enum DIR {UP, RIGHT, DOWN, LEFT, DIR_N};

// Definition of 6 faces
enum SIDE {RIGHTSIDE, TOPSIDE, FRONTSIDE, LEFTSIDE, BOTTOMSIDE, BACKSIDE, SIDE_N};

// Definition of 6 axis
static D3DXVECTOR3 axes[6]={
	D3DXVECTOR3(1,0,0),	D3DXVECTOR3(0,1,0),	D3DXVECTOR3(0,0,1),
	D3DXVECTOR3(-1,0,0),D3DXVECTOR3(0,-1,0),D3DXVECTOR3(0,0,-1),
};

static DWORD Vertex_format = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
static DWORD g_side_colors[6] = {RED,GREEN,BLUE,YELLOW,ORANGE,PURPLE};
static char g_face_index[6][4]={
	{5,1,2,6}, //RIGHTSIDE
	{7,6,2,3}, //TOPSIDE
	{4,5,6,7}, //FRONTSIDE
	{0,4,7,3}, //LEFTSIDE
	{5,4,0,1}, //BOTTOMSIZE
	{1,0,3,2}, //BACKSIDE
};
static float g_text_coord[4][2]={{1,1},{0,1},{0,0},{1,0}};
static float g_alpha = 0.6f;
static float g_rotate_speed = 180; // degrees per second
static float g_min_angle_continue_rotate = 10;

static float Degree2Radian(float deg){	return deg*(M_PI/180.0f);}
static float Radian2Degree(float rad){	return 180*(rad/M_PI);}


class Vertex{
public:
	float xyz[3];
	DWORD color;
	float tex_xy[2];
};

typedef struct{
	Vertex verts[3];
}Triangle;

typedef struct{
	int index;
	int orient;
}State;

typedef struct{
	int axis;
	int slice;
}Move;

class Cube{
public:
	int		orient;							// orientation
	int		index;							// original position in the whole Rubix

	// side_mask: bit 012, XYZ start side visible; bit 345, XYZ end side visible
	DWORD	draw_mask, side_mask;
	FLOAT	radius;
	D3DXVECTOR3 center, center_orig;
	D3DXVECTOR3 dir0, dir1;

	static map <int,int> side_mask_to_compfunc;

	//Vertex vertices_prev[8];				// the 8 vertices previous
	//Vertex vertices_now[8];					// the 8 vertices now
	vector <Vertex> box_buffer_prev;		// selection box previous
	vector <Vertex> box_buffer_now;			// selection box now
	vector <Vertex> vertex_buffer_prev;		// previous coordinate
	vector <Vertex> vertex_buffer_now;		// now coordinate

	void rotate(D3DXMATRIX &trans);
	void rotate_angle(D3DXMATRIX &trans);
	void highlight(int alpha);				// highest bit=1: relative; =0: absolute
	void draw(vector <Vertex> &vertex_buffer);
	void create_box();
	void clear_box();
	bool is_boxed();
	State get_state(){
		State s={index,orient};
		return s;
	}

	// side_mask: bit 012, XYZ start side visible; bit 345, XYZ end side visible
	Cube(float *center, float radius, DWORD side_mask, int posi_index);
	Cube(){}
	~Cube();

	// if intersect, return distance from intersect point to eye (and output face axis), otherwise, return -1
	float	IntersectDist(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &dir) const;
	D3DXVECTOR3 IntersectFaceNormal(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &dir) const;
	D3DXVECTOR3 IntersectFacePoint(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &dir) const;
	void	Reset();
	int		getDir();
	bool	isSolved(Cube &cube_ref, bool directional=true);
};

class Rubix{
public:
	int		order;
	int		directional;
	int		solid;
	int		history_posi;
	vector	<DWORD>	history;
	vector	<Cube*>	cubes;
	set	<int>	highlighted_indices;
	set	<int>	boxed_indices;

	Rubix(int order=DEFAULTORDER, bool directional=true, bool solid=true);
	~Rubix();
	void	release();
	int		Draw(vector <Vertex> &vertex_buffer, int mask);	// mask bit 0: don't draw if out of position

	// if intersect, return cube index, otherwise, return -1
	int		Intersect(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &dir, int layer=-1);

	// start from outer-most layer with index = 0
	int		GetInwardLayerIndex(int x, int y, int z);
	int		GetInwardLayerIndex(int posi);

	bool	isSolved();
	void	ClearHighlight();
	void	RotateSlice(int axis, int slice, int angle, int history_shift, bool bFinalize);	// angle in degrees
	void	SelectSlice(int axis, int slice, int *x, int *y, int *z, int **px, int **py);
	void	CubeIndex2XYZ(int index, int &x, int &y, int &z);
	void	HighlightOne(int id, bool clear=true);
	void	HighlightSlice(int axis, int slice);
	void	HighlightLayer(int layer, int alphaA, int alphaB);
	void	HighlightAll(int alpha);
	void	Shuffle(int seed=rand());

	void	create_box(const vector <int> &indices);
	void	clear_box();
	void	get_state(vector <State> &out);

	void	save(char* filename);
	void	load(char* filename);
};

