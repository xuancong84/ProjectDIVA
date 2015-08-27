#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <cassert>
#include "rubix.h"
#include "resource.h"
#include "vect_sse.h"
#include "d3dmath.h"
#include "base.h"
#include "defines.h"
#include "UIScreen.h"
#include "rubix_solver_orbit.h"
#include "RubixWindow.h"

char filename[1024];
OPENFILENAME g_ofn={
	sizeof(OPENFILENAME),NULL,hInst,
	"Rubix Files (*.rubix)\0*.rubix\0All Files (*.*)\0*.*\0",NULL,0,0,
	filename,256,NULL,0,NULL,"Open a rubix file",
	OFN_FILEMUSTEXIST|OFN_LONGNAMES|OFN_PATHMUSTEXIST|OFN_NOREADONLYRETURN|OFN_NOCHANGEDIR,
	0,0,"rubix",0,NULL,NULL
};
OPENFILENAME g_sfn={
	sizeof(OPENFILENAME),NULL,hInst,
	"Rubix Files (*.rubix)\0*.rubix\0All Files (*.*)\0*.*\0",NULL,0,0,
	filename,256,NULL,0,NULL,"Save As...",
	OFN_LONGNAMES|OFN_OVERWRITEPROMPT|OFN_NOCHANGEDIR,0,0,"rubix",0,NULL,NULL
};

RubixWindow *g_win;
BOOL CALLBACK DialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{ return g_win->m_DialogProc(hDlg, uMsg, wParam, lParam); }

int distFunc_fast(const void *lhs, const void *rhs)
{
	D3DXVECTOR3 viewv = base::pRubixWindow->centerv-base::pRubixWindow->eyev;
	Vertex *pL = (Vertex*)lhs;
	Vertex *pR = (Vertex*)rhs;
	D3DXVECTOR3 L = (*(D3DXVECTOR3*)pL[0].xyz + *(D3DXVECTOR3*)pL[1].xyz + *(D3DXVECTOR3*)pL[2].xyz);
	D3DXVECTOR3 R = (*(D3DXVECTOR3*)pR[0].xyz + *(D3DXVECTOR3*)pR[1].xyz + *(D3DXVECTOR3*)pR[2].xyz);
	float distL = D3DXVec3Dot(&L, &viewv);
	float distR = D3DXVec3Dot(&R, &viewv);
	return distL>distR?-1:1;
}

int distFunc_accurate(const void *lhs, const void *rhs)
{
	D3DXVECTOR3 viewv = base::pRubixWindow->centerv-base::pRubixWindow->eyev;
	Vertex *pL = (Vertex*)lhs;
	Vertex *pR = (Vertex*)rhs;
	D3DXVECTOR3 L = findLongestEdgeCenter(*(D3DXVECTOR3*)pL[0].xyz,*(D3DXVECTOR3*)pL[1].xyz,*(D3DXVECTOR3*)pL[2].xyz);
	D3DXVECTOR3 R = findLongestEdgeCenter(*(D3DXVECTOR3*)pR[0].xyz,*(D3DXVECTOR3*)pR[1].xyz,*(D3DXVECTOR3*)pR[2].xyz);
	float distL = D3DXVec3Dot(&L, &viewv);
	float distR = D3DXVec3Dot(&R, &viewv);
	return distL>distR?-1:1;
}

int (*distFunc)(const void *lhs, const void *rhs)=&distFunc_accurate;

RubixWindow::RubixWindow()
{
	pSolver = NULL;
	pRubix = NULL;
	pAnimator = NULL;
	OnReturn = NULL;
}
RubixWindow::~RubixWindow()
{
	if(base::hDlg){
		HWND hDlg = base::hDlg;
		base::hDlg = NULL;
		DestroyWindow(hDlg);
	}
	SAFE_DELETE(pAnimator);
	SAFE_DELETE(pRubix);
	SAFE_DELETE(pSolver);
	SAFE_RELEASE(pTex_arrow);
	SAFE_RELEASE(pTex_square);
}

void RubixWindow::Simulate1Step(int axis, int slice)
{
	rotation_angle = 1;
	selected_axis = axis;
	selected_XYZ[selected_axis%3] = slice;
	rotation_angular_displacement = 0;
}

int RubixWindow::StepHistory(int steps)
{
	int x, inc = steps>0?1:-1;
	for(x=0; x!=steps; x+=inc){
		Update(1e10, 1);
		int new_history_posi = pRubix->history_posi+(inc>0?0:-1);
		if(new_history_posi<0 || new_history_posi>=pRubix->history.size())
			break;
		DWORD move = pRubix->history[new_history_posi];
		undo_redo = inc;
		Simulate1Step(inc>0?(move>>16):(((move>>16)+3)%6), move&0xffff);
	}
	return x;
}

int RubixWindow::GotoHistory(int hist_posi)
{
	if(hist_posi<0 || hist_posi>=pRubix->history.size())
		return 0;
	return StepHistory(hist_posi-pRubix->history_posi);
}

LRESULT RubixWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	LONG	retval = 1;
	int		acc = 0;
	switch(uMsg){
	case WM_MOUSEMOVE:
		if(wParam & MK_LBUTTON){
			if(selected_cube_index>=0 && !pAnimator){		// mouse rotate a slice
				float dx = mouse_l_x-(short)LOWORD(lParam);
				float dy = mouse_l_y-(short)HIWORD(lParam);
				//if(dx*dx+dy*dy>100 && rotation_angle!=90)	// mouse drag for at least 10 screen pixels
				if(dx*dx+dy*dy>100)	// mouse drag for at least 10 screen pixels
					DetectRotation((short)LOWORD(lParam), (short)HIWORD(lParam));
			}else{							// mouse rotate camera
				Xrot	=	Degree2Radian((float)mouse_l_x-(short)LOWORD(lParam));
				Yrot	=	Degree2Radian((float)((short)HIWORD(lParam))-mouse_l_y);
				mouse_l_x =	(short)LOWORD(lParam);
				mouse_l_y =	(short)HIWORD(lParam);
				UpdateMatrix();
			}
		}else if(wParam & MK_RBUTTON){
			if(mouse_r_x)
				distance	*=	1.0f+((float)((short)HIWORD(lParam))-mouse_r_y)/Height;
			viewAngle	*=	1.0f+((float)((short)LOWORD(lParam))-mouse_r_x)/Width;
			mouse_r_x = (short)LOWORD(lParam);
			mouse_r_y = (short)HIWORD(lParam);
			UpdateMatrix();
		}else if(wParam & MK_MBUTTON){
			H_shift	-=	mouse_m_x-(short)LOWORD(lParam);
			V_shift	+=	((short)HIWORD(lParam))-mouse_m_y;
			mouse_m_x = (short)LOWORD(lParam);
			mouse_m_y = (short)HIWORD(lParam);
			UpdateMatrix();
		}
		break;
	case WM_MOUSEWHEEL:
		Zrot = Degree2Radian(-(float)((short)HIWORD(wParam))/30.0f);
		UpdateMatrix();
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		mouse_ld_x = mouse_l_x = (short)LOWORD(lParam);
		mouse_ld_y = mouse_l_y = (short)HIWORD(lParam);
		mouse_button_state |= 1;
		if(!pAnimator){
			if(rotation_angle!=-1)
				Update(1e10,1);
			selected_cube_index = DetectCollision(mouse_l_x, mouse_l_y);
		}
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		selected_cube_index = -1;
		mouse_button_state &= (0xffffffff^1);
		retval = 1;
		if(mouse_ld_x==LOWORD(lParam) && mouse_ld_y==HIWORD(lParam))
			if(GetAsyncKeyState(VK_CONTROL)>=0 && GetAsyncKeyState(VK_SHIFT)>=0)
				if(!pRubix->boxed_indices.empty()){
					pRubix->clear_box();
					dirty = max(dirty, 2);
				}
				break;
	case WM_RBUTTONDOWN:
		SetCapture(hWnd);
		mouse_r_x = (short)LOWORD(lParam);
		mouse_r_y = (short)HIWORD(lParam);
		mouse_button_state |= 2;
		break;
	case WM_RBUTTONUP:
		ReleaseCapture();
		mouse_button_state &= (0xffffffff^2);
		retval=1;
		break;
	case WM_MBUTTONDOWN:
		SetCapture(hWnd);
		mouse_m_x = (short)LOWORD(lParam);
		mouse_m_y = (short)HIWORD(lParam);
		mouse_button_state |= 4;
		break;
	case WM_MBUTTONUP:
		ReleaseCapture();
		mouse_button_state &= (0xffffffff^4);
		retval=1;
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		Width = LOWORD(lParam);
		Height = HIWORD(lParam);
		UpdateMatrix();
		break;
	case WM_KEYDOWN:
		switch(wParam){
		case VK_ESCAPE:
			if(pAnimator){
				SAFE_DELETE(pAnimator);
				Update(1e10);
			}else{
				if(OnReturn)
					OnReturn();
				RubixWindow *pwin = base::pRubixWindow;
				base::pRubixWindow = NULL;
				delete pwin;
			}
			break;
		case VK_BACK:
		case VK_LEFT:
			if(pAnimator) break;
			StepHistory(-1);
			break;
		case VK_RIGHT:
			if(pAnimator) break;
			StepHistory(1);
			break;
		case VK_F2:	// blink
			blink = !blink;
			break;
		case VK_F4:	// shuffle
			if(pAnimator) break;
			pRubix->Shuffle(GetTickCount());
			dirty=2;
			break;
		case VK_F5:	// reset
			if(pAnimator) break;
			Create(pRubix->order, pRubix->directional, pRubix->solid);
			break;
		case VK_F6:
			break;
		case VK_OEM_3:
			highlighted_layer = -1;
			pRubix->HighlightAll((int)(255*g_alpha+0.5f));
			dirty=2;
			break;
		case VK_PRIOR:
			g_rotate_speed *= 1.2f;
			break;
		case VK_NEXT:
			g_rotate_speed /= 1.2f;
			break;
		case VK_ADD:
		case VK_OEM_PLUS:
			acc = highlighted_layer-1;
			goto HighLight;
		case VK_SUBTRACT:
		case VK_OEM_MINUS:
			acc = highlighted_layer+1;
			goto HighLight;
		case '0':  ++acc;
		case '9':  ++acc;
		case '8':  ++acc;
		case '7':  ++acc;
		case '6':  ++acc;
		case '5':  ++acc;
		case '4':  ++acc;
		case '3':  ++acc;
		case '2':  ++acc;
		case '1':
HighLight:
			if(acc<(pRubix->order+1)/2 && acc>=0){
				if(acc==highlighted_layer){
					highlight_status = !highlight_status;
					pRubix->HighlightLayer(acc, 0xff, highlight_status?0x0:0x1f);
				}else{
					highlighted_layer = acc;
					highlight_status = 0;
					pRubix->HighlightLayer(acc,0xff,0x1f);
				}
				dirty=max(dirty,2);
			}
			break;
		}
		break;
	default:
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}

	return	retval;
}//Window procedure

BOOL CALLBACK RubixWindow::m_DialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	BOOL a = FALSE;
	int order;
	static int fields[][2]={
		{IDS_help, 115},
		{IDS_order, 116},
		{IDC_dim, 117},
		{IDS_drawmode, 118},
		{ID_rollback, 119},
		{ID_solve_orbit, 120},
		{IDR_drawfast, 121},
		{IDR_drawacc, 122},
		{IDOK, 123},
		{ID_save, 124},
		{ID_load, 125},
		{ID_shuffle, 126},
		{ID_goto, 128},
		{IDC_dir, 129},
		{IDC_solid, 130},
	};

	switch(uMsg){
	case WM_INITDIALOG:
		Init:
		SetWindowTitle(hDlg, StringTable(114).c_str());
		for(int x=0, X=sizeof(fields)/sizeof(int)/2; x<X; x++)
			SetDlgItemTextW(hDlg, fields[x][0], Ansi2WChar(StringTable(fields[x][1])).data());
		SetDlgItemInt(hDlg, IDC_gotohist, pRubix->history_posi, false);
		SetDlgItemInt(hDlg, IDC_order, pRubix->order, false);
		CheckDlgButton(hDlg, IDR_drawacc, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_dir, pRubix->directional?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_solid, pRubix->solid?BST_CHECKED:BST_UNCHECKED);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDOK:
			order = GetDlgItemInt(hDlg, IDC_order, &a, false);
			if(a && order>0)
				Create(order, pRubix->directional, pRubix->solid);
			else
				SetDlgItemInt(hDlg, IDC_order, pRubix->order, false);
			goto Init;
		case ID_rollback:
			StartAnimation(new Animator(this));
			break;
		case ID_solve_orbit:
			StartAnimation(new OrbitSolver(this));
			break;
		case IDC_dim:
			use_distance_alpha = (IsDlgButtonChecked(hDlg, IDC_dim)==BST_CHECKED);
			dirty = 2;
			break;
		case IDC_dir:
			pRubix->directional = (IsDlgButtonChecked(hDlg, IDC_dir)==BST_CHECKED);
			break;
		case IDC_solid:
			pRubix->solid = (IsDlgButtonChecked(hDlg, IDC_solid)==BST_CHECKED);
			dirty = max(dirty,2);
			break;
		case IDR_drawfast:
			if(IsDlgButtonChecked(hDlg, IDR_drawfast)==BST_CHECKED)
				distFunc = &distFunc_fast, dirty=2;
			break;
		case IDR_drawacc:
			if(IsDlgButtonChecked(hDlg, IDR_drawacc)==BST_CHECKED)
				distFunc = &distFunc_accurate, dirty=2;
			break;
		case ID_save:
			g_sfn.hwndOwner = hDlg;
			if(GetSaveFileName(&g_sfn))
				pRubix->save(filename);
			break;
		case ID_load:
			g_ofn.hwndOwner = hDlg;
			if(GetOpenFileName(&g_ofn)){
				pRubix->load(filename);
				dirty = max(dirty,2);
				goto Init;
			}
			break;
		case ID_shuffle:
			if(pAnimator) break;
			pRubix->Shuffle(GetTickCount());
			dirty=max(dirty,2);
			break;
		case ID_goto:
			order = GetDlgItemInt(hDlg, IDC_gotohist, &a, false);
			if(a)
				GotoHistory(order);
			break;
		default:
			return 0;
		}
		break;
	default:
		return 0;
	}
	return 1;
}

void RubixWindow::Create(int order, bool directional, bool solid)
{
	SAFE_DELETE(pRubix);
	pRubix = new Rubix(order, directional, solid);

	SAFE_DELETE(pSolver);
	pSolver = new OrbitSolver(this);

	upv	= D3DXVECTOR3(0,1,0);
	eyev = D3DXVECTOR3(0,0,4);
	centerv = D3DXVECTOR3(0,0,0);
	viewAngle	=	Degree2Radian(30.0f);
	distance	=	D3DXVec3Length(&eyev);
	Xrot		=	Yrot	=	Zrot	=	H_shift	=	V_shift	=	0;
	selected_cube_index = -1;
	selected_axis = -1;
	rotation_angle = -1;
	highlighted_layer = -1;
	dirty = 2;
	mouse_button_state = 0;
	blink = false;
	UpdateMatrix();
}

void RubixWindow::Init(HWND hwnd, IDirect3DDevice9 *pDevice, HWND *hDlg)
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	Width = rect.right-rect.left;
	Height = rect.bottom-rect.top;
	Device = pDevice;
	hWnd = hwnd;
	use_distance_alpha = false;
	undo_redo = 0;

	D3DXCreateTextureFromResource(pDevice, NULL, (char*)IDB_ARROW, &pTex_arrow);
	D3DXCreateTextureFromResource(pDevice, NULL, (char*)IDB_SQUARE, &pTex_square);

	// create Rubic cube object
	Create(DEFAULTORDER, true, true);

	// create dialog
	if(hDlg){
		g_win = this;
		*hDlg = CreateDialog( base::hInst, (char*)IDD_rubic, hwnd, (DLGPROC)&DialogProc );
		ShowWindow(*hDlg, SW_SHOW);
	}
}

void RubixWindow::UpdateMatrix(){
	D3DXVECTOR3	newupv, neweyev, newrightv, newcenterv;
	D3DXVECTOR3	rightv;
	D3DXMATRIX view_mat, mat;

	if(!Width || !Height)
		return;

	// obtain rightv w.r.t origin
	D3DXVec3Cross(&rightv, &upv, &eyev);
	D3DXVec3Normalize(&rightv, &rightv);

	// shift centerv according to H_shift and V_shift
	newupv = upv*(V_shift*distance/Width);
	newrightv = rightv*(H_shift*distance/Height);
	centerv += (newupv-newrightv);

	// obtain rotation matrix w.r.t origin
	D3DXMatrixRotationAxis(&view_mat, &upv, Xrot);
	D3DXMatrixRotationAxis(&mat, &rightv, -Yrot);
	view_mat = mat*view_mat;
	D3DXMatrixRotationAxis(&mat, &eyev, Zrot);
	view_mat = mat*view_mat;

	// rotate eyev w.r.t origin
	D3DXVec3TransformCoord(&neweyev, &eyev, &view_mat);
	D3DXVec3TransformCoord(&newcenterv, &centerv, &view_mat);
	D3DXVec3TransformCoord(&newupv, &upv, &view_mat);

	// set eyev distance from origin
	eyev = neweyev*(distance/D3DXVec3Length(&neweyev));
	D3DXVec3Normalize(&upv, &newupv);
	centerv = newcenterv;

	// set matrices
	D3DXMatrixLookAtRH( &viewMat, &eyev, &centerv, &upv );
	D3DXMatrixPerspectiveFovRH( &projMat, viewAngle, (float)Width/Height, 0.1f, 10.0f);
	Xrot = Yrot = Zrot = H_shift = V_shift = 0;

	dirty = max(dirty,1);
}

void RubixWindow::SortVertices(vector <Vertex> &vertex_buffer)
{
	qsort(vertex_buffer.data(), vertex_buffer.size()/3, sizeof(Triangle), distFunc);
}

void RubixWindow::VertexAlphaDistance(vector <Vertex> &out_buffer, const vector <Vertex> &in_buffer)
{
	out_buffer.clear();
	D3DXVECTOR3 viewv = centerv-eyev;
	D3DXVec3Normalize(&viewv, &viewv);
	float maxVal = D3DXVec3Dot((D3DXVECTOR3*)in_buffer.front().xyz, &viewv);
	float minVal = D3DXVec3Dot((D3DXVECTOR3*)in_buffer.back().xyz, &viewv);
	float range = (maxVal-minVal)*sqrt(3.0f);
	for each(const Vertex &vertex in in_buffer)
	{
		out_buffer.push_back(vertex);
		Vertex &v = out_buffer.back();
		int alpha = (v.color>>24);
		float val = D3DXVec3Dot((D3DXVECTOR3*)v.xyz, &viewv);
		float f_alpha = 1.0f-(val-minVal)/range;
		if(f_alpha<0)
			f_alpha=0;
		if(f_alpha>1)
			f_alpha=1;
		alpha = alpha*f_alpha;
		v.color = (v.color&0x00ffffff)|(alpha<<24);
	}
}

void RubixWindow::StartAnimation(Animator *pAni)
{
	pAni->init();
	pAnimator = pAni;
}

void RubixWindow::Update(double time_ms, int stack)
{
	if(rotation_angle!=-1 && !(selected_cube_index>=0 && (mouse_button_state&1)))
	{
		float rotation_angular_velocity = abs(rotation_angular_displacement)/(time_ms*0.001f);
		rotation_angle += time_ms*max(g_rotate_speed,rotation_angular_velocity*4)*0.001f*(rotation_angular_displacement>=0?1:-1);
		rotation_angle = min(rotation_angle, 90);
		rotation_angle = max(rotation_angle, 0);
		pRubix->RotateSlice(selected_axis, selected_XYZ[selected_axis%3], rotation_angle, undo_redo, rotation_angle==90);
		if(rotation_angle==0 || rotation_angle>=90)
		{
			rotation_angle = -1;
			selected_axis = -1;
			undo_redo = 0;
			ostringstream oss;
			oss << "History=" << pRubix->history_posi << "/" << pRubix->history.size();
			if(pRubix->isSolved())
				oss << endl << StringTable(127);
			base::ShowMessage(oss.str(), 10);
			SetDlgItemInt(hDlg, IDC_gotohist, pRubix->history_posi, false);
		}
		dirty = max(dirty, 2);
	}
	if(blink)
	{
		bool new_status = ((GetTickCount()%512)>=256);
		if(new_status != blink_status)
		{
			blink_status = new_status;
			dirty = max(dirty, 2);
		}
	}else if(blink_status){
		blink_status = false;
		dirty = max(dirty, 2);
	}
	if(stack)
		return;
	if(pAnimator){
		animator_time -= time_ms;
		if(animator_time<0){
			int axis, slice, angle;
			animator_time = pAnimator->nextMove(axis, slice, angle);
			if(angle){
				if(angle<0)
					axis = (axis+3)%6;
				if(abs(angle)==180){
					Simulate1Step(axis, slice);
					Update(1e10, 1);
				}
				Simulate1Step(axis, slice);
			}
			if(animator_time<0){
				delete pAnimator;
				pAnimator = NULL;
			}
		}
	}
}

void RubixWindow::Draw()
{
	// prepare DX states
	D3DXMATRIX m_view, m_proj;
	Device->GetTransform(D3DTS_VIEW, &m_view);
	Device->GetTransform(D3DTS_PROJECTION, &m_proj);

	Device->SetTransform(D3DTS_VIEW, &viewMat);
	Device->SetTransform(D3DTS_PROJECTION, &projMat);
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	Device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
	Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	Device->SetRenderState(D3DRS_ZENABLE, TRUE);
	Device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	Device->SetTexture(0, pRubix->directional?pTex_arrow:pTex_square);
	Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_GAUSSIANQUAD);
	Device->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
	if(use_distance_alpha){
		if(dirty>=2)
			pRubix->Draw(vertex_buffer2, blink_status?1:0);
		if(dirty>=1){
			SortVertices(vertex_buffer2);
			VertexAlphaDistance(vertex_buffer, vertex_buffer2);
		}
	}else{
		if(dirty>=2)
			pRubix->Draw(vertex_buffer, blink_status?1:0);
		if(dirty>=1)
			SortVertices(vertex_buffer);
	}
	dirty = 0;
	Device->SetFVF(Vertex_format);
	Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertex_buffer.size()/3, vertex_buffer.data(), sizeof(Vertex));

	Device->SetTransform(D3DTS_VIEW, &m_view);
	Device->SetTransform(D3DTS_PROJECTION, &m_proj);
	Device->SetRenderState(D3DRS_ZENABLE, FALSE);
	Device->SetTexture(0, NULL);
}

void RubixWindow::ComputeRay(D3DXVECTOR3 &outDirv, float x, float y)
{	// compute direction ray vector from screen mouse XY, -1<=x,y<=1
	D3DXVECTOR3 dirv = centerv-eyev;
	D3DXVECTOR3 rightv_norm, upv_norm;

	D3DXVec3Normalize(&dirv, &dirv);

	// obtain right unit vector
	D3DXVec3Cross(&rightv_norm, &dirv, &upv);

	// obtain up unit vector
	D3DXVec3Cross(&upv_norm, &rightv_norm, &dirv);

	// vertical and horizontal maximum
	float vert_dist = tan(viewAngle*0.5f)*D3DXVec3Length(&dirv);
	float hori_dist = vert_dist*Width/Height;

	outDirv = dirv + rightv_norm*hori_dist*x + upv_norm*vert_dist*y;
}

int RubixWindow::FindAxis(D3DXVECTOR3 &v)
{
	vector <float> angles;
	for(int x=0; x<6; x++)
		angles.push_back(AngleBetween2Vectors(v, axes[x]));
	return std::min_element(angles.begin(),angles.end())-angles.begin();
}

int RubixWindow::DetectRotation(int mouse_hit_x, int mouse_hit_y)
{
	// compute mouse hit ray
	D3DXVECTOR3 dirv, rotatev;
	ScreenXY2Ray(dirv, eyev, centerv, upv, mouse_hit_x, mouse_hit_y, Width, Height, viewAngle);

	// obtain rotation axis together with last hit ray
	D3DXVec3Cross(&rotatev, &dirv, &last_hit_dirv);

	{// compute the angle between rotation axis and +X,+Y,+Z,-X,-Y,-Z; the one with minimal angle is the wanted
		vector <float> angles;
		for(int x=0; x<6; x++)
			angles.push_back(((x%3)==(selected_cube_face%3))?1000:AngleBetween2Vectors(rotatev, axes[x]));
		int new_selected_axis = std::min_element(angles.begin(),angles.end())-angles.begin();
		if((selected_axis%3)==(new_selected_axis%3) || selected_axis==-1)
			selected_axis = new_selected_axis;
		string s = " selected_cube_face="+ToString(selected_cube_face)
			+" new_selected_axis="+ToString(new_selected_axis);
		base::ShowMessage(s);
	}

	// compute angle of rotation
	float new_rotation_angle = Radian2Degree(AngleBetween2Vectors(dirv, last_hit_dirv))*2;
	rotation_angular_displacement = new_rotation_angle-rotation_angle;
	rotation_angle = new_rotation_angle;

	// highlight the slice
	pRubix->CubeIndex2XYZ(selected_cube_index, selected_XYZ[0], selected_XYZ[1], selected_XYZ[2]);
	//pRubic->HighlightSlice(selected_axis, selected_XYZ[selected_axis%3]);
	pRubix->RotateSlice(selected_axis, selected_XYZ[selected_axis%3], rotation_angle, 0, false);

	dirty = max(dirty, 2);

	return selected_axis;
}

int RubixWindow::DetectCollision(int mouse_hit_x, int mouse_hit_y)
{
	// compute mouse hit ray
	ScreenXY2Ray(last_hit_dirv, eyev, centerv, upv, mouse_hit_x, mouse_hit_y, Width, Height, viewAngle);

	// check intersect
	int cube_id = pRubix->Intersect(eyev, last_hit_dirv, highlighted_layer);

	if(cube_id>=0){
		D3DXVECTOR3 norm = pRubix->cubes[cube_id]->IntersectFaceNormal(eyev, last_hit_dirv);
		selected_cube_face = FindAxis(norm);
		//pRubic->HighlightOne(cube_id);
		rotation_angle = -1;

		/*
		string s = "posi="+ToString(cube_id)
			+" orient="+ToString(pRubix->cubes[cube_id]->orient)
			+" selected_cube_face="+ToString(selected_cube_face);
		base::ShowMessage(s);*/

		if(GetAsyncKeyState(VK_CONTROL)<0){
			vector <int> indices;
			((OrbitSolver*)pSolver)->select_orbit(indices, cube_id);
			pRubix->clear_box();
			pRubix->create_box(indices);
		}

		if(GetAsyncKeyState(VK_SHIFT)<0){
			Cube *cube = pRubix->cubes[cube_id];
			if(cube->is_boxed()){
				cube->clear_box();
				pRubix->boxed_indices.erase(cube_id);
			}else{
				cube->create_box();
				pRubix->boxed_indices.insert(cube_id);
			}
		}
	}

	dirty = max(dirty, 2);

	return cube_id;
}

double Animator::nextMove(int &axis, int &slice, int &angle)
{
	angle = 0;	// skip further action
	return (!pRubixWindow->StepHistory(-1))?-1:(90.0/g_rotate_speed*1000.0);
}

