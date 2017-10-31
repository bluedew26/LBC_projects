#include "stdafx.h"
#include "grid.h"
#include "config.h"

vector<Pipe_Info> pipes;
vector<displayinfo> vis_container;
vector<BOX> boxes;     /// Boundary box 정보가 담긴 벡터
vector<connection_info> pipe_connection;


bool displaymodel = 1;
bool displayboundary = 1;
bool displaybndbox = 1;
bool displaypipe = 1;
bool pipeselection = 0;




double MIN_X_COORD = (-20000.0);// grid의 최소좌표
double MAX_X_COORD = (20000.0);// grid의 최대좌표
double MIN_Y_COORD = (-20000.0);//; grid의 최소좌표
double MAX_Y_COORD = (20000.0);// grid의 최대좌표
double MIN_Z_COORD = (-20000.0);// grid의 최소좌표
double MAX_Z_COORD = (20000.0);// grid의 최대좌표
int RANGEOFOBSTACLE = (2);		// 장애물주변 몇 그리드까지 장애물로 인식하는가?
int RANGEOFPIPEOBSTACLE = (2);     // 파이프 주변 몇 그리드를 장애물로 취급할 것인지.


double GRIDSIZE = (200);// diameter
double PIPE_IO_LENGTH = (500);  // 파이프로 취급할 실린더의 높이.
int WEIGHT_OF_OFFSET = (4); // 파이프 입구로부터 얼마큼의 여유공간을 허용할건지.

double TRANSLATIONSCALE = (500);   // 방향키 이동시 한번에 이동하는 좌표

double SENSITIVITY = (45.0); // OBB설정시 각도 변화 단위



// 가정 단위길이 = 100;
// 단위길이당 코스트 = 1;
// bend발생시 코스트 = 10;
double UNIT_DISTANCE = (100.0);
double STANDARD_COST = (1.0);
double BENDING_COST = (10.0 * STANDARD_COST);
