#pragma once

#include "stdafx.h"





enum DIRECTION{ X_POS, X_NEG, Y_POS, Y_NEG, Z_POS, Z_NEG, NONE };
using namespace std;
struct point
{
	int x;
	int y;
	int z;
	point()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	point(int _x, int _y, int _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
};

struct point_d
{
	double x;
	double y;
	double z;
	point_d()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	point_d(double _x, double _y, double _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
};

class grid
{
public:
	// current position
	int x;
	int y;
	int z;

	bool open;
	bool closed;
	bool obstacle;
	float G_value;
	float F_value;
	float H_value;

	int level;
	bool go_here;

public:

	grid(int _x, int _y, int _z)
	{
		x = _x;
		y = _y;
		z = _z;
		F_value = 0;
		open = 0;
		closed = 0;
		level = 0;
		go_here = 0;
		obstacle = 0;
	}
	grid() { x = 0;		y = 0;	 z = 0;   G_value = 0; H_value = 0;	F_value = 0; open = 0; closed = 0; level = 0; go_here = 0;  obstacle = 0; }
	bool operator ==(const grid& s) const    // 비교연산을 위한 오버로딩
	{
		return (x == s.x && y == s.y && z == s.z);
	}
	bool operator <(const grid& s) const    // 비교연산을 위한 오버로딩
	{
		return (H_value < s.H_value);
	}
	void reset() { x = 0;		y = 0; G_value = 0;	H_value = 0;	F_value = 0; open = 0; closed = 0; level = 0; go_here = 0; }
	void close() { closed = 1; }

	bool isopen()	{ return open; }
	void setG(double _level) { G_value = _level; }
	void setF(double _F) { F_value = _F; }
	void setXYZ(int _x, int _y, int _z) { x = _x, y = _y; z = _z; }
	void setlevel(int _level) { level = _level; }
};


struct path_data
{
	int x;
	int y;
	int z;
	double cost;
	DIRECTION pre_dir;
	path_data(int _x, int _y, int _z, double _cost, DIRECTION _pre_dir)
	{
		x = _x;
		y = _y;
		z = _z;
		cost = _cost;
		pre_dir = _pre_dir;
	}
	path_data()
	{
		x = 0; y = 0; z = 0; cost = 0;
	}

	bool operator ==(const path_data& s) const    // 비교연산을 위한 오버로딩
	{
		return (x == s.x && y == s.y && z == s.z);
	}

};

struct BOX
{
	TopoDS_Shape shape;
	double xangle;
	double yangle;
	double Xmin;
	double Ymin;
	double Zmin;
	double Xmax;
	double Ymax;
	double Zmax;
	double out_xmin, out_xmax, out_ymin, out_ymax, out_zmax, out_zmin;
	bool isOBB;
	bool isPipeIO;
	int boxindex;


	BOX(TopoDS_Shape _shape, double _xangle, double _yangle, double _Xmin, double _Xmax, double _Ymin, double _Ymax, double _Zmin, double _Zmax)
	{
		isPipeIO = 0;
		boxindex = 0;
		Xmin = min(_Xmin,_Xmax);
		Xmax = max(_Xmin, _Xmax);
		Ymin = min(_Ymin, _Ymax);
		Ymax = max(_Ymin, _Ymax);
		Zmin = min(_Zmin, _Zmax);
		Zmax = max(_Zmin, _Zmax);

		shape = _shape;
		xangle = _xangle;
		yangle = _yangle;
		if (xangle == 0 && yangle == 0)
		{
			isOBB = false;
//			printf("this is AABB\n");
//			printf("xmin :%lf, ymin : %lf, zmin: %lf, xmax : %lf, ymax : %lf, zmax %lf \n", Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);
		}
		else
		{
			double _out_xmin, _out_xmax, _out_ymin, _out_ymax, _out_zmax, _out_zmin;

			isOBB = true;
			Bnd_Box B;
			BRepBndLib::Add(shape, B);
			B.Get(_out_xmin, _out_ymin, _out_zmin, _out_xmax, _out_ymax, _out_zmax);

			out_xmin = min(_out_xmin, _out_xmax);
			out_xmax = max(_out_xmin, _out_xmax);
			out_ymin = min(_out_ymin, _out_ymax);
			out_ymax = max(_out_ymin, _out_ymax);
			out_zmin = min(_out_zmin, _out_zmax);
			out_zmax = max(_out_zmin, _out_zmax);

//			printf("this is OBB\n");
//			printf("out_xmin :%lf, out_ymin : %lf, out_zmin: %lf, out_xmax : %lf, out_ymax : %lf, out_zmax %lf \n", out_xmin, out_ymin, out_zmin, out_xmax, out_ymax, out_zmax);

		}
	}
	BOX()
	{

	}
};


struct searchcoord
{
	int x, y, z;
	searchcoord(int _x, int _y, int _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
	searchcoord()
	{}
};


struct Pipe_Info
{
	Handle(AIS_InteractiveObject) obj;
	gp_Pnt P1;
	gp_Pnt P2;
	gp_Pnt MainPnt;
	gp_Dir normal;
	int filenum;
	//CString name;
	bool isusing;
	double x, y, z;
	double radius;
	Pipe_Info(Handle(AIS_InteractiveObject) _obj, int _filenum, double _x, double _y, double _z, gp_Dir _normal, double _radius)
	{
		obj = _obj;
		isusing = 0;
		filenum = _filenum;
		x = _x;
		y = _y;
		z = _z;
		normal = _normal;
		radius = _radius;
	}
	void reset() { isusing = 0;}

	bool operator == (Handle(AIS_InteractiveObject) _obj) const   // 검색할때 라벨로 검색.
	{
		return obj == _obj;
	}
};

struct connection_info
{
	int dest;
	int src;
	connection_info() {}
	connection_info(int _dest, int _src)
	{
		dest = _dest;
		src = _src;
	}
};

// DISPLAY INFO 정의. 회전각이나 위치이동과 같은 데이터를 저장.
// grid, boundary pipe, import model, bounding box과 같은 플래그를 만들어서 구분 가능하게 설정.


enum shapetype {MODEL, BND_BOX, PIPE_IO, ROUTED_PIPE, BOUNDARY};
struct displayinfo
{
	Handle(AIS_InteractiveObject) obj;
	// 회전각이나 위치이동 정보
	// 여기에 추가
	////
	shapetype type;
	int fileindex;
	int boxindex;
	int pipeindex;
	int modelnum;

	gp_Trsf trsf;   // 이동 거리



	/// pipe의 파라미터 ///
	gp_Pnt P1, P2;
//	gp_Pnt MainPnt;
	gp_Dir normal;
	double radius;
//	point_d center;
	/////////////////////////////


	/// Routed_Pipe의 파라미터 //
	int dest;
	int src;
	vector<path_data> path;
	double extracost;
	double extralength;

	displayinfo(Handle(AIS_InteractiveObject) _obj, shapetype _type, int _fileindex = 0, int _boxindex = 0, int _modelnum = 0)
	{
		trsf.SetTranslation(gp_Vec(0, 0, 0));
		obj = _obj;
		type = _type;
		fileindex = _fileindex;
		boxindex = _boxindex;
		modelnum = _modelnum;
		if (type == BND_BOX)
		{

		}
		else if (type == PIPE_IO)
		{

		}
	}

	displayinfo(TopoDS_Shape shape, shapetype _type)
	{
		obj = new AIS_Shape(shape);
		type = _type;
	}

	displayinfo()
	{
		trsf.SetTranslation(gp_Vec(0, 0, 0));
		boxindex = 0;
		modelnum = 0;
	}

	bool operator == (Handle(AIS_InteractiveObject) _obj) const   // 검색할때 라벨로 검색.
	{
		return obj == _obj;
	}
};




extern vector<Pipe_Info> pipes;
extern vector<displayinfo> vis_container;
extern vector<BOX> boxes;     /// Boundary box 정보가 담긴 벡터
extern vector<connection_info> pipe_connection;


extern bool displaymodel;
extern bool displayboundary;
extern bool displaybndbox;
extern bool displaypipe;
extern bool pipeselection;