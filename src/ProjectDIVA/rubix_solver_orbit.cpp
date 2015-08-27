#include <set>
#include <sstream>
#include <queue>
#include <map>
#include <cassert>
#include "rubix.h"
#include "rubix_solver_orbit.h"
#include "AStar.hpp"

using namespace std;

int	orient_map[6][36];		// under the rotation, current orientation -> which orientation

void check(int val, int vmax){
	if(val<0 || val>=vmax)
		__asm int 3
}


OrbitSolver::OrbitSolver(RubixWindow *p):Animator(p){
	pRubixWindow = p;
	pRubix = p->getRubix();
}
OrbitSolver::~OrbitSolver(){}

int OrbitSolver::select_orbit(vector <int> &indices, int posi)
{
	int order = pRubix->order;
	int order2 = order*order;
	int order3 = order*order*order;
	set <int> covered_indices;
	vector <int> stack;

	// create and initialize a binary mask cube
	char *cubes = new char [order3];
	char *aslice = new char [order2];

	// BFS loop
	covered_indices.insert(posi);
	stack.push_back(posi);
	while(!stack.empty()){
		// pop last element
		posi = stack.back();
		stack.pop_back();

		// rotate 90 deg along each of xyz axis
		for(int axis=0; axis<6; axis++){
			// initialize cube buffer
			memset(cubes, 0, order3);
			cubes[posi] = 1;

			// get XYZ coordinates
			int x,y,z;
			pRubix->CubeIndex2XYZ(posi,x,y,z);
			int *px,*py,xyz[3]={x,y,z};
			pRubix->SelectSlice(axis%3,xyz[axis%3],&x,&y,&z,&px,&py);

			// Save rotated slice on aslice buffer
			if(axis<3){
				for(*px=0;*px<order;(*px)++) for(*py=0;*py<order;(*py)++){
					//check((*px)*order+order-1-(*py),order2);
					//check(z*order2+y*order+x,order3);
					aslice[(*px)*order+order-1-(*py)] = cubes[z*order2+y*order+x];
				}
			}else{
				for(*px=0;*px<order;(*px)++) for(*py=0;*py<order;(*py)++){
					//check((order-1-(*px))*order+(*py),order2);
					//check(z*order2+y*order+x,order3);
					aslice[(order-1-(*px))*order+(*py)] = cubes[z*order2+y*order+x];
				}
			}

			// Perform rotation and copy back from aslice buffer
			for(*px=0;*px<order;(*px)++) for(*py=0;*py<order;(*py)++){
				//check(z*order2+y*order+x, order3);
				//check((*py)*order+(*px), order2);
				cubes[z*order2+y*order+x] = aslice[(*py)*order+(*px)];
			}

			// Find new position and add
			bool found = false;
			for(int i=0; i<order3; i++) if(cubes[i])
			{
				if(covered_indices.find(i)==covered_indices.end())
				{
					covered_indices.insert(i);
					stack.push_back(i);
					found = true;
				}
				break;
			}
		}
	}

	// release
	delete [] cubes, aslice;

	indices.clear();
	for each(int i in covered_indices)
		indices.push_back(i);

	return indices.size();
}

int OrbitSolver::enumerate_orbit(vector <vector <int> > &indices_list, int order)
{
	int order2 = order*order;
	int odd = (order&1)?1:0;

	// for both even and odd, from inner outwards
	int inner_order = (order&1)?1:0;
	int n_total = 0;
	for(int z=pRubix->order/2; z>=0; z--, inner_order+=2){	
		/* inner_order=7,	8
		    *			    *#
		   **#			   **##
		  ***##			  ***###
		 ****###		 ****####
		*/
		int half=(inner_order+1)/2;
		for(int y=(order-1)/2, Y=0; Y<half; y--,Y++){
			if((Y==0 && odd) || Y==half-1){
				for(int x=y,X=(order-1)/2; x<=X; x++){
					vector <int> indices;
					select_orbit(indices, z*order2+y*order+x);
					indices_list.push_back(indices);
					n_total += indices.size();
				}
			}else{
				for(int x=(order-1)/2,X=(inner_order-1)/2; X>0; X--,x--){
					vector <int> indices;
					select_orbit(indices, z*order2+y*order+x);
					indices_list.push_back(indices);
					n_total += indices.size();
				}
			}
		}
	}
	assert (n_total==order*order*order);
	return indices_list.size();
}

void OrbitSolver::init()
{
	// prepare rotation matrices
	for(int x=0; x<6; x++)
		D3DXMatrixRotationAxis(&rot_mats[x], &axes[x], Degree2Radian(90));

	// obtain 24 orientations
	vector <Cube> cubes;
	queue <Cube> q;
	orients.clear();
	cubes.push_back(*pRubix->cubes[0]);
	orients.insert(pRubix->cubes[0]->orient);
	q.push(*pRubix->cubes[0]);
	while(!q.empty()){
		Cube &cube = q.front();
		for(int x=0; x<6; x++){
			Cube newcube = cube;
			newcube.rotate(rot_mats[x]);
			if(orients.find(newcube.orient)==orients.end()){
				orients.insert(newcube.orient);
				cubes.push_back(newcube);
				q.push(newcube);
			}
		}
		q.pop();
	}

	// build orientation map
	for(int x=0; x<6; x++){
		for(int y=0; y<36; y++)
			orient_map[x][y] = 0;
		for each(const Cube &cube in cubes){
			Cube newcube = cube;
			newcube.rotate(rot_mats[x]);
			orient_map[x][cube.orient]=newcube.orient;
		}
	}

	// Start solving
	vector <State> state;
	pRubix->get_state(state);
	Solve(state, pRubix->order);
	moves_posi = 0;
}

double OrbitSolver::nextMove(int &axis, int &slice, int &angle)
{
	if(moves_posi>=moves.size()){
		angle = 0;
		return -1;
	}
	Move m = moves[moves_posi++];
	axis = m.axis;
	slice = m.slice;
	angle = 90;
	return (moves_posi>=moves.size())?-1:(90.0/g_rotate_speed*1000.0);
}

int gen_func(vector <int> &state_list, vector <int> &action_list, const int &state)
{
	for(int x=0; x<6; x++){
		action_list.push_back(x);
		state_list.push_back(orient_map[x][state]);
	}
	return state_list.size();
}

bool eval_func(int &score, const int &obj)
{
	score = (obj==1?0:-1);
	return (obj==1);
}

int OrbitSolver::SolveCenter(vector<Move> &output_move, vector <State> &input_state, int index, int order)
{
	Cube cube = *pRubix->cubes[index];
	vector <int> action_seq;
	vector <int> state_seq;
	AStarSearch(state_seq,action_seq,input_state[index].orient,&gen_func,&eval_func);
	for each(int axis in action_seq){
		Move m = {axis, order/2};
		output_move.push_back(m);
	}
	return 0;
}

int OrbitSolver::SolveOrbit(vector <Move> &output_move, vector<State> &input_state, vector <int> &indices, int order)
{
	if(indices.size()==1)
		return SolveCenter(output_move, input_state, indices[0], order);
	return 0;
}

int OrbitSolver::Solve(vector<State> &rubix_state, int order)
{//	main solver function
	moves.clear();

	// enumerate all orbits (in the order for solving)
	vector <vector <int> > orbits;
	enumerate_orbit(orbits, order);

	// solve for each orbit independent
	for(int x=0; x<orbits.size(); ++x)
		SolveOrbit(moves, rubix_state, orbits[x], order);

	return 0;
}

