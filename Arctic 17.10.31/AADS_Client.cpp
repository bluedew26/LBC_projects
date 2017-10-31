#include "stdafx.h"
#include "AADS_Client.h"
#include "libxl.h"
#include "config.h"
using namespace std;
using namespace libxl;


/*
float* AADS_Client::KaistRequest_widely(int data, int x, int y, int w, int h, float time)
{
	float* a = new float;
	*a = 3;
	return a;
}
*/



double Rradians(double dir){
	return dir * 180 / PI;
}
double radians(double dir){
	return dir * PI / 180;
}

KRISOPoint AADS_Client::GridToLatLon(GRIDPoint gpoint)
{
	//gpoint.s_xgrid = 1.06900;
	//gpoint.s_ygrid = -2186.06900;

	KRISOPoint kpoint;

	double st_degrees = 70.0;
	double LongOrigin = 315.0;
	double a = 6378137.0;
	double eccentricity = 0.081819191;
	double eccent1 = sqrt(pow(1 + eccentricity, 1 + eccentricity)*pow(1 - eccentricity, 1 - eccentricity));
	double pp;
	double tt;
	double FEasting = 0.0;
	double FNorthing = -2500;
	double Tf = tan((PI / 4) - (radians(st_degrees) / 2))*pow(((1 + eccentricity*sin(radians(st_degrees))) / (1 - eccentricity*sin(radians(st_degrees)))), (eccentricity / 2));
	double mf = cos(radians(st_degrees)) / sqrt((1 - pow(eccentricity, 2)*pow(sin(radians(st_degrees)), 2)));
	double k0 = (mf*eccent1) / (2 * Tf);
	double xx;
	float lat;
	float lon;

	pp = sqrt(pow(gpoint.s_xgrid * 1000 - FEasting, 2) + pow(gpoint.s_ygrid * 1000 - FNorthing, 2));
	tt = (pp*eccent1) / (2 * a*k0);
	xx = PI / 2 - (2 * atan(tt));
	lon = radians(LongOrigin) + atan2((FNorthing - gpoint.s_ygrid * 1000), (gpoint.s_xgrid * 1000 - FEasting));
	lat = ((1.0 / 2) * pow(eccentricity, 2) + (5.0 / 24) * pow(eccentricity, 4) + (1.0 / 12)*pow(eccentricity, 6) + (13.0 / 360)*pow(eccentricity, 8))*sin(2 * xx) +
		((7.0 / 48) * pow(eccentricity, 4) + (29.0 / 240)*pow(eccentricity, 6) + (811.0 / 11520)*pow(eccentricity, 8))*sin(4 * xx) +
		((7.0 / 120)*pow(eccentricity, 6) + (81.0 / 1120)*pow(eccentricity, 8))*sin(6 * xx) +
		(4279.0 / 161280)*pow(eccentricity, 8)*sin(8 * xx) + xx;


	//printf("lat = %.5f\n", Rradians(lat));
	//printf("lon = %.5f\n", -Rradians(lon)+360);


	kpoint.s_lat = Rradians(lat);
	kpoint.s_lon = -Rradians(lon) + 360;

	return kpoint;
}

float AADS_Client::GetSpeed(int x1, int y1, int x2, int y2, float v, double time, float datas[8]) // 이미 알고있는 값이라면 데이터를 추가로 제공한다.
{
	for (int i = 0; i < 8; i++)
	{
		if (datas[i] == INVALID_VALUE)
		{
			if (i == 0 || i == 1)
				return v;
			else
				datas[i] = 0;
			//return v;			// 모르는 값이 포함되어있으면 기존 v값을 리턴
		}
	}


	v = v / KNOTS_TO_M_PER_SEC; /// m/s에서 노트로 변환
	time = time / 3600;			/// 초에서 시간으로 변환
	float Result[4] = { 0 };
	float v2 = 0, v3 = 0, v4 = 0;
	float wind_U, wind_V;
	float current_U, current_V;
	double BrakePower;
	//ICH  빙두께
	//ICF  빙집적도
	//	v = 12;
	float waveHeight = 0;
	float swellHeight = 0;

	Result[0] = datas[0];
	Result[1] = datas[1];
	Result[2] = datas[2];
	Result[3] = datas[3];
	wind_U = datas[4];
	wind_V = datas[5];
	waveHeight = datas[6];
	swellHeight = datas[7];
	
	/*
	waveHeight = 0;
	swellHeight = 0;
	wind_U = 0;
	wind_V = 0;
	Result[0] = 0;
	Result[1] = 0;
	Result[2] = 0;
	Result[3] = 0;
	*/
	current_U = Result[2];
	current_V = Result[3];

	
	IndexPoint gpoint;
	gpoint.s_xindex = x1; gpoint.s_yindex = y2;
	gpoint.e_xindex = x2; gpoint.e_yindex = y2;

	if (v > 15)
		int a = 3;
	float shipSpeed = v;
	float shipAngle = getBearingAngle2(gpoint);//방위각 Bearing Angle
	
//	shipAngle = 0;

	float airAngle = wind_V;
	float airSpeed = getRelativeWind2(wind_U, wind_V, shipAngle, shipSpeed, &airAngle);

	//airAngle = PI;

	float driftAngle = getDriftAngle2(current_U, current_V, shipAngle, shipSpeed);/// driftangle내부 식에서 쓰레기값이 사용됨
	float hullRough = config[2].ROUGHHULL; /// 확인 필요


	BrakePower = config[2].getPower2(shipSpeed, shipAngle, waveHeight, swellHeight, airSpeed, airAngle, driftAngle, hullRough, v);


	///김현수 교수님  attain speed -> Excel데이터를 포함하므로 일단 보류
	double attspeed;
	attspeed = Attainspeed2(Result[0], Result[1], v);



	//	attspeed = 0;



	BrakePower = BrakePower + attspeed;
	
	if (Result[0] < 0) Result[0] = 0;
	if (Result[1] < 0) Result[1] = 0;




	/// 이것의 영향이 좀 커 보인다. 근데 기준이 ICF인데...
	if (Result[0] <= 0.6){
		//	v2 = pow(2390 / BrakePower, 0.28571) * v;
		v2 = pow(2390 / BrakePower, 0.28571) * v;
	}
	else if (Result[0] > 0.6){
		v2 = pow(3400 / BrakePower, 0.28571) * v;
	}
	//v2 += attspeed;
	if (v2 < 2)
		int a = 3;

	return v2 * KNOTS_TO_M_PER_SEC;  /// m/s로 반환

}

void AADS_Client::Init()
{
	// get new clients
	readConfig2(&config[2], "ARAON1S - 11Aㅊㅚㅈㅗㅇ.SDB");
}









void AADS_Client::ExcelInit()
{
	book60 = xlCreateBook();
	book70 = xlCreateBook();
	book80 = xlCreateBook();
	book90 = xlCreateBook();
	book100 = xlCreateBook();
	book60->load(L"Attainablespeed60.xls");
	book70->load(L"Attainablespeed70.xls");
	book80->load(L"Attainablespeed80.xls");
	book90->load(L"Attainablespeed90.xls");
	book100->load(L"Attainablespeed100.xls");
	sheet60 = book60->getSheet(0);
	sheet70 = book70->getSheet(0);
	sheet80 = book80->getSheet(0);
	sheet90 = book90->getSheet(0);
	sheet100 = book100->getSheet(0);

}
void AADS_Client::ExcelRelease()
{
	book60->release();
	book70->release();
	book80->release();
	book90->release();
	book100->release();
}


int AADS_Client::Attainspeed2(float ih, float ifd, float v)
{
	int countxl = 0;
	//ih = 빙두께
	//ihd = 빙집적도
	//v = 속도
	double integer, fraction;
	int first, second, point;
	fraction = modf(v, &integer);
	second = (fraction * 10) + 2;
	if (integer == 3)
	{
		first = 0;
		point = first + second;
	}
	else if (integer == 4)
	{
		first = 10;
		point = first + second;
	}
	else if (integer == 5)
	{
		first = 20;
		point = first + second;
	}
	else if (integer == 6)
	{
		first = 30;
		point = first + second;
	}
	else if (integer == 7)
	{
		first = 40;
		point = first + second;
	}
	else if (integer == 8)
	{
		first = 50;
		point = first + second;
	}
	else if (integer == 9)
	{
		first = 60;
		point = first + second;
	}
	else if (integer == 10)
	{
		first = 70;
		point = first + second;
	}
	else if (integer == 11)
	{
		first = 80;
		point = first + second;
	}
	else if (integer == 12)
	{
		first = 90;
		point = first + second;
	}
	else if (integer == 13)
	{
		first = 100;
		point = first + second;
	}
	else if (integer == 14)
	{
		first = 110;
		point = first + second;
	}
	else if (integer == 15)
	{
		first = 120;
		point = first + second;
	}
	else if (integer == 16)
	{
		first = 130;
		point = first + second;
	}
	else if (integer == 17)
	{
		first = 140;
		point = first;
	}
	else if (integer > 17)
	{
		first = 140;
		point = first;
	}

	double s = 0;

//	return s;
	if (ifd > 0.6 && ifd<0.7 && ih >0.6 && ih<0.7 && v>2.9)
	{
		s = sheet60->readNum(point, 8);
	}
	else if (ifd > 0.7 && ifd<0.8 && ih >0.6 && ih<0.7 && v>2.9)
	{
		s = sheet60->readNum(point, 10);
	}
	else if (ifd > 0.8 && ifd<0.9 && ih >0.6 && ih<0.7 && v>2.9)
	{
		s = sheet60->readNum(point, 12);
	}
	else if (ifd > 0.9 && ifd<1 && ih >0.6 && ih<0.7 && v>2.9)
	{
		s = sheet60->readNum(point, 14);
	}
	else if (ifd == 1 && ih > 0.6 && ih<0.7 && v>2.9)
	{
		s = sheet60->readNum(point, 14);
	}
	else if (ifd > 0.6 && ifd<0.7 && ih >0.7 && ih<0.8 && v>2.9)
	{
		s = sheet70->readNum(point, 8);
	}
	else if (ifd > 0.7 && ifd<0.8 && ih >0.7 && ih<0.8 && v>2.9)
	{
		s = sheet70->readNum(point, 10);
	}
	else if (ifd > 0.8 && ifd<0.9 && ih >0.7 && ih<0.8 && v>2.9)
	{
		s = sheet70->readNum(point, 12);
	}
	else if (ifd > 0.9 && ifd< 1 && ih >0.7 && ih<0.8 && v>2.9)
	{
		s = sheet70->readNum(point, 14);
	}
	else if (ifd == 1 && ih >0.7 && ih<0.8 && v>2.9)
	{
		s = sheet70->readNum(point, 14);
	}
	else if (ifd > 0.6 && ifd<0.7 && ih >0.8 && ih<0.9 && v>2.9)
	{
		s = sheet80->readNum(point, 8);
	}
	else if (ifd > 0.7 && ifd<0.8 && ih >0.8 && ih<0.9 && v>2.9)
	{
		s = sheet80->readNum(point, 10);
	}
	else if (ifd > 0.8 && ifd<0.9 && ih >0.8 && ih<0.9 && v>2.9)
	{
		s = sheet80->readNum(point, 12);
	}
	else if (ifd > 0.9 && ifd< 1 && ih >0.8 && ih<0.9 && v>2.9)
	{
		s = sheet80->readNum(point, 14);
	}
	else if (ifd == 1 && ih >0.8 && ih<0.9 && v>2.9)
	{
		s = sheet80->readNum(point, 14);
	}
	else if (ifd > 0.6 && ifd<0.7 && ih >0.9 && ih<1 && v>2.9)
	{
		s = sheet90->readNum(point, 8);
	}
	else if (ifd > 0.7 && ifd<0.8 && ih >0.9 && ih<1 && v>2.9)
	{
		s = sheet90->readNum(point, 10);
	}
	else if (ifd > 0.8 && ifd<0.9 && ih >0.9 && ih<1 && v>2.9)
	{
		s = sheet90->readNum(point, 12);
	}
	else if (ifd > 0.9 && ifd< 1 && ih >0.9 && ih<1 && v>2.9)
	{
		s = sheet90->readNum(point, 14);
	}
	else if (ifd == 1 && ih >0.9 && ih<1 && v>2.9)
	{
		s = sheet90->readNum(point, 14);
	}
	else if (ifd > 0.6 && ifd<0.7 && ih >1 && v>2.9)
	{
		s = sheet100->readNum(point, 8);
	}
	else if (ifd > 0.7 && ifd<0.8 && ih >1 && v>2.9)
	{
		s = sheet100->readNum(point, 10);
	}
	else if (ifd > 0.8 && ifd<0.9 && ih >1 && v>2.9)
	{
		s = sheet100->readNum(point, 12);
	}
	else if (ifd > 0.9 && ifd< 1 && ih >1 && v>2.9)
	{
		s = sheet100->readNum(point, 14);
	}
	else if (ifd == 1 && ih >1 && v>2.9)   //(ifd > 1 && ih >0.9 && ih <1 && v>2.9)
	{
		s = sheet100->readNum(point, 14);
	}
	else if (ih <0.6)
	{
		return 0;
	}
	return s;
}
