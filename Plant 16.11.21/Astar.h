#pragma once
#include "grid.h"
#include <list>

using namespace std;



void grid_init();

vector<path_data> pathfinder(point pinit, point pfinal);
vector<path_data> sharpening(vector<path_data> path); // 직각화 알고리즘

point coord_to_grid(double x, double y, double z);
point_d grid_to_coord(int x, int y, int z);
void gatherpipe();