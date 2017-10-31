#pragma once
#include "outputconfig.h"
#include "KRISOServermanager.h"
#include "KRISOClient.h"
#include "Config_dk.h"
#include <stdlib.h>
#include <ctime>
#include "libxl.h"
#define PI  3.141592653f

using namespace libxl;


enum AADS_DATA {
	AWU = 1, AWV, APR, ATM,
	TWH, TWP, TWD, WWH, WWVP, WWD, SWH, SWP, SWD,
	SST, SSS, SCU, SCV, ICF, ICH, SNW, ICU, ICV
};



class AADS_Client
{

public:
	Book *book60, *book70, *book80, *book90, *book100;
	Sheet *sheet60, *sheet70, *sheet80, *sheet90, *sheet100;
	int start_x, start_y;
	int width, height;
	double lon;
	AraonConfig config[3];
	WeatherServer server;

	AADS_Client(){
		lon = 0; start_x = 0; start_y = 0; width = 0; height = 0;
		Init();
	}
	~AADS_Client(void);

	void Init();
	void WeatherUpdate(GRIDPoint k_point, char * date, char * time);
	float * WeatherDataUpdate(gamemap_point map, char * date, char * time, int * size);
	float WeatherDataRequest(int data, int x, int y, float v, float time);
	float GetSpeed(int x1, int y1, int x2, int y2, float v, double time, float datas[8]); // 이미 알고있는 값이라면 데이터를 추가로 제공한다.
	float*  KaistRequest_widely(int data, int x, int y, int w, int h, float time);
	void libprint();
	void ExcelInit();
	void ExcelRelease();
	
	int Attainspeed2(float ih, float ifd, float v);
	

	KRISOPoint GridToLatLon(GRIDPoint gpoint);

	float getBearingAngle2(IndexPoint g_point){
		float Vx, Vy;
		float EAST, NORTH;
		float LongOrigin = 315.0;
		float BearingAngle;
		
		GRIDPoint gpt;
		gpt.s_xgrid = g_point.s_xindex;
		gpt.s_ygrid = g_point.s_yindex;
		gpt.e_xgrid = g_point.e_xindex;
		gpt.e_ygrid = g_point.e_yindex;

		KRISOPoint kp = GridToLatLon(gpt);
		float lon = kp.s_lon;

		Vx = g_point.e_xindex - g_point.s_xindex;
		Vy = g_point.e_yindex - g_point.s_yindex;
		EAST = Vx*cos(radianss2(lon - LongOrigin)) + Vy*sin(radianss2(lon - LongOrigin));
		NORTH = -Vx*sin(radianss2(lon - LongOrigin)) + Vy*cos(radianss2(lon - LongOrigin));
		//BearingAngle = atan2f(EAST, NORTH);// *180 / PI;
		BearingAngle = atan2f(NORTH, EAST);
		return BearingAngle;
	}

	double radianss2(double dir){
		return dir * PI / 180;
	}
	float getRelativeWind2(float Wu, float Wv, float angle, float shipspeed, float * rangle){

		float rWind;
		float Wur, Wvr;

		Wur = Wu - shipspeed*cos(angle);
		Wvr = Wv - shipspeed*sin(angle);

		rWind = sqrt(pow(Wur, 2) + pow(Wvr, 2));
		//*rangle = fabs(atan2f(-Wvr, -Wur) - angle);// *180 / PI;
		*rangle = fabs(atan2f(-Wvr, -Wur) - angle);// *180 / PI;
		return rWind;
	}

	float getDriftAngle2(float Cu, float Cv, float angle, float shipspeed){

		float driftAngle;
		float Cur, Cvr;

		Cur = shipspeed*cos(angle) - Cu;
		Cvr = shipspeed*sin(angle) - Cv;

		float rSpeed = sqrt(pow(Cur, 2) + pow(Cvr, 2));
		driftAngle = fabs(atan2f(Cvr, Cur) - angle);
		return driftAngle;
	}

};