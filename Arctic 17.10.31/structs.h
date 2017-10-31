#pragma once

#include <set>
using namespace std;

struct Datas
{
	int hour;   /// 
	float ice_t;
	float ice_c;
	float cur_u;
	float cur_v;
	float wind_U;
	float wind_V;
	float waveHeight;
	float swellHeight;

	Datas(){ hour = -1; ice_t = 0; ice_c = 0; cur_u = 0; cur_v = 0; wind_U = 0; wind_V = 0; waveHeight = 0; swellHeight = 0;  }
	Datas(int _hour){ hour = _hour; ice_t = 0; ice_c = 0; cur_u = 0; cur_v = 0; wind_U = 0; wind_V = 0; waveHeight = 0; swellHeight = 0; }
	Datas(int _hour, float _ice_t, float _ice_c, float _cur_u, float _cur_v, float _wind_U, float _wind_V, float _waveHeight, float _swellHeight, bool _invalid = false)
	{
		hour = _hour;
		ice_t = _ice_t;
		ice_c = _ice_c;
		cur_u = _cur_u;
		cur_v = _cur_v;
		wind_U = _wind_U;
		wind_V = _wind_V;
		waveHeight = _waveHeight;
		swellHeight = _swellHeight;
	}
	bool operator ==(const Datas& s) const    // 비교연산을 위한 오버로딩
	{
		return (hour == s.hour);
	}
	bool operator ==(const int h) const    // 비교연산을 위한 오버로딩
	{
		return (hour == h);
	}
	bool operator <(const Datas& s) const    // 비교연산을 위한 오버로딩
	{
		return (hour < s.hour);
	}
};


enum DIRECTION { NONE, LEFT, UP, DOWN, RIGHT, LEFT_UP, LEFT_DOWN, RIGHT_UP, RIGHT_DOWN };

class node
{
public:
	bool isflooded;
	bool isblocked;
	bool isnextpoint;
	double cost;
	double time;
	int dir;
	float speed;
	Datas datas[MAX_DATA];
	double depth;

	int dataindex;

	node() 
	{
		isflooded = false; cost = 0; dataindex = 0; isblocked = false; dir = NONE; speed = 0; time = 0; depth = -HULLDEPTH - 1; isnextpoint = false;
	}
	void add_data(Datas d)
	{
		if (dataindex < MAX_DATA)
		{
			datas[dataindex] = d;
			dataindex++;
		}
		else
		{
			for (int i = 1; i < MAX_DATA; i++)
			{
				datas[i - 1] = datas[i];
			}
			dataindex = MAX_DATA - 1;
			datas[dataindex] = d;
			dataindex++;
		}
	}
	int find(int hour) // 못찾으면 -1 반환
	{
		if (pre_load)
		{
			return 0; // 0번째 인덱스 리턴
		}
		if (hour >= MAX_DATA * TIME_GAP)
		{
			return MAX_DATA - 1;
		}
		
		for (int i = 0; i < dataindex; i++)
		{
			if (datas[i].hour == hour)
				return i;
		}
		return -1;
	}
	void reset_data()
	{
		for (int i = 0; i < dataindex; i++)
		{
			datas[i] = Datas();
		}
		dataindex = 0;
	}
};

struct path_data
{
	int x;
	int y;
	double time;
	double cost;
	double speed;
	path_data(int _x, int _y, double _cost, double _time, double _speed)
	{
		x = _x;
		y = _y;
		time = _time;
		cost = _cost;
		speed = _speed;
	}
	path_data()
	{
	}
	bool operator ==(const path_data& s) const    // 비교연산을 위한 오버로딩
	{
		return (x == s.x && y == s.y);
	}

};

#define NONE 0
#define LEFT 1
#define RIGHT 2
#define DOWN 3
#define UP 4
#define LEFT_DOWN 5
#define LEFT_UP 6
#define RIGHT_UP 7
#define RIGHT_DOWN 8

struct ptr
{
	int x;
	int y;

	int direction; // 0 = LEFT, 1 = RIGHT, 2 = DOWN, 3 = UP, 4 = LEFT_DOWN, 5 = LEFT_UP,  6 = RIGHT_UP, 7 = RIGHT_DOWN
	float v;
	double costused;
	double time;


	ptr() { x = 0; y = 0; direction = 0; v = 0; costused = 0; time = 0; }
	ptr(int _x, int _y, int _direction, float _v, double _costused, double _time)
	{
		x = _x;
		y = _y;
		direction = _direction;
		v = _v;
		costused = _costused;
		time = _time;
	}

	bool operator ==(const ptr &p) const    // 검색용 오버로딩
	{
		if (x == p.x && y == p.y)
			return true;
		else
			return false;
	}
	bool operator <(const ptr &p) const
	{
		if (x < p.x)
			return true;
		else if (x == p.x && y < p.y)
			return true;
		else
			return false;
	}
};