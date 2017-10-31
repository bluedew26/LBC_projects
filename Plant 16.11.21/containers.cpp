#include "stdafx.h"
#include "grid.h"
#include "config.h"

vector<Pipe_Info> pipes;
vector<displayinfo> vis_container;
vector<BOX> boxes;     /// Boundary box ������ ��� ����
vector<connection_info> pipe_connection;


bool displaymodel = 1;
bool displayboundary = 1;
bool displaybndbox = 1;
bool displaypipe = 1;
bool pipeselection = 0;




double MIN_X_COORD = (-20000.0);// grid�� �ּ���ǥ
double MAX_X_COORD = (20000.0);// grid�� �ִ���ǥ
double MIN_Y_COORD = (-20000.0);//; grid�� �ּ���ǥ
double MAX_Y_COORD = (20000.0);// grid�� �ִ���ǥ
double MIN_Z_COORD = (-20000.0);// grid�� �ּ���ǥ
double MAX_Z_COORD = (20000.0);// grid�� �ִ���ǥ
int RANGEOFOBSTACLE = (2);		// ��ֹ��ֺ� �� �׸������ ��ֹ��� �ν��ϴ°�?
int RANGEOFPIPEOBSTACLE = (2);     // ������ �ֺ� �� �׸��带 ��ֹ��� ����� ������.


double GRIDSIZE = (200);// diameter
double PIPE_IO_LENGTH = (500);  // �������� ����� �Ǹ����� ����.
int WEIGHT_OF_OFFSET = (4); // ������ �Ա��κ��� ��ŭ�� ���������� ����Ұ���.

double TRANSLATIONSCALE = (500);   // ����Ű �̵��� �ѹ��� �̵��ϴ� ��ǥ

double SENSITIVITY = (45.0); // OBB������ ���� ��ȭ ����



// ���� �������� = 100;
// �������̴� �ڽ�Ʈ = 1;
// bend�߻��� �ڽ�Ʈ = 10;
double UNIT_DISTANCE = (100.0);
double STANDARD_COST = (1.0);
double BENDING_COST = (10.0 * STANDARD_COST);
