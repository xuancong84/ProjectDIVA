#define USESIMD 1

#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <cassert>
#include "rubix.h"
#include "d3dmath.h"
#include "resource.h"
#include "base.h"
#include "defines.h"
#include "rubix_solver_orbit.h"

using namespace std;
map <int,int> Cube::side_mask_to_compfunc;

void build_cube_vertices(	vector <Vertex> &vb_out, D3DXVECTOR3 center, FLOAT radius, DWORD side_mask,
	const float (&text_coord) [4][2], const DWORD (&side_colors)[6])
{
	// Set the 8 vertex coordinates
	float X=center[0], Y=center[1], Z=center[2];
	Vertex vert8[8] = {
		{X-radius,Y-radius,Z-radius}, {X+radius,Y-radius,Z-radius},
		{X+radius,Y+radius,Z-radius}, {X-radius,Y+radius,Z-radius},

		{X-radius,Y-radius,Z+radius}, {X+radius,Y-radius,Z+radius},
		{X+radius,Y+radius,Z+radius}, {X-radius,Y+radius,Z+radius},
	};

	// add faces according to side_mask
	int indices[] = {0,1,2,0,2,3};
	DWORD alpha = ((int)(255*g_alpha+0.5f))<<24;
	for(int x=0, mask=1; x<6; x++, mask<<=1)	// for each of the 6 faces
	{
		if(!(side_mask&mask))
			continue;
		for(int i=0; i<6; i++)
		{
			int y = indices[i];
			Vertex &vert = vert8[g_face_index[x][y]];
			vert.color = side_colors[x] | alpha;
			vert.tex_xy[0] = text_coord[y][0];
			vert.tex_xy[1] = text_coord[y][1];
			vb_out.push_back(vert);
		}
	}
}

Cube::Cube(float *center, float radius, DWORD side_mask, int posi)
{
	this->radius = radius;
	this->center = center;
	this->center_orig = center;
	this->side_mask = side_mask;
	this->index = posi;

	Reset();
}
Cube::~Cube(){}

void Cube::Reset()
{
	// initially orientations
	center = center_orig;
	dir0 = axes[0];
	dir1 = axes[1];
	draw_mask = 0;

	build_cube_vertices(vertex_buffer_prev, center, radius, side_mask, g_text_coord, g_side_colors);

	// duplicate vertex buffer to now
	vertex_buffer_now = vertex_buffer_prev;
	getDir();
}

void Cube::highlight(int alpha)
{
	vector <Vertex> *vertex_buffers[2]={&vertex_buffer_now,&vertex_buffer_prev};
	for(int i=0; i<2; i++){
		vector <Vertex> &vertex_buffer = *vertex_buffers[i];
		if(alpha&0x80000000){	// relative
			float ratio = (alpha&0xff)/255.0f;
			for(int x=0; x<vertex_buffer.size(); x++){
				DWORD &color = vertex_buffer[x].color;
				DWORD a = (int)((color>>24)*ratio+0.5f)<<24;
				color = (color&0x00ffffff)|a;
			}
		}else{	// absolute
			alpha <<= 24;
			for(int x=0; x<vertex_buffer.size(); x++){
				DWORD &color = vertex_buffer[x].color;
				color = (color&0x00ffffff)|alpha;
			}
		}
	}
}

void Cube::draw(vector <Vertex> &vertex_buffer)
{
	vertex_buffer.insert(vertex_buffer.end(), vertex_buffer_now.begin(), vertex_buffer_now.end());
	vertex_buffer.insert(vertex_buffer.end(), box_buffer_now.begin(), box_buffer_now.end());
}

void Cube::create_box()
{
	static float tex_coord[4][2] = {{0.5,0.5},{0.5,0.5},{0.5,0.5},{0.5,0.5}};
	static DWORD side_colors[6] = {WHITE,WHITE,WHITE,WHITE,WHITE,WHITE};
	float outer_radius = radius*0.99f;
	build_cube_vertices(box_buffer_prev, center, outer_radius, 0xffffffff, tex_coord, side_colors);
	box_buffer_now = box_buffer_prev;
}

void Cube::clear_box()
{
	box_buffer_now.clear();
	box_buffer_prev.clear();
}

bool Cube::is_boxed()
{
	return !box_buffer_now.empty();
}

float Cube::IntersectDist(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &dir) const
{	// if intersect, return distance from center to eye, otherwise, return -1
	for(int x=0,X=vertex_buffer_now.size(); x<X; x+=3){
		D3DXVECTOR3 &p1 = *(D3DXVECTOR3*)vertex_buffer_now[x].xyz;
		D3DXVECTOR3 &p2 = *(D3DXVECTOR3*)vertex_buffer_now[x+1].xyz;
		D3DXVECTOR3 &p3 = *(D3DXVECTOR3*)vertex_buffer_now[x+2].xyz;
		if(LineIntersectTriangle(eye, dir, p1, p2, p3)){
			D3DXVECTOR3 v = center-eye;
			return D3DXVec3Length(&v);
			//D3DXVECTOR3 v = (p1+p2+p3)*0.333333f-eye;
			//return D3DXVec3Length(&v);
		}
	}
	return -1;
}

D3DXVECTOR3 Cube::IntersectFaceNormal(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &dir) const
{	// if intersect, return the normal vector of the intersecting triangle, otherwise, return [0,0,0]
	D3DXVECTOR3 vface(0,0,0);
	float best_dist = 1e10;
	for(int x=0,X=vertex_buffer_now.size(); x<X; x+=3){
		D3DXVECTOR3 &p1 = *(D3DXVECTOR3*)vertex_buffer_now[x].xyz;
		D3DXVECTOR3 &p2 = *(D3DXVECTOR3*)vertex_buffer_now[x+1].xyz;
		D3DXVECTOR3 &p3 = *(D3DXVECTOR3*)vertex_buffer_now[x+2].xyz;
		if(LineIntersectTriangle(eye, dir, p1, p2, p3)){
			D3DXVECTOR3 v = (p1+p2+p3)*0.333333f-eye;
			float dist = D3DXVec3Length(&v);
			if(dist<best_dist){
				best_dist = dist;
				D3DXVECTOR3 v1=p2-p1, v2=p3-p1;
				D3DXVec3Cross(&vface, &v1, &v2);
			}
		}
	}
	return vface;
}

D3DXVECTOR3 Cube::IntersectFacePoint(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &dir) const
{	// if intersect, return the intersect point, otherwise, return [0,0,0]
	for(int x=0,X=vertex_buffer_now.size(); x<X; x+=3){
		D3DXVECTOR3 &p1 = *(D3DXVECTOR3*)vertex_buffer_now[x].xyz;
		D3DXVECTOR3 &p2 = *(D3DXVECTOR3*)vertex_buffer_now[x+1].xyz;
		D3DXVECTOR3 &p3 = *(D3DXVECTOR3*)vertex_buffer_now[x+2].xyz;
		if(LineIntersectTriangle(eye, dir, p1, p2, p3))
			return LineIntersectTriangleWhere(eye, dir, p1, p2, p3);
	}
	return D3DXVECTOR3(0,0,0);
}

void Cube::rotate(D3DXMATRIX &mat)
{
	rotate_angle(mat);
	D3DXVec3TransformCoord((D3DXVECTOR3*)&center, (D3DXVECTOR3*)&center, &mat);
	D3DXVec3TransformCoord((D3DXVECTOR3*)&dir0, (D3DXVECTOR3*)&dir0, &mat);
	D3DXVec3TransformCoord((D3DXVECTOR3*)&dir1, (D3DXVECTOR3*)&dir1, &mat);
	getDir();
	vertex_buffer_prev = vertex_buffer_now;
	box_buffer_prev = box_buffer_now;
}

void Cube::rotate_angle(D3DXMATRIX &mat)
{
	for(int x=0,X=vertex_buffer_now.size(); x<X; x++)
		D3DXVec3TransformCoord((D3DXVECTOR3*)&vertex_buffer_now[x], (D3DXVECTOR3*)&vertex_buffer_prev[x], &mat);
	for(int x=0,X=box_buffer_now.size(); x<X; x++)
		D3DXVec3TransformCoord((D3DXVECTOR3*)&box_buffer_now[x], (D3DXVECTOR3*)&box_buffer_prev[x], &mat);
	//for(int x=0; x<8; x++)
	//	D3DXVec3TransformCoord((D3DXVECTOR3*)&vertices_now[x], (D3DXVECTOR3*)&vertices_prev[x], &mat);
}

int Cube::getDir()
{
	vector <float> dists;
	for(int x=0; x<6; x++){
		D3DXVECTOR3 diffv = axes[x]-dir0;
		dists.push_back(D3DXVec3Length(&diffv));
	}
	int idir0 = std::min_element(dists.begin(), dists.end())-dists.begin();
	dir0 = axes[idir0];

	dists.clear();
	for(int x=0; x<6; x++){
		D3DXVECTOR3 diffv = axes[x]-dir1;
		dists.push_back(D3DXVec3Length(&diffv));
	}
	int idir1 = std::min_element(dists.begin(), dists.end())-dists.begin();
	dir1 = axes[idir1];

	orient = idir0*6+idir1;
	return orient;
}

bool Cube::isSolved(Cube &cube_ref, bool directional)
{	// check whether this is in solved orientation
	if(directional)	// directional cube, everything must be the same
		return orient==cube_ref.orient;
	// undirectional
	int compfunc = side_mask_to_compfunc[side_mask];
	switch(compfunc){
	case 0:	//for corners and edges, orient must be the same
		return orient==cube_ref.orient;
	case 1:	//face along x-axis
		return dir0==cube_ref.dir0;
	case 2:	//face along y-axis
		return dir1==cube_ref.dir1;
	case 3:	//face along z-axis
		{
			D3DXVECTOR3 dir2, ref_dir2;
			D3DXVec3Cross(&dir2, &dir0, &dir1);
			D3DXVec3Cross(&ref_dir2, &cube_ref.dir0, &cube_ref.dir1);
			return dir2==ref_dir2;
		}
	}
	return false;
}

Rubix::Rubix(int order, bool directional, bool solid)
{
	if(Cube::side_mask_to_compfunc.empty()){
		Cube::side_mask_to_compfunc[1]=1;
		Cube::side_mask_to_compfunc[2]=2;
		Cube::side_mask_to_compfunc[4]=3;
		Cube::side_mask_to_compfunc[8]=1;
		Cube::side_mask_to_compfunc[16]=2;
		Cube::side_mask_to_compfunc[32]=3;
	}

	history_posi = 0;
	this->order = order;
	this->solid = solid?1:0;
	this->directional = directional?1:0;

	int posi = 0;
	float side_size = 1.0f/(order+(order-1)*GAPSIDERATIO);
	float gap_size  = side_size*GAPSIDERATIO;
	float step_size = side_size+gap_size;
	float xyz[3];

	xyz[2] = -0.5f;
	for(int z=0; z<order; z++, xyz[2]+=step_size)
	{
		xyz[1] = -0.5f;
		for(int y=0; y<order; y++, xyz[1]+=step_size)
		{
			xyz[0] = -0.5f;
			for(int x=0; x<order; x++, xyz[0]+=step_size)
			{
				// mask
				DWORD mask = 0;
				int L=0, R=order-1;
				L=GetInwardLayerIndex(x,y,z);
				R=order-L-1;
				if(x==L) mask |= (1<<LEFTSIDE);
				if(x==R) mask |= (1<<RIGHTSIDE);
				if(y==L) mask |= (1<<BOTTOMSIDE);
				if(y==R) mask |= (1<<TOPSIDE);
				if(z==L) mask |= (1<<BACKSIDE);
				if(z==R) mask |= (1<<FRONTSIDE);

				float center[3] = {xyz[0]+side_size/2, xyz[1]+side_size/2, xyz[2]+side_size/2};
				cubes.push_back(new Cube(center, side_size/2, mask, posi++));
			}
		}
	}
}
Rubix::~Rubix(){	release(); }
void Rubix::release()
{
	for each(Cube *p in cubes)
		delete p;
	cubes.clear();
	boxed_indices.clear();
	highlighted_indices.clear();
	history.clear();
	history_posi=0;
}
int	Rubix::GetInwardLayerIndex(int x, int y, int z)
{
	int L=min(x,min(y,z));
	int R=order-1-max(x,max(y,z));
	return min(L,R);
}
int	Rubix::GetInwardLayerIndex(int posi)
{
	int x,y,z;
	CubeIndex2XYZ(posi,x,y,z);
	return GetInwardLayerIndex(x,y,z);
}
int Rubix::Draw(vector <Vertex> &vertex_buffer, int mask)
{
	vertex_buffer.clear();
	for(int x=0; x<cubes.size(); x++){
		if(!solid) if(GetInwardLayerIndex(x))
			continue;
		if(mask&1) if(cubes[x]->orient!=1)
			continue;
		cubes[x]->draw(vertex_buffer);
	}
	return 0;
}

int Rubix::Intersect(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &dir, int layer)
{	// if intersect, return cube index, otherwise, return -1
	D3DXVECTOR3 origin(0,0,0);

	// if it does not intersect with whole cube -> skip, 0.75=0.5^2+0.5^2+0.5^2
	if(!LineIntersectSphere(eye, dir, origin, sqrt(0.75f)))
		return -1;

	// find the closest intersecting cube
	FLOAT radius3 = sqrt(3.0f);
	float best_dist = 1e10;
	int	best_index = -1;
	for(int x=0,X=cubes.size(); x<X; x++){
		if(layer>=0){
			if(GetInwardLayerIndex(x)!=layer)
				continue;
		}
		Cube &cube = *cubes[x];
		if(!LineIntersectSphere(eye, dir, cube.center, radius3))
			continue;
		float intersect_dist = cube.IntersectDist(eye, dir);
		if(intersect_dist>=0) if(intersect_dist<best_dist){
			best_dist = intersect_dist;
			best_index = x;
		}
	}

	return best_index;
}

void Rubix::ClearHighlight()
{
	for each(int i in highlighted_indices)
		cubes[i]->highlight(0xff);
	highlighted_indices.clear();
}

void Rubix::RotateSlice(int axis, int slice, int angle, int history_shift, bool bFinalize)
{	// Rotate a slice by angle degrees, if angle=90, finalize the position
	/*	axis:	0:x-axis, 1:y-axis, 2:z-axis
				3:-x-axis, 4:-y-axis, 5:-z-axis
		slice:	starting from 0:at most negative axis position */
	int order2=order*order;
	int	x,y,z,*px,*py;
	D3DXMATRIX mat_rot;
	bool ret = false;

	D3DXMatrixRotationAxis(&mat_rot, &axes[axis], Degree2Radian(angle));
	SelectSlice(axis%3,slice,&x,&y,&z,&px,&py);

	if(bFinalize){	// finalize new position
		vector <Cube*> aslice(order*order);
		// Save rotated slice on aslice buffer
		if(axis<3){
			for(*px=0;*px<order;(*px)++) for(*py=0;*py<order;(*py)++)
				aslice[(*px)*order+order-1-(*py)] = cubes[z*order2+y*order+x];
		}else{
			for(*px=0;*px<order;(*px)++) for(*py=0;*py<order;(*py)++)
				aslice[(order-1-(*px))*order+(*py)] = cubes[z*order2+y*order+x];
		}
		// Perform rotation and copy back from aslice buffer
		for(*px=0;*px<order;(*px)++) for(*py=0;*py<order;(*py)++){
			Cube *cube = aslice[(*py)*order+(*px)];
			cube->rotate(mat_rot);
			cubes[z*order2+y*order+x] = cube;
		}
		// manage history buffer and pointer
		if(history_shift)
			history_posi += history_shift;
		else{
			history.resize(++history_posi, 0);
			history.back() = (axis<<16|slice);
		}
	}else{		// rotate by an angle <90 deg
		for(*px=0;*px<order;(*px)++) for(*py=0;*py<order;(*py)++)
			cubes[z*order2+y*order+x]->rotate_angle(mat_rot);
	}
}

bool Rubix::isSolved()
{
	for(int x=1; x<cubes.size(); x++){
		if(!solid)
			if(GetInwardLayerIndex(x))
				continue;
		if(!cubes[x]->isSolved(*cubes[0], directional))
			return false;
	}
	return true;
}

void Rubix::SelectSlice(int axis, int slice, int *x, int *y, int *z, int **px, int **py){
	switch(axis){
	case 0:	//X
		*x = slice;		*y = *z = 0;	*px = y;		*py = z;		break;
	case 1:	//Y
		*y = slice;		*x = *z = 0;	*px = z;		*py = x;		break;
	case 2:	//Z
		*z = slice;		*x = *y = 0;	*px = x;		*py = y;		break;
	default:
		assert(false);
	}
}// return 2D-permutation axis

void Rubix::HighlightOne(int id, bool clear)
{
	if(clear)
		ClearHighlight();
	cubes[id]->highlight(true);
	highlighted_indices.insert(id);
}

void Rubix::HighlightSlice(int axis, int slice)
{
	int order2=order*order;
	int	x,y,z,*px,*py;

	ClearHighlight();
	SelectSlice(axis%3,slice,&x,&y,&z,&px,&py);

	// highlight NxN
	for(*px=0;*px<order;(*px)++) for(*py=0;*py<order;(*py)++)
		HighlightOne(z*order2+y*order+x, false);
}

void Rubix::HighlightLayer(int layer, int alphaA, int alphaB)
{
	ClearHighlight();
	int order2=order*order;
	for(int z=0; z<order; z++)
		for(int y=0; y<order; y++)
			for(int x=0; x<order; x++)
				cubes[z*order2+y*order+x]->highlight((GetInwardLayerIndex(x,y,z)==layer)?alphaA:alphaB);
}

void Rubix::HighlightAll(int alpha)
{
	for(int x=0; x<cubes.size(); x++)
		cubes[x]->highlight(alpha);
}

void Rubix::CubeIndex2XYZ(int index, int &x, int &y, int &z)
{
	int order2 = order*order;
	x = index%order;
	y = (index%order2)/order;
	z = index/order2;
}

void Rubix::Shuffle(int seed)
{
	srand(seed);
	int last_axis=0, last_slice=0;
	int axis, slice;
	for(int x=0; x<cubes.size(); x++){
		int axis, slice;
		do{
			int r = rand();
			axis = r%6, slice = r%order;
		}while(slice==last_slice && (axis%3)==(last_axis%3));
		RotateSlice(axis, slice, 90, 0, true);
		last_axis = axis, last_slice = slice;
	}
}

void Rubix::create_box(const vector <int> &indices)
{
	for each(int id in indices){
		cubes[id]->create_box();
		boxed_indices.insert(id);
	}
}

void Rubix::clear_box()
{
	for each(int x in boxed_indices)
		cubes[x]->clear_box();
	boxed_indices.clear();
}

void Rubix::get_state(vector <State> &out)
{
	out.clear();
	for each(Cube* cube in cubes)
		out.push_back(cube->get_state());
}



template <class T>
ostream & operator << (ostream &ofs, const vector <T> &data)
{
	ofs << data.size() << endl;
	for each(const T& e in data)
		ofs << e << " ";
	ofs << endl;
	return ofs;
}

template <class T>
istream & operator >> (istream &ifs, vector <T> &data)
{
	int n;
	ifs >> n;
	T e;
	data.clear();
	for(int x=0; x<n; x++){
		ifs >> e;
		data.push_back(e);
	}
	return ifs;
}

ostream & operator << (ostream &ofs, const D3DXVECTOR3 &c)
{
	ofs << c.x << " " << c.y << " " << c.z << endl;
	return ofs;
}

istream & operator >> (istream &ifs, D3DXVECTOR3 &c)
{
	ifs >> c.x >> c.y >> c.z;
	return ifs;
}

ostream & operator << (ostream &ofs, const Vertex &c)
{
	ofs << c.xyz[0] << " " << c.xyz[1] << " " << c.xyz[2] << " " << c.color << " " << c.tex_xy[0] << " " << c.tex_xy[1] << endl;
	return ofs;
}

istream & operator >> (istream &ifs, Vertex &c)
{
	ifs >> c.xyz[0] >> c.xyz[1] >> c.xyz[2] >> c.color >> c.tex_xy[0] >> c.tex_xy[1];
	return ifs;
}

ostream & operator << (ostream &ofs, const Cube &c)
{
	ofs << c.orient << endl;
	ofs << c.index << endl;
	ofs << c.draw_mask << endl;
	ofs << c.side_mask << endl;
	ofs << c.radius << endl;
	ofs << c.center << c.center_orig << c.dir0 << c.dir1 << endl;
	ofs << c.box_buffer_prev << c.box_buffer_now << c.vertex_buffer_prev << c.vertex_buffer_now << endl;
	return ofs;
}

istream & operator >> (istream &ifs, Cube &c)
{
	ifs >> c.orient;
	ifs >> c.index;
	ifs >> c.draw_mask;
	ifs >> c.side_mask;
	ifs >> c.radius;
	ifs >> c.center >> c.center_orig >> c.dir0 >> c.dir1;
	ifs >> c.box_buffer_prev >> c.box_buffer_now >> c.vertex_buffer_prev >> c.vertex_buffer_now;
	return ifs;
}

void Rubix::save(char *filename)
{
	ofstream fout(filename);
	fout << order << endl;
	fout << directional << endl;
	fout << solid << endl;
	fout << history_posi << endl;
	fout << history << endl;
	{	// save cubes
		fout << cubes.size() << endl;
		for each(const Cube *c in cubes)
			fout << *c;
	}
}

void Rubix::load(char *filename)
{
	release();
	ifstream fin(filename);
	fin >> order;
	fin >> directional;
	fin >> solid;
	fin >> history_posi;
	fin >> history;
	{	// load cubes
		int n;
		fin >> n;
		for(int x=0; x<n; x++){
			cubes.push_back(new Cube());
			fin >> *cubes.back();
		}
	}
}
