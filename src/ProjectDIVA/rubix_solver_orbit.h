#pragma once

#include <vector>
#include <set>
#include "Rubix.h"
#include "RubixWindow.h"

using namespace std;
extern int	orient_map[6][36];		// under the rotation, current orientation -> which orientation

class OrbitSolver : public Solver, public Animator{
public:
	Rubix *pRubix;
	vector <Move> moves;
	int moves_posi;

	D3DXMATRIX rot_mats[6];		// basis rotation matrices on orientations, +/-90 deg along each axis
	set <int> orients;			// the 24 distinct orientations from 0~35

	OrbitSolver(RubixWindow *p);
	~OrbitSolver();

	// implementation override
	void init();
	int Solve(vector <State> &state, int order);
	double nextMove(int &axis, int &slice, int &angle);

	// self-owned functions
	// input posi=0~(N*N*N-1), output "indices" on the same orbit
	int	select_orbit(vector <int> &indices, int posi);
	int	enumerate_orbit(vector <vector<int> > &indices_list, int order);
	int SolveOrbit(vector<Move> &output_move, vector <State> &input_state, vector <int> &indices, int order);
	int SolveCenter(vector<Move> &output_move, vector <State> &input_state, int index, int order);
};

