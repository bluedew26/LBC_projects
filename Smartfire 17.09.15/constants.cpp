#include "stdafx.h"


double THRESHOLD_OF_OBSTACLE = (100); // 50m보다 길이가 짧은 구조물은 내부에 놓인 장애물 취급
double THRESHOLD_OF_FLOOD_AREA = (400); // 100m^2 보다 넓이가 작은 곳은 flood-fill 안함
double THRESHOLD_OF_PATH_AREA = (100);
double THRESHOLD_OF_WALL_AREA = (5);



double THRESHOLD_OF_DOOR  = (3); // 문의 최대 길이 (3m)
double RESTRICTED_DISTANCE = (5); // 소화전 주변 최소 얼마나 떨어져야 하는가
