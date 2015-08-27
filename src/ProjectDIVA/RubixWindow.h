#pragma once

#include <vector>
#include <set>
#include <wtypes.h>
#include <d3dx9.h>
#include <d3d9.h>
#include <math.h>
#include "Rubix.h"

using namespace std;

class Animator;
class Solver;
class RubixWindow{
	friend class Animator;

	IDirect3DDevice9 *Device;
	HWND	hWnd;
	Rubix	*pRubix;
	Solver	*pSolver;
	Animator *pAnimator;
	IDirect3DTexture9	*pTex_arrow, *pTex_square;
	D3DXMATRIX		viewMat, projMat;
	vector <Vertex> vertex_buffer, vertex_buffer2;

	DWORD	mouse_button_state;	// bit0:left, 1:right, 2:middle
	short	mouse_l_x, mouse_l_y;
	short	mouse_ld_x, mouse_ld_y;
	short	mouse_r_x, mouse_r_y;
	short	mouse_m_x, mouse_m_y;
	float	distance, viewAngle;
	float	Xrot, Yrot, Zrot;
	float	H_shift, V_shift;
	float	rotation_angle;		// -1: not rotated, 0~90: rotation angle
	float	rotation_angular_displacement;
	double	animator_time;
	int		texture_name;
	int		Width, Height;
	int		BMP_width;
	int		selected_cube_index;
	int		selected_cube_face;
	int		selected_axis;
	int		selected_XYZ[3];
	int		highlighted_layer, highlight_status;
	int		dirty;
	int		undo_redo;
	bool	blink, blink_status, use_distance_alpha;

	void	UpdateMatrix();
	BYTE*	m_LoadBitmap(char*,int*,int*);
	bool	m_InitScene(Rubix*);
	void	SortVertices(vector <Vertex> &vertex_buffer);
	void	VertexAlphaDistance(vector <Vertex> &output_buffer, const vector <Vertex> &in_buffer);
	void	ComputeRay(D3DXVECTOR3 &pOut, float x, float y);	// -1<=x,y<=1
	void	Simulate1Step(int axis, int slice);
	int		StepHistory(int steps);
	int		GotoHistory(int steps);
	int		DetectCollision(int mouse_hit_x, int mouse_hit_y);
	int		DetectRotation(int mouse_hit_x, int mouse_hit_y);
	int		FindAxis(D3DXVECTOR3 &v);

public:
	D3DXVECTOR3		eyev, upv, centerv, last_hit_dirv;

	RubixWindow();
	~RubixWindow();

	void	(*OnReturn)();
	LRESULT WndProc(HWND, UINT, WPARAM ,LPARAM);
	BOOL CALLBACK m_DialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void	Init(HWND hwnd, IDirect3DDevice9 *pDevice, HWND *hDlg=NULL);
	void	Create(int order, bool directional, bool solid);
	void	Update(double time_ms, int stack=0);
	void	Draw();
	void	StartAnimation(Animator *pAni);
	Rubix	*getRubix(){return pRubix;}
};

// interface to be implemented for animation
// by default, it will roll back to 1st step
class Animator{
	friend class RubixWindow;

public:
	RubixWindow *pRubixWindow;
	virtual void init(){}
	virtual void release(){delete this;}

	// core function: return time elapse in ms, return <0 to terminate the animation
	// angle can only be -90, 90, 180 and 0 (to avoid further rotation action (already done implicitly))
	virtual double nextMove(int &axis, int &slice, int &angle);

	Animator(RubixWindow *p){ pRubixWindow = p;	}
	virtual ~Animator(){}
};

class Solver{
public:
	virtual int Solve(vector<Move> &output_moves, vector<State> input_state, int order){return 0;}
};


