#include <vector>
#include <map>
#include <queue>

using namespace std;


bool comp_func(int l, int r){
	return true;
}

template <class Score>
class hscore_compare
{
	const bool reverse;
	const vector <Score> &data;

public:
	hscore_compare(vector <Score> &_data, const bool& revparam=false):data(_data),reverse(revparam){}
	bool operator() (int lhs, int rhs) const
	{
		if(lhs<0||lhs>=data.size()||rhs<0||rhs>=data.size())
			__asm int 3
		return (data[lhs]<=data[rhs])?reverse:(!reverse);
	}
};

// Generic A-star search, return number of steps to reach goal, or <0 if not reachable
template <class Node, class Action, class Score>
int AStarSearch(
	vector <Node> &out_state_seq,		// output optimal state seq, excluding start state
	vector <Action> &out_action_seq,	// output optimal action seq
	const Node &start_state,

	// search all available directions for current node, return output list size
	int (*gen_func)(vector <Node> &state_list, vector <Action> &action_list, const Node &state),

	// compute heuristic score, return whether destination is found
	bool (*eval_func)(Score &score, const Node &obj)
	)
{	// for upstream/downstream purposes, output sequences are not cleared
	vector <int> from_indices;				// index of the state which it comes from
	vector <Score> h_scores;				// heuristic scores of all states
	vector <Action> action_list;			// action to reach current state
	vector <const Node*> node_list;		// list of all node pointers into the map keys
	map <Node,int> node_map;				// global state-to-index map
	priority_queue <int, vector<int>, hscore_compare<Score>> stack (h_scores);		// main search stack, store indices

	// init
	node_map[start_state]=0;		// start state has an index of 0
	node_list.push_back(&node_map.find(start_state)->first);
	from_indices.push_back(0);		// start state comes from itself
	Score sc;
	const Node *goal = NULL;
	if(eval_func(sc, start_state))	// start state is goal
		return 0;
	h_scores.push_back(sc);
	action_list.push_back(0);
	stack.push(0);

	// main search loop
	while(!stack.empty()){
		// get queue head
		int top_id = stack.top();
		const Node &top = *node_list[top_id];
		stack.pop();

		// generate all adjacents
		vector <Node> states;
		vector <Action> actions;
		int n_adj = gen_func(states, actions, top);
		assert(n_adj==states.size() && n_adj==actions.size());

		// add adjacents
		for(int x=0; x<states.size(); x++){
			auto it = node_map.find(states[x]);
			if(it==node_map.end()){
				// add new element
				node_map[states[x]] = node_list.size();
				it = node_map.find(states[x]);
				node_list.push_back(&it->first);
				from_indices.push_back(top_id);
				action_list.push_back(actions[x]);
				if(eval_func(sc,it->first))
					goal=&it->first;
				h_scores.push_back(sc);
				stack.push(it->second);
			}else{	// node exist in map
			}
			if(goal)
				break;
		}
	}

	if(!goal)
		return -1;

	// backtrack
	vector <int> history;
	// get reverse state indices
	for(int x=node_map[*goal]; x!=0; x=from_indices[x])
		history.push_back(x);
	// save state history and action history
	for(auto it=history.rbegin(); it!=history.rend(); ++it){
		out_state_seq.push_back(*node_list[*it]);
		out_action_seq.push_back(action_list[*it]);
	}

	return 0;
}


