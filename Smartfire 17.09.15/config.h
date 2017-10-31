// Interpolation 없이 Resize하기!!!!
#pragma once
// 원의 크기 - 40px;
#include "stdafx.h"
/// 컬러 정보
#include <vector>
#include <set>
#include <stack>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <opencv2\core.hpp>
using namespace std;


#define TYPE_HYD_0 0
#define TYPE_HYD_1 1
#define TYPE_HYD_2 2
#define TYPE_HOSEREEL 3
#define TYPE_EXTINGUISHER 4
#define TYPE_MONITOR 5

struct ScaleLine
{
	int x1, x2, y;
	double length;
	ScaleLine(){}
	ScaleLine(int _x1, int _x2, int _y, double _length)
	{
		x1 = _x1;
		x2 = _x2;
		y = _y;
		length = _length;
	}
};

struct COLOR
{
	unsigned char R, G, B;

	COLOR(){}
	COLOR(unsigned char _R, unsigned char _G, unsigned char _B)
	{
		R = _R;
		G = _G;
		B = _B;
	}
	bool operator ==(const COLOR &col) const    // 비교용
	{
		return (R == col.R && G == col.G && B == col.B);
	}
};

struct COLORPATTERN
{
	unsigned int cumulR, cumulG, cumulB; // 누적값
	int count; // 중첩횟수
	COLORPATTERN(){ cumulR = 0, cumulG = 0, cumulB = 0, count = 0;}
};

#define NUMOFLAYER 3

struct Node
{
	bool isobstacle[NUMOFLAYER];   // 빨간색 벽 영역
	bool isequipment;
	bool iswall;	   // 연두색 벽 영역
	bool ispath;
	bool multipurpose;
	bool istempstreamable;
	bool isfeasible;
	bool istemprecommended;
	bool isinfeasible; // 배치 불가능 지점
	bool isdoor;
	bool doubleoverlapped;
	bool isstreamable;
	bool isfloodable;
	bool isrecommended;

	bool checked;

	bool isoutofwall;
	bool isoutofwd;
	bool isoutofpath;
	bool isstair;
	bool lossed;       // 연속된 선에서 끊긴 부분


	int unum; // trim에 사용되는 고유번호, 어느 선으로부터 만들어진 것인지
	double costweight;


	int stairindex;
	Node()
	{
		for (int i = 0; i < NUMOFLAYER; i++)
			isobstacle[i] = 0;
		isequipment = 0;
		iswall = 0;
		isstair = 0;
		ispath = 0;
		isstreamable = 0;
		isfloodable = 0; 
		isfeasible = 0;
		isrecommended = 0;
		multipurpose = 0;
		isdoor = 0;
		doubleoverlapped = 0;
		istempstreamable = 0;
		istemprecommended = 0;
		unum = 0;
		isinfeasible = 0;
		checked = 0;
		isoutofwall = 0;
		isoutofwd = 0;
		isoutofpath = 0;
		stairindex = -1;
		costweight = 1;
	}
};

enum DIRECTION { NONE, LEFT, UP, DOWN, RIGHT, LEFT_UP, LEFT_DOWN, RIGHT_UP, RIGHT_DOWN};


struct node
{
	int x;
	int y;
	int last_dir; // 반시계방향
	node() { x = 0; y = 0;  }
	node(int _x, int _y) { x = _x; y = _y; last_dir = 0; }

	bool operator ==(const node &n) const  
	{
		if (x == n.x && y == n.y)
			return true;
		else
			return false;
	}

	bool operator <(const node &n)const
	{
		if (x < n.x)
			return true;
		else if (x == n.x && y < n.y)
			return true;
		else
			return false;
	}

};


struct ptr
{
	int x;
	int y;

	ptr() { x = 0; y = 0;}
	ptr(int _x, int _y)
	{
		x = _x;
		y = _y;
	}
	bool operator ==(const ptr &p) const    // 검색용 오버로딩
	{
		// 만약에 NONE이 아닐경우 해당 방향으롣 덮어씌워버림.
		if (x == p.x && y == p.y)
			return true;
		else
			return false;
	}


	bool operator <(const ptr &p)const
	{
		if (x < p.x)
			return true;
		else if (x == p.x && y < p.y)
			return true;
		else
			return false;
	}
};


struct LocMap
{
	bool isflooded;
	bool isstreamed;
	bool isoutline;
	bool isfinalpoint;
	bool extended;
	DIRECTION dir;
	double rdis;
	LocMap()
	{
		isflooded = false; isstreamed = false; isoutline = false; dir = NONE; rdis = 0; extended = false; isfinalpoint = true;
	}
};

struct HYDRANT
{
	int x, y;   // 시작점
	int xx[3], yy[3]; // Monitor일때만 사용
	int rel_x, rel_y; // LocMap 그리드상에서 시작점의 index
	int sourcenum; //소스 넘버
	int colorindex;
	int unum; // 고유번호
	double length;
	double streamlength;
	bool highlighted;
	int type; // 0은 소화전(0), 1은 소화전(1), 2는 소화전(2), 3은 HOSEREEL, 4는 소화기
	bool activate;
	int needtoupdate; // 0은 업데이트 불필요, 1은 전체, 2는 Stream만


	vector<vector<LocMap>> LMap; // flooding 할때만 갱신
	/// 스타트점은 x,y를 보면 알 수 있음.
	vector<ptr> range;
	vector<ptr> stream;
	vector<ptr> outline;
	vector<ptr> guidebox;
	set<ptr> wallblocked;
	HYDRANT(){ unum = 0; }
	HYDRANT(int _x, int _y, double _streamlength, double _length, int _type, int _sourcenum = 0, int _colorindex = 0)
	{
		x = _x;
		y = _y;
		colorindex = _colorindex;
		streamlength = _streamlength;
		sourcenum = _sourcenum;
		length = _length;
		type = _type;
		activate = true;
		highlighted = 0;
		needtoupdate = 1;
		rel_x = 0;
		rel_y = 0;
	}
	HYDRANT(int x1, int x2, int x3, int y1, int y2, int y3, double _streamlength, int _colorindex)
	{
		xx[0] = x1; xx[1] = x2; xx[2] = x3;
		yy[0] = y1; yy[1] = y2; yy[2] = y3;
		colorindex = _colorindex;
		activate = true;
		highlighted = 0;
		needtoupdate = 1;
		streamlength = _streamlength;
		type = TYPE_MONITOR;
		length = 0;
		sourcenum = 0;
		x = x1;
		y = y1;
	}
	void reset()
	{
		LMap.clear();
		range.clear();
		stream.clear();
		outline.clear();
		guidebox.clear();
		wallblocked.clear();
		needtoupdate = 1;
	}

	bool operator ==(const HYDRANT &p) const    // 검색용 오버로딩
	{
		return x == p.x && y == p.y;
	}
	bool operator ==(const int uniquenum) const
	{
		return unum == uniquenum;
	}
	bool operator <(const HYDRANT &p)const
	{
		if (type < p.type) // 타입별로 정렬
			return true;
		else
			return false;
	}
};


struct optptr
{
	float vel_u;
	float vel_v;
	double overlap_rate;
	bool fixed;
	HYDRANT hyd;
	optptr() {}
	optptr(int _x, int _y, double _length, float _vel_u, float _vel_v, double _streamlength, double _sourcenum)
	{
		hyd = HYDRANT(_x, _y, _streamlength, _length, 0, _sourcenum, 0);
		vel_u = _vel_u;
		vel_v = _vel_v;
		overlap_rate = 0;
		fixed = false;
	}
	bool operator ==(const optptr &p) const    // 검색용 오버로딩
	{
		// 만약에 NONE이 아닐경우 해당 방향으롣 덮어씌워버림.
		if (hyd.x == p.hyd.x && hyd.y == p.hyd.y)
			return true;
		else
			return false;
	}
	bool operator ==(const double _length) const
	{
		if (hyd.length == _length)
			return true;
	}
	bool operator <(const optptr &p)const
	{
		if (hyd.x < p.hyd.x)
			return true;
		else if (hyd.x == p.hyd.x && hyd.y < p.hyd.y)
			return true;
		else
			return false;
	}
};


struct drawinginfo
{
	int x1, y1, x2, y2;  // 그리드 단위의 상대좌표
	int type_2p;            // 0은 벽, 1은 장애물?
	int type_object;
	int layernum;
	int unum;  // 고유 번호
	drawinginfo(){}
	drawinginfo(int _x1, int _y1, int _x2, int _y2, int _type_2p, int _type2_object, int _unum, int _layernum = 0)
	{
		x1 = _x1, y1 = _y1, x2 = _x2, y2 = _y2;
		type_2p = _type_2p;
		type_object = _type2_object;
		unum = _unum;
		layernum = _layernum;
	}
	bool operator ==(const int uniquenum) const
	{
		return unum == uniquenum;
	}
};
enum ActionType { ADD_HYDRANT, DELETE_HYDRANT, MODIFY_HYDRANT, DRAW_RECTANGLE, DRAW_LINE, DRAW_DOUBLELINE, DRAW_CIRCLE, DRAW_TRIM, RESET };
struct ActionStack
{
	ActionType type; // 타입에 따라 최근 몇개나 지울 지 결정 
	HYDRANT prev_hyd; // ADD/DELETE/MODIFY 시 기억
	drawinginfo trimmed;
	int trimmedindex;
	int numofadded;
	ActionStack(){}
	ActionStack(ActionType _type) { type = _type; }
	ActionStack(ActionType _type, HYDRANT _prev_hyd) { type = _type; prev_hyd = _prev_hyd; }
	ActionStack(ActionType _type, drawinginfo _trimmed, int triindex, int _numofadded) { type = _type; trimmed = _trimmed; trimmedindex = triindex; numofadded = _numofadded; }
};
