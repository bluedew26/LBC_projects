#pragma once
#include "AADS_Client.h"
/*
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <opencv2\core.hpp>
*/
/////////////////////////////////////////////
const float ICE_BREAKING_CAPA = 1.2;  /// 빙두께 제한
const float PENALTY = 0;
const float HULLDEPTH = 20.0;
const bool USE_SMOOTHING = true;
const int GRIDSIZE = 1;
const int LANDOFFSET = 1; // 3만큼 manhattan거리에 대해서 육지를 두껍게 취급
const int MAP_MAGITUDE = 30;
const unsigned int Offset = 150 / GRIDSIZE; // Offset 수준

const unsigned int ENTIRE_MAP_WIDTH = 2047; // 총 그리드 갯수
const unsigned int ENTIRE_MAP_HEIGHT = 2047; // 총 그리드 갯수


const float SPEED_FOR_OPEN_WATER = 6.17328;   // 12knots
const float KNOTS_TO_M_PER_SEC = 6.17328 / 12;


#ifdef TEST
#define MAX_DATA 1 // 기상 데이터 저장 갯수
#endif
#ifndef TEST
#define MAX_DATA 8 // 기상 데이터 저장 갯수
#endif

/////////////////////////////////////////////



const int	 NODE_DISTANCE = 2500;


#define DUMMYFLOAT -1500.0
#define TIME_GAP 24      // 24시간마다 갱신
#define INVALID_VALUE -111.0f
#define MAX_DAYS_STORED 8 // 8일치 데이터 저장



#define MAX_ICE_T 1.5 // 표현 가능한 빙두께 최대값


#define DEPTH_THRESHOLD (0.0)

#ifdef TEST

extern IplImage *sample;

#endif

#ifdef TEST
const bool pre_load = true;   // 미리 txt파일로부터 데이터를 로드할것인지 (true) , 서버에 요청해서 로드할것인지(false)
#endif
#ifndef TEST
const bool pre_load = false;
#endif


