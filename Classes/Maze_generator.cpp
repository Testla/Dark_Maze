#include <list>
#include <queue>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include "Matrix.hpp"

#include "cocos2d.h"
USING_NS_CC;

using std::pair;

const char WALL = '#', FLOOR = ' ';
const pair<int, int> adjacent[] = {
	{ -1, 0 },
	{ 1, 0 },
	{ 0, -1 },
	{ 0, 1 }
};

struct Prim_State {
	/* positions of the wall and corresponding floor */
	pair<int, int> wall, floor;
	int weight;
};

/* if the positions are identical, they are equal,
* else, they are compared by the random weight */
struct Prim_State_compare {
	bool operator()(const Prim_State &a, const Prim_State &b) {
		return (a.wall != b.wall && a.weight <= b.weight);
	}
};

pair<int, int> coordinate_add(const pair<int, int> a, const pair<int, int> b) {
	return{ a.first + b.first, a.second + b.second };
}

/*
* the size of the matrix should be even times even
* the coordinates of possible floors are all in the form (odd, odd)
* will randomly pick a start point, generate a maze,
* and randomly pick an end point in the leaves with a far enough distance
* please make sure that the size of the matrix is bigger than 3 by 3
*/
void Maze_generate(
	Matrix<char> &matrix,
	pair<int, int> &start,
	pair<int, int> &end
	) {
	std::priority_queue<
		Prim_State,
		std::vector<Prim_State>,
		Prim_State_compare
	> prim_states;
	Prim_State prim_state;
	Matrix<int> distance(matrix.size());
	Matrix<bool> visited(matrix.size());
	std::vector<pair<int, pair<int, int> > > candidate_ends;
	int i;
	pair<int, int> destiny, corresponding_floor;
	visited.assign(false);
	srand(time(NULL));
	char buf[40];
	sprintf(buf, "--------time : %d", time(NULL));
	log(buf);
	matrix.assign(WALL);
	start.first = (rand() % (matrix.size().first - 1) / 2) * 2 + 1;
	start.second = (rand() % (matrix.size().second - 1) / 2) * 2 + 1;
	distance.at(start) = 0;
	matrix.at(start) = FLOOR;
	for (i = 0; i < 4; ++i) {
		destiny = coordinate_add(start, adjacent[i]);
		corresponding_floor = coordinate_add(destiny, adjacent[i]);
		if (destiny.first > 0 && destiny.first + 1 < matrix.size().first
			&& destiny.second > 0 && destiny.second + 1 < matrix.size().second) {
			prim_states.push(Prim_State{ destiny, corresponding_floor, rand() });
			distance.at(corresponding_floor) = distance.at(start) + 2;
		}
	}
	/* run prim algoithm */
	while (!prim_states.empty()) {
		prim_state = prim_states.top();
		prim_states.pop();
		if (matrix.at(prim_state.floor) != FLOOR) {
			matrix.at(prim_state.wall) = FLOOR;
			matrix.at(prim_state.floor) = FLOOR;
			for (i = 0; i < 4; ++i) {
				destiny = coordinate_add(prim_state.floor, adjacent[i]);
				corresponding_floor = coordinate_add(destiny, adjacent[i]);
				if (destiny.first > 0 && destiny.first + 1 < matrix.size().first
					&& destiny.second > 0 && destiny.second + 1 < matrix.size().second
					&& matrix.at(corresponding_floor) != FLOOR) {
					prim_states.push(Prim_State{ destiny, corresponding_floor, rand() });
					distance.at(corresponding_floor) = distance.at(prim_state.floor) + 2;
				}
			}
		}
	}
	/* use BFS to find the leaves */
	std::queue<pair<int, int> > BFS_States;
	//Matrix<bool> visited(matrix.size());
	pair<int, int> current;
	bool is_leave;
	visited.assign(false);
	BFS_States.push(start);
	visited.at(start) = true;
	while (!BFS_States.empty()) {
		current = BFS_States.front();
		BFS_States.pop();
		is_leave = true;
		for (i = 0; i < 4; ++i) {
			destiny = coordinate_add(current, adjacent[i]);
			if (destiny.first >= 0 && destiny.first < matrix.size().first
				&& destiny.second >= 0 && destiny.second < matrix.size().second
				&& matrix.at(destiny) == FLOOR
				&& !visited.at(destiny)) {
				BFS_States.push(destiny);
				visited.at(destiny) = true;
				is_leave = false;
			}
		}
		if (is_leave)
			candidate_ends.push_back({ distance.at(current), current });
	}
	/* select end point */
	std::sort(candidate_ends.begin(), candidate_ends.end());
	end = candidate_ends[(candidate_ends.size() - 1) * (1 - 1 / 4.0 * rand() / RAND_MAX)].second;
}
