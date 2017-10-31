#pragma once
#include "AADS_Client.h"
/*
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <opencv2\core.hpp>
*/
/////////////////////////////////////////////
const float ICE_BREAKING_CAPA = 1.2;  /// ���β� ����
const float PENALTY = 0;
const float HULLDEPTH = 20.0;
const bool USE_SMOOTHING = true;
const int GRIDSIZE = 1;
const int LANDOFFSET = 1; // 3��ŭ manhattan�Ÿ��� ���ؼ� ������ �β��� ���
const int MAP_MAGITUDE = 30;
const unsigned int Offset = 150 / GRIDSIZE; // Offset ����

const unsigned int ENTIRE_MAP_WIDTH = 2047; // �� �׸��� ����
const unsigned int ENTIRE_MAP_HEIGHT = 2047; // �� �׸��� ����


const float SPEED_FOR_OPEN_WATER = 6.17328;   // 12knots
const float KNOTS_TO_M_PER_SEC = 6.17328 / 12;


#ifdef TEST
#define MAX_DATA 1 // ��� ������ ���� ����
#endif
#ifndef TEST
#define MAX_DATA 8 // ��� ������ ���� ����
#endif

/////////////////////////////////////////////



const int	 NODE_DISTANCE = 2500;


#define DUMMYFLOAT -1500.0
#define TIME_GAP 24      // 24�ð����� ����
#define INVALID_VALUE -111.0f
#define MAX_DAYS_STORED 8 // 8��ġ ������ ����



#define MAX_ICE_T 1.5 // ǥ�� ������ ���β� �ִ밪


#define DEPTH_THRESHOLD (0.0)

#ifdef TEST

extern IplImage *sample;

#endif

#ifdef TEST
const bool pre_load = true;   // �̸� txt���Ϸκ��� �����͸� �ε��Ұ����� (true) , ������ ��û�ؼ� �ε��Ұ�����(false)
#endif
#ifndef TEST
const bool pre_load = false;
#endif


