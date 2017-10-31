
// TESTOCCView.cpp : CTESTOCCView 클래스의 구현
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "TESTOCC.h"
#endif

#include "TESTOCCDoc.h"
#include "TESTOCCView.h"
#include "PartListDialog.h"
#include <Mathematics/GteMinimumVolumeBox3.h>
#include <vector>
#include <iostream>
#include <cmath>

#include "config.h"


// CTESTOCCView

IMPLEMENT_DYNCREATE(CTESTOCCView, CView)
BEGIN_MESSAGE_MAP(CTESTOCCView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()
// CTESTOCCView 생성/소멸
using namespace std;
void CTESTOCCView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	myView = ((CTESTOCCDoc*)GetDocument())->GetViewer()->CreateView();
	myHlrModeIsOn = Standard_False;
	myView->SetComputedMode(myHlrModeIsOn);
	Handle(Graphic3d_GraphicDriver) aGraphicDriver = ((CTESTOCCApp*)AfxGetApp())->GetGraphicDriver();
	Handle(WNT_Window) aWNTWindow = new WNT_Window(GetSafeHwnd());
	myView->SetWindow(aWNTWindow);

	if (!aWNTWindow->IsMapped()) aWNTWindow->Map();
	aWNTWindow->SetBackground(Quantity_NOC_SLATEBLUE2);
	// TODO: 在此添加专用代码和/或调用基类

	Standard_Integer w = 100;
	Standard_Integer h = 100;
	aWNTWindow->Size(w, h);
	::PostMessage(GetSafeHwnd(), WM_SIZE, SIZE_RESTORED, w + h * 65536);
	myView->FitAll();
	myView->ZBufferTriedronSetup(Quantity_NOC_RED, Quantity_NOC_GREEN, Quantity_NOC_BLUE1, 0.8, 0.05, 12);
	myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_WHITE, 0.2, V3d_ZBUFFER);

}
CTESTOCCView::CTESTOCCView()
{
	// TODO: 여기에 생성 코드를 추가합니다.

}
CTESTOCCView::~CTESTOCCView()
{
}
BOOL CTESTOCCView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}
// CTESTOCCView 그리기
BOOL CTESTOCCView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}
void CTESTOCCView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}
void CTESTOCCView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}
// CTESTOCCView 진단
#ifdef _DEBUG
void CTESTOCCView::AssertValid() const
{
	CView::AssertValid();
}

void CTESTOCCView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTESTOCCDoc* CTESTOCCView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTESTOCCDoc)));
	return (CTESTOCCDoc*)m_pDocument;
}
#endif //_DEBUG
// CTESTOCCView 메시지 처리기
void CTESTOCCView::OnDraw(CDC* /*pDC*/)
{
	CTESTOCCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	////////////////////////////////////


	myView->Redraw();
	myView->MustBeResized();
	myView->Update();
	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
}



//////// 전역 변수  /////////
bool Lbuttondown = false;
int X_Coord = 0;
int Y_Coord = 0;
int X_Rot = 0;
int Y_Rot = 0;
int ddx = 0;
int ddy = 0;
/////////////////////////////



bool CTESTOCCView::Collision_box_and_triangle(TopoDS_Shape triangle)
{
	if (boxes.empty()) // 박스가 하나도 없으면 리턴
		return false;

	vector<BOX>::iterator it;
	for (it = boxes.begin(); it != boxes.end(); it++)
	{
		TopoDS_Shape Intersec = BRepAlgoAPI_Common(triangle, it->shape);
		/// calculate volume
		GProp_GProps System;
		BRepGProp::VolumeProperties(Intersec, System);
		double volume = System.Mass();
		///
		if (volume >= 0.00001)
		{
			return true;
		}
	}
	return false;
}


void  CTESTOCCView::displaybox(double x, double y, double z)
{
//	static int count = 0;
//	count++;
//	printf("count : %d\n", count);

	CTESTOCCDoc* pDoc = GetDocument();
	const Handle(AIS_InteractiveContext) aCon = pDoc->myAISContext;

	BRepPrimAPI_MakeBox box = BRepPrimAPI_MakeBox(gp_Pnt(x-GRIDSIZE/2.0,y-GRIDSIZE/2.0,z-GRIDSIZE/2.0),gp_Pnt(x+GRIDSIZE/2.0,y+GRIDSIZE/2.0,z+GRIDSIZE/2.0));


	TopoDS_Shape sh_box = box.Shape();
	

	AIS_InteractiveObject *todisplay = new AIS_Shape(sh_box);


	todisplay->SetTransparency(0.8); // 투명도 설정
	todisplay->SetColor(Quantity_Color(0, 0, 1, Quantity_TOC_RGB)); // 색상 설정
	aCon->Display(todisplay);
}

int CTESTOCCView::Collision_box_and_coord_without_pipe(gp_Pnt pnt)  // 리턴값은 boxindex
{
	if (boxes.empty()) // 박스가 하나도 없으면 리턴
		return false;

	double xstart = MIN_X_COORD;
	double ystart = MIN_Y_COORD;
	double zstart = MIN_Z_COORD;
	double delta = GRIDSIZE;

	/// 점과 박스와의 판정

	double coordx = pnt.X();
	double coordy = pnt.Y();
	double coordz = pnt.Z();

	for (int i = 0; i<boxes.size(); i++)    // 모든 박스와 검사
	{
		if (boxes[i].isPipeIO == 1)    // 파이프는 검사 무시
			continue;
		if (boxes[i].isOBB == false)    // 박스가 AABB면
		{
			if (coordx >= boxes[i].Xmin && coordx <= boxes[i].Xmax && coordy >= boxes[i].Ymin && coordy <= boxes[i].Ymax && coordz >= boxes[i].Zmin && coordz <= boxes[i].Zmax)
			{  
				return boxes[i].boxindex;
			}
		}
		else    // 박스가 OBB면
		{

			/// inverse transform으로 판정
			gp_Pnt pt;
			pt.SetX(coordx); pt.SetY(coordy); pt.SetZ(coordz);
			double centerX = (boxes[i].Xmin + boxes[i].Xmax) / 2.0;
			double centerY = (boxes[i].Ymin + boxes[i].Ymax) / 2.0;
			double centerZ = (boxes[i].Zmin + boxes[i].Zmax) / 2.0;
			gp_Pnt center = gp_Pnt(centerX, centerY, centerZ);

			pt.Rotate(gp_Ax1(center, gp_Dir(1, 0, 0)), boxes[i].xangle * pi / 180);
			pt.Rotate(gp_Ax1(center, gp_Dir(0, 1, 0)), boxes[i].yangle * pi / 180);


			/// 이부분 요주의

			double tr_coordx = pt.X();
			double tr_coordy = pt.Y();
			double tr_coordz = pt.Z();   // 트랜스폼된 좌표들을 받음.




			if (tr_coordx >= boxes[i].Xmin && tr_coordx <= boxes[i].Xmax && tr_coordy >= boxes[i].Ymin && tr_coordy <= boxes[i].Ymax && tr_coordz >= boxes[i].Zmin && tr_coordz <= boxes[i].Zmax)
			{  
				return boxes[i].boxindex;
			}
		}
	}
	return -1;

}


bool CTESTOCCView::Collision_box_and_grid(int x, int y, int z)
{
	if (boxes.empty()) // 박스가 하나도 없으면 리턴
		return false;


//	static int count = 0;
//	printf("count = %d\n", count);
//	count++;

	double xstart = MIN_X_COORD;
	double ystart = MIN_Y_COORD;
	double zstart = MIN_Z_COORD;
	double delta = GRIDSIZE;

	/// 점과 박스와의 판정

	double coordx = xstart + (x*delta);
	double coordy = ystart + (y*delta);
	double coordz = zstart + (z*delta);

//	printf("x : %d, y: %d, z: %d ", x,y,z);
	for (int i = 0; i<boxes.size(); i++)    // 모든 박스와 검사
	{
		if (boxes[i].isOBB == false)    // 박스가 AABB면
		{
			if (coordx >= boxes[i].Xmin && coordx <= boxes[i].Xmax && coordy >= boxes[i].Ymin && coordy <= boxes[i].Ymax && coordz >= boxes[i].Zmin && coordz <= boxes[i].Zmax)
			{  // 완전히 AABB의 내부에 있으면
		//		printf("test222222");
				return true;
			}
		}
		else    // 박스가 OBB면
		{

			/// inverse transform으로 판정
			gp_Pnt pt;
			pt.SetX(coordx); pt.SetY(coordy); pt.SetZ(coordz);
			double centerX = (boxes[i].Xmin + boxes[i].Xmax) / 2.0;
			double centerY = (boxes[i].Ymin + boxes[i].Ymax) / 2.0;
			double centerZ = (boxes[i].Zmin + boxes[i].Zmax) / 2.0;
			gp_Pnt center = gp_Pnt(centerX, centerY, centerZ);

			pt.Rotate(gp_Ax1(center, gp_Dir(1, 0, 0)), boxes[i].xangle * pi / 180);
			pt.Rotate(gp_Ax1(center, gp_Dir(0, 1, 0)), boxes[i].yangle * pi / 180);

		
			/// 이부분 요주의

			double tr_coordx = pt.X();
			double tr_coordy = pt.Y();
			double tr_coordz = pt.Z();   // 트랜스폼된 좌표들을 받음.




			if (tr_coordx >= boxes[i].Xmin && tr_coordx <= boxes[i].Xmax && tr_coordy >= boxes[i].Ymin && tr_coordy <= boxes[i].Ymax && tr_coordz >= boxes[i].Zmin && tr_coordz <= boxes[i].Zmax)
			{  // 완전히 AABB의 내부에 있으면
				return true;
			}
			//		if (tr_coordx >= boxes[i].out_xmin && tr_coordx <= boxes[i].out_xmax && tr_coordy >= boxes[i].out_ymin && tr_coordy <= boxes[i].out_ymax && tr_coordz >= boxes[i].out_zmin && tr_coordz <= boxes[i].out_zmax)
			//		{  // 완전히 AABB의 내부에 있으면
			//			///디버그용. 장애물박스 표시
			//
			//				return true;
			//			}
		}
	}
	return false;

}


void CTESTOCCView::Collect_BB()
{
	CTESTOCCDoc* pDoc = GetDocument();
	const Handle(AIS_InteractiveContext) aCon = pDoc->myAISContext;

	boxes.clear(); // 박스 컨테이너 비움

	/// Part로 전체 selection
//	pDoc->Selection(SELECT_PART);
//	aCon->SetAutoActivateSelection(Standard_False);
	aCon->Select(-2000, -2000, 2000, 2000, myView);  // 이게 필요했다!!
	aCon->UnhilightSelected();
	for (aCon->InitSelected(); aCon->MoreSelected(); aCon->NextSelected())
	{
		TopoDS_Shape ashape = aCon->SelectedShape();
		BOX B = CreateOBB(ashape);
		boxes.push_back(B);
	}
//	pDoc->End_Selection();
}


TopoDS_Shape CTESTOCCView::getcylinder(double x1, double x2, double y1, double y2, double z1, double z2, double radius)
{
	gp_Pnt P1(x1, y1, z1);
	gp_Pnt P2(x2, y2, z2);
	Handle(Geom_TrimmedCurve) Seg1 = GC_MakeSegment(P1, P2);
	TopoDS_Edge Edge1 = BRepBuilderAPI_MakeEdge(Seg1);
	gp_Dir dir = gp_Dir(x2 - x1, y2 - y1, z2 - z1);
	gp_Ax2 axis = gp_Ax2(P1, dir);
	double height = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
	TopoDS_Shape shape = BRepPrimAPI_MakeCylinder(axis, radius, height);
	
	/// 여기서부턴display
	return shape;

//	todisplay->SetTransparency(0.8); // 투명도 설정
//	todisplay->SetColor(Quantity_Color(1, 0, 0, Quantity_TOC_RGB)); // 색상 설정
//	CTESTOCCDoc* pDoc = GetDocument();
//	const Handle(AIS_InteractiveContext) aCon = pDoc->myAISContext;
	
//	aCon->Display(todisplay);

}



void CTESTOCCView::drawboundary()
{
	double xstart = MIN_X_COORD;
	double ystart = MIN_Y_COORD;
	double zstart = MIN_Z_COORD;
	double xend = MAX_X_COORD;
	double yend = MAX_Y_COORD;
	double zend = MAX_Z_COORD;

	double delta = GRIDSIZE;

	int max_xindex = (xend - xstart) / delta;
	int max_yindex = (yend - ystart) / delta;
	int max_zindex = (zend - zstart) / delta;
	
	TopoDS_Compound grid;    // 선의 집합이 담기는 Compound
	BRep_Builder builder;
	builder.MakeCompound(grid);

	gp_Pnt P1(xstart, ystart, zstart);
	gp_Pnt P2(xend, ystart, zstart);
	Handle(Geom_TrimmedCurve) Seg = GC_MakeSegment(P1, P2);
	TopoDS_Edge Edge = BRepBuilderAPI_MakeEdge(Seg);
	TopoDS_Shape line = Edge;
	builder.Add(grid, line);

	P1 = gp_Pnt(xstart, ystart, zstart);
	P2 = gp_Pnt(xstart, yend, zstart);
	Seg = GC_MakeSegment(P1, P2);
	Edge = BRepBuilderAPI_MakeEdge(Seg);
	line = Edge;
	builder.Add(grid, line);

	P1 = gp_Pnt(xstart, ystart, zstart);
	P2 = gp_Pnt(xstart, ystart, zend);
	Seg = GC_MakeSegment(P1, P2);
	Edge = BRepBuilderAPI_MakeEdge(Seg);
	line = Edge;
	builder.Add(grid, line);

	P1 = gp_Pnt(xend, ystart, zstart);
	P2 = gp_Pnt(xend, yend, zstart);
	Seg = GC_MakeSegment(P1, P2);
	Edge = BRepBuilderAPI_MakeEdge(Seg);
	line = Edge;
	builder.Add(grid, line);

	P1 = gp_Pnt(xend, ystart, zstart);
	P2 = gp_Pnt(xend, ystart, zend);
	Seg = GC_MakeSegment(P1, P2);
	Edge = BRepBuilderAPI_MakeEdge(Seg);
	line = Edge;
	builder.Add(grid, line);

	P1 = gp_Pnt(xend, ystart, zend);
	P2 = gp_Pnt(xend, yend, zend);
	Seg = GC_MakeSegment(P1, P2);
	Edge = BRepBuilderAPI_MakeEdge(Seg);
	line = Edge;
	builder.Add(grid, line);

	P1 = gp_Pnt(xend, yend, zstart);
	P2 = gp_Pnt(xend, yend, zend);
	Seg = GC_MakeSegment(P1, P2);
	Edge = BRepBuilderAPI_MakeEdge(Seg);
	line = Edge;
	builder.Add(grid, line);

	P1 = gp_Pnt(xstart, ystart, zend);
	P2 = gp_Pnt(xend, ystart, zend);
	Seg = GC_MakeSegment(P1, P2);
	Edge = BRepBuilderAPI_MakeEdge(Seg);
	line = Edge;
	builder.Add(grid, line);

	P1 = gp_Pnt(xstart, ystart, zend);
	P2 = gp_Pnt(xstart, yend, zend);
	Seg = GC_MakeSegment(P1, P2);
	Edge = BRepBuilderAPI_MakeEdge(Seg);
	line = Edge;
	builder.Add(grid, line);

	P1 = gp_Pnt(xstart, yend, zstart);
	P2 = gp_Pnt(xend, yend, zstart);
	Seg = GC_MakeSegment(P1, P2);
	Edge = BRepBuilderAPI_MakeEdge(Seg);
	line = Edge;
	builder.Add(grid, line); P1 = gp_Pnt(xend, ystart, zend);

	P1 = gp_Pnt(xstart, yend, zstart);
	P2 = gp_Pnt(xstart, yend, zend);
	Seg = GC_MakeSegment(P1, P2);
	Edge = BRepBuilderAPI_MakeEdge(Seg);
	line = Edge;
	builder.Add(grid, line); 
	
	P1 = gp_Pnt(xstart, yend, zend);
	P2 = gp_Pnt(xend, yend, zend);
	Seg = GC_MakeSegment(P1, P2);
	Edge = BRepBuilderAPI_MakeEdge(Seg);
	line = Edge;
	builder.Add(grid, line);



	
	TopoDS_Shape resultshape = grid;

	Handle(AIS_InteractiveObject) todisplay = new AIS_Shape(resultshape);
	vis_container.push_back(displayinfo(todisplay, BOUNDARY, 0, -1, 0));




}

BOX CTESTOCCView::CreateOBB(TopoDS_Shape shape)
{
	double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
	double _Xmin, _Ymin, _Zmin, _Xmax, _Ymax, _Zmax;

	///////////////
	Bnd_Box B2;
	BRepBndLib::Add(shape, B2);
	B2.Get(_Xmin, _Ymin, _Zmin, _Xmax, _Ymax, _Zmax);

	Xmin = min(_Xmin, _Xmax);
	Xmax = max(_Xmin, _Xmax);
	Ymin = min(_Ymin, _Ymax);
	Ymax = max(_Ymin, _Ymax);
	Zmin = min(_Zmin, _Zmax);
	Zmax = max(_Zmin, _Zmax);


	double CenterX, CenterY, CenterZ;
	CenterX = (Xmax + Xmin) / 2;
	CenterY = (Ymax + Ymin) / 2;
	CenterZ = (Zmax + Zmin) / 2;


	double Xangle = 0, Yangle = 0;


	TopoDS_Shape _shape;
	_shape = shape;
	double min_value = pow(2, 32);
	gp_Pnt Center = gp_Pnt(CenterX, CenterY, CenterZ);
	


	min_value = pow(2, 32);
	for (double angle = 0; angle < 180; angle += 180 / SENSITIVITY)
	{
		_shape = shape;
		gp_Trsf myTrsf;
		myTrsf.SetRotation(gp_Ax1(Center, gp_Dir(1, 0, 0)), angle * pi / 180);
		BRepBuilderAPI_Transform xform(_shape, myTrsf);
		_shape = xform.Shape();
		Bnd_Box B;
		BRepBndLib::Add(_shape, B);
		double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
		B.Get(Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);
		if (min_value - 0.001> ((Ymax - Ymin) * (Zmax - Zmin)))
		{
			min_value = ((Ymax - Ymin) * (Zmax - Zmin));
			Xangle = angle;
		}
	}
	min_value = pow(2, 32);
	for (double angle = 0; angle < 180; angle += 180 / SENSITIVITY)
	{
		_shape = shape;
		gp_Trsf myTrsf;
		myTrsf.SetRotation(gp_Ax1(Center, gp_Dir(0, 1, 0)), angle * pi / 180);
		BRepBuilderAPI_Transform xform(_shape, myTrsf);
		_shape = xform.Shape();


		Bnd_Box B;
		BRepBndLib::Add(_shape, B);
		double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
		B.Get(Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);
		if (min_value - 0.001 > ((Ymax - Ymin) * (Zmax - Zmin)))
		{
			min_value = ((Ymax - Ymin) * (Zmax - Zmin));
			Yangle = angle;
		}
	}
	
	if (Xangle == 0 || Yangle == 0)
	{
		goto check;						// 회전할 필요가 없으면 바로 처리 가능.
	}

	
	double _angle1, _angle2;
	_angle1 = 180; // (Xangle == 0 ? 0 : 180);
	_angle2 = 180; // (Yangle == 0 ? 0 : 180);
	{
		_shape = shape;
		gp_Trsf myTrsf;
		myTrsf.SetRotation(gp_Ax1(Center, gp_Dir(1, 0, 0)), Xangle *pi / 180);
		BRepBuilderAPI_Transform xform(_shape, myTrsf);
		_shape = xform.Shape();
		myTrsf.SetRotation(gp_Ax1(Center, gp_Dir(0, 1, 0)), Yangle *pi / 180);
		xform = BRepBuilderAPI_Transform(_shape, myTrsf);
		_shape = xform.Shape();

		Bnd_Box B;
		BRepBndLib::Add(_shape, B);
		B.Get(Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);
		min_value = (Zmax - Zmin)*(Ymax - Ymin)*(Xmax - Xmin);
	}


//	printf("Xangle : %lf\nYangle : %lf\n", Xangle, Yangle);

	for (double angle1 = 0; angle1 < _angle1; angle1 += 180 / SENSITIVITY)
	{
		for (double angle2 = 0; angle2 < _angle2; angle2 += 180 / SENSITIVITY)
		{
			_shape = shape;
			gp_Trsf myTrsf;
			myTrsf.SetRotation(gp_Ax1(Center, gp_Dir(1, 0, 0)), angle1 * pi / 180);
			BRepBuilderAPI_Transform xform(_shape, myTrsf);
			_shape = xform.Shape();
			myTrsf.SetRotation(gp_Ax1(Center, gp_Dir(0, 1, 0)), angle2 * pi / 180);
			xform = BRepBuilderAPI_Transform(_shape, myTrsf);
			_shape = xform.Shape();

			Bnd_Box B;
			BRepBndLib::Add(_shape, B);
			double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
			B.Get(Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);
			if (min_value > ((Ymax - Ymin) * (Zmax - Zmin) * (Xmax - Xmin)))
			{
				min_value = ((Ymax - Ymin) * (Zmax - Zmin) * (Xmax - Xmin));
				Xangle = angle1;
				Yangle = angle2;
			}
		}
	}
	

	check:   /// goto 구문으로 이동하는 곳

	printf("Xangle : %lf\nYangle : %lf\n", Xangle, Yangle);
	_shape = shape;
	gp_Trsf myTrsf;
	myTrsf.SetRotation(gp_Ax1(Center, gp_Dir(1, 0, 0)), Xangle *pi / 180);
	BRepBuilderAPI_Transform xform(_shape, myTrsf);
	_shape = xform.Shape();
	myTrsf.SetRotation(gp_Ax1(Center, gp_Dir(0, 1, 0)), Yangle *pi / 180);
	xform = BRepBuilderAPI_Transform(_shape, myTrsf);
	_shape = xform.Shape();

	Bnd_Box B;
	BRepBndLib::Add(_shape, B);
	B.Get(Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);
	gp_Pnt minp = gp_Pnt(Xmin, Ymin, Zmin);
	gp_Pnt maxp = gp_Pnt(Xmax, Ymax, Zmax);

	BRepPrimAPI_MakeBox box = BRepPrimAPI_MakeBox(minp, maxp);
	TopoDS_Shape sh_box = box.Shape();


	gp_Trsf myTrsf2;

	myTrsf2.SetRotation(gp_Ax1(Center, gp_Dir(0, 1, 0)), -Yangle *pi / 180.0); 
	BRepBuilderAPI_Transform xform2 = BRepBuilderAPI_Transform(sh_box, myTrsf2);
	sh_box = xform2.Shape();
	myTrsf2.SetRotation(gp_Ax1(Center, gp_Dir(1, 0, 0)), -Xangle *pi / 180.0);
	xform2 = BRepBuilderAPI_Transform(sh_box, myTrsf2);
	sh_box = xform2.Shape();

	BOX boxstruct = BOX(sh_box, Xangle, Yangle, Xmin, Xmax, Ymin, Ymax, Zmin, Zmax);

	return boxstruct;
}

bool flag = 0;

point_d CTESTOCCView::GetAbsCoord(TopoDS_Shape shape)
{
	///좌표 테스트 (찍은곳의 좌표 얻는 법)
	GProp_GProps System;
	BRepGProp::VolumeProperties(shape, System);
	gp_Pnt pt = System.CentreOfMass();

//	printf("test -> x_coord:%lf, y_coord:%lf, z_coord:%lf\n", pt.X(), pt.Y(), pt.Z());

	///
	point_d ptd;
	ptd.x = pt.X();
	ptd.y = pt.Y();
	ptd.z = pt.Z();

	return ptd;
}


int ax = 0;
int ay = 0;

void CTESTOCCView::OnLButtonDown(UINT nFlags, CPoint point)
{
	ax = point.x;
	ay = point.y;

	CTESTOCCDoc* pDoc = GetDocument();





	const Handle(AIS_InteractiveContext) aCon = pDoc->myAISContext;
	
	if (isCtrlPressed())
	{
		if (!pipeselection)
		{
	//		pDoc->End_Selection();
			pDoc->Selection(SELECT_PART);
			aCon->SetAutoActivateSelection(Standard_False);
			aCon->MoveTo(point.x, point.y, myView);
			aCon->Select(false);  // 이게 필요했다!!

	//		aCon->UnhilightSelected();
		

			aCon->InitSelected();

			if (aCon->HasSelectedShape())
			{
				pDoc->Picked_Interactive = aCon->SelectedInteractive();
				pDoc->Picked = true;
			}
			else
				pDoc->Picked = false;


			/// select object로 부터 정보를 얻는 붑
			for (aCon->InitSelected(); aCon->MoreSelected(); aCon->NextSelected())
			{
				bool dobreak = 0;
				TopoDS_Shape selected_shape;
				selected_shape = aCon->SelectedShape();
				Handle(AIS_InteractiveObject) obj = aCon->SelectedInteractive();
				for (int i = 0; i < vis_container.size(); i++)
				{
					if (vis_container[i].obj == obj && vis_container[i].type == BND_BOX)
					{
						extern PartListDialog* pDlg; // 다이얼로그 포인터 가져오기
						if (pDlg)
						{
							CString string;
							string.Format(_T("BNDBOX-%03d"), vis_container[i].boxindex);
							int indx = pDlg->List_bndbox.FindString(0, string);
							if (indx != -1)
							{
								pDlg->List_bndbox.SetCurSel(indx);
								pDlg->OnLbnSelchangeListBndbox();
							}
						}
						dobreak = 1;
						break;

					}
					else if (vis_container[i].obj == obj && vis_container[i].type == PIPE_IO)
					{
						extern PartListDialog* pDlg; // 다이얼로그 포인터 가져오기
						if (pDlg)
						{
							CString string;
							string.Format(_T("PIPE-%03d"), vis_container[i].pipeindex);
							int indx = pDlg->List_bndbox.FindString(0, string);
							if (indx != -1)
							{
								pDlg->List_bndbox.SetCurSel(indx);
								pDlg->OnLbnSelchangeListBndbox();
							}
						}
						dobreak = 1;
						break;
					}
				}
				if (dobreak)
					break;


//////////////////////////////////////////////////
				/// Selection 처리 ///////////
				CString selected_name;
				for (int k = 0; k < pDoc->num_of_file; k++)
				{
					TDF_Label aLabel = pDoc->myAssembly[k]->FindShape(selected_shape);
					if (aLabel.IsNull())
						continue;
					bool dobreak = 0;
					for (int i = 1; i <= pDoc->num_of_file; i++)
					{
						for (int j = 1; j <= pDoc->frshapes[i].Length(); j++)
						{
							if (pDoc->frshapes[i](j) == aLabel)
							{
								aLabel = pDoc->frshapes[i](j);
								dobreak = 1;
								break;
							}
						}
						if (dobreak)
							break;
					}


					/// 이름 받기를 위한 임시처리
					/// get name 
					Handle(TDataStd_Name) N;
					if (!aLabel.FindAttribute(TDataStd_Name::GetID(), N)) {
						// no name is attached

					}
					TCollection_ExtendedString name = N->Get();
					cout << name << "   test" << endl;
					selected_name = (wchar_t*)name.ToExtString();
					/// 이걸로 받을 수 있을듯.

					if (aLabel.IsNull())
						cout << "NULL" << endl;
					extern PartListDialog* pDlg; // 다이얼로그 포인터 가져오기
					if (pDlg)
						pDlg->renewal(selected_name);

				}

			}
			
		}
		else // pipe selection 상태이면
		{
			static bool hasselected = false;   // 첫번째 찍힌게 있나?
			static Handle(AIS_InteractiveObject) pobj = NULL;
			static int pindex = -1;


			pDoc->Selection(SELECT_PART);
			aCon->SetAutoActivateSelection(Standard_False);
			aCon->MoveTo(point.x, point.y, myView);
			aCon->Select(point.x - 15, point.y - 15, point.x + 15, point.y + 15, myView, false);  // 이게 필요했다!!
			aCon->UnhilightSelected();
			aCon->InitSelected();

			if (!aCon->MoreSelected())
			{
				printf("unselect\n");
				hasselected = false;
			}


			for (aCon->InitSelected(); aCon->MoreSelected(); aCon->NextSelected())
			{
				/// test
				Handle(AIS_InteractiveObject) obj = aCon->SelectedInteractive();


				vector<displayinfo>::iterator it;
				it = find(vis_container.begin(), vis_container.end(), obj);
				if (it != vis_container.end()) // obj내에 있는 것들
				{
					if (it->type == PIPE_IO)
					{
						printf("found!!!!\n");
						printf("boxindex : %d\n", it->boxindex);
						aCon->HilightSelected();


						/// 선택되지 않은 상태일때
						if (!hasselected)
						{
							for (int k = 0; k < pipes.size(); k++)
							{
								if (obj == pipes[k].obj)  // interactiveobject가 선택된것이 있는지 pipes내에서 찾음.
								{
									if (!pipes[k].isusing)   // 사용중이 아니어야함
									{
										pobj = obj;
										pindex = k;
										hasselected = true;

									}
									else
									{
										printf("isusing...\n");
									}
								}
							}
						}
						/// 이미 하나가 선택된 상태일때
						else
						{
							for (int k = 0; k < pipes.size(); k++)    // 이전 index와 k를 이으면 된다.
							{
								if (obj == pipes[k].obj)  // interactiveobject가 선택된것이 있는지 pipes내에서 찾음.
								{
									if (pindex != k)
									{

										if (draw_route(pindex, k) == true)
										{
											pipes[pindex].isusing = 1;
											pipes[k].isusing = 1;
											pipe_connection.push_back(connection_info(pindex, k));  // 파이프 연결 정보 등록
											extern PartListDialog* pDlg; // 다이얼로그 리스트박스에 정보 넣기
											int mn = min(pindex, k);
											int mx = max(pindex, k);

											if (pDlg)
												pDlg->InsertconnectionList(mn, mx);
											hasselected = false;
											break;
										}
										
									}
									else
										printf("동일한 두 파이프가 선택됨\n");
									hasselected = false;
								}
							}
						}
						break;
					}
				}
				else
				{
					hasselected = false;
				}
			}
		
		}
		pDoc->ReDisplay();
	}

	else if (isShiftPressed())
	{
		if (!pipeselection)
		{
			bool dobreak = 0;

			pDoc->Selection(SELECT_ASSEMBLY);
			aCon->SetAutoActivateSelection(Standard_False);
			aCon->MoveTo(point.x, point.y, myView);
			aCon->Select(false);  // 이게 필요했다!!
			aCon->InitSelected();
			/// select object로 부터 정보를 얻는 붑
			for (aCon->InitSelected(); aCon->MoreSelected(); aCon->NextSelected())
			{
				Handle(AIS_InteractiveObject) obj = aCon->SelectedInteractive();
				for (int i = 0; i < vis_container.size(); i++)
				{
					if (vis_container[i].obj == obj && vis_container[i].type == ROUTED_PIPE)
					{
						int mn = min(vis_container[i].dest, vis_container[i].src);
						extern PartListDialog* pDlg; // 다이얼로그 포인터 가져오기
						if (pDlg)
						{
							CString string;
							string.Format(_T("Pipe%03d"), mn);
							int indx = pDlg->List_Pipeconnection.FindString(0, string);
							pDlg->List_Pipeconnection.SetCurSel(indx);
							pDlg->OnLbnSelchangeListPipeConnection();
						}
						dobreak = 1;
						break;
							
					}
				}
				if (dobreak)
					break;

			}

		}
	}

	///
	CView::OnLButtonDown(nFlags, point);
}


void CTESTOCCView::OnMButtonDown(UINT nFlags, CPoint point)
{
	CTESTOCCDoc* pDoc = GetDocument();
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	ddx = point.x;
	ddy = point.y;
	CView::OnMButtonDown(nFlags, point);
}


BOOL CTESTOCCView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	myView->Zoom(0, 0, -zDelta / 20, 0);
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}




void CTESTOCCView::OnMouseMove(UINT nFlags, CPoint point)
{
	
	CTESTOCCDoc* pDoc = GetDocument();
	const Handle(AIS_InteractiveContext) aCon = pDoc->myAISContext;

	static bool onces = true;
	if (onces)
	{
		ax = point.x;
		ax = point.y;
		onces = false;
	}


	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	int dx, dy;
	
	if (nFlags & MK_LBUTTON)
	{

			/*
			Quantity_Factor fac = myView->Scale();

			gp_Trsf myTrsf;
			myTrsf.SetTranslation(gp_Vec((X_Coord - point.x) / 5.0, -(X_Coord - point.x) / 5.0, (Y_Coord - point.y) / 5.0));
			aCon->SetLocation(pDoc->Picked_Interactive, TopLoc_Location(myTrsf));
			aCon->Redisplay(pDoc->Picked_Interactive);
			
			AIS_ListOfInteractive aList;      // 이거 이용하면 어셈블리로부터 파트 선택이 가능할것같은데?
			aCon->DisplayedObjects(aList);
			AIS_ListIteratorOfListOfInteractive aListIterator;
			for (aListIterator.Initialize(aList); aListIterator.More(); aListIterator.Next())
			{
				if (pDoc->Picked_Interactive == aListIterator.Value())
					continue;
				Handle(AIS_Shape) ais = Handle(AIS_Shape)::DownCast(aListIterator.Value()); /// 다운캐스팅.. AIS_Object -> AIS_Shape -> TopoDS_Shape로 가는 과정
				TopoDS_Shape CurShp = ais->Shape();
				Handle(AIS_Shape) ais2 = Handle(AIS_Shape)::DownCast(pDoc->Picked_Interactive); /// 다운캐스팅.. AIS_Object -> AIS_Shape -> TopoDS_Shape로 가는 과정
				TopoDS_Shape RefShp = ais2->Shape();
			}
			*/

		myView->Rotate((point.x-ax)*(0.01), -(point.y-ay)*(0.01), 0);
		//myView->Rotation(dx, dy);
		ax = point.x;
		ay = point.y;

		
		
		
	}
	else if (nFlags & MK_MBUTTON)
	{
		Quantity_Factor fac = myView->Scale();
		myView->Translate((point.x - ddx)/fac, (ddy - point.y)/fac, 0);
		ddx = point.x;
		ddy = point.y;
	}

	CView::OnMouseMove(nFlags, point);
}


void CTESTOCCView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CView::OnLButtonUp(nFlags, point);
}


void CTESTOCCView::OnRButtonDown(UINT nFlags, CPoint point)
{

	CView::OnRButtonDown(nFlags, point);
}


void CTESTOCCView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CTESTOCCDoc* pDoc = GetDocument();
	const Handle(AIS_InteractiveContext) aCon = pDoc->myAISContext;

	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (pDoc->Picked)
	{
		int index = 0;
		int boxindex = 0;
		for (int i = 0; i < vis_container.size(); i++)
		{
			if (vis_container[i].obj == pDoc->Picked_Interactive)
			{
				index = i;
				boxindex = vis_container[i].boxindex;
			}
		}
		switch (nChar)
		{
		case 'A':   /// box index가 같은 것들은 모두 따라가야함.
			pDoc->ReDisplay(true);
			for (int i = 0; i < vis_container.size(); i++)
			{
				if (vis_container[i].boxindex == boxindex)
				{

					//aCon->SetLocation(pDoc->Picked_Interactive, TopLoc_Location(myTrsf));
					gp_Trsf trs;
					gp_XYZ xyz = vis_container[i].trsf.TranslationPart();  // 기존의 translation을 얻ㅇ므
					xyz.SetX(xyz.X() - TRANSLATIONSCALE);
					trs.SetTranslation(gp_Vec(xyz.X(), xyz.Y(), xyz.Z()));
					vis_container[i].trsf = trs;
					vis_container[i].obj->SetLocalTransformation(trs);
					aCon->Redisplay(vis_container[i].obj);

					if (vis_container[i].type == PIPE_IO)
					{
						int m = vis_container[i].pipeindex;   // pipeindex를 얻음
						pipes[m].x += -TRANSLATIONSCALE;
					}
				}

			}
			break;
		case 'D':
			pDoc->ReDisplay(true);
			for (int i = 0; i < vis_container.size(); i++)
			{
				if (vis_container[i].boxindex == boxindex)
				{

					//aCon->SetLocation(pDoc->Picked_Interactive, TopLoc_Location(myTrsf));
					gp_Trsf trs;
					gp_XYZ xyz = vis_container[i].trsf.TranslationPart();  // 기존의 translation을 얻ㅇ므
					xyz.SetX(xyz.X() + TRANSLATIONSCALE);
					trs.SetTranslation(gp_Vec(xyz.X(), xyz.Y(), xyz.Z()));
					vis_container[i].trsf = trs;
					vis_container[i].obj->SetLocalTransformation(trs);
					aCon->Redisplay(vis_container[i].obj);
					if (vis_container[i].type == PIPE_IO)
					{
						int m = vis_container[i].pipeindex;   // pipeindex를 얻음
						pipes[m].x += TRANSLATIONSCALE;
					}
				}

			}
			break;
		case 'W':
			pDoc->ReDisplay(true);
			for (int i = 0; i < vis_container.size(); i++)
			{
				if (vis_container[i].boxindex == boxindex)
				{

					//aCon->SetLocation(pDoc->Picked_Interactive, TopLoc_Location(myTrsf));
					gp_Trsf trs;
					gp_XYZ xyz = vis_container[i].trsf.TranslationPart();  // 기존의 translation을 얻ㅇ므
					xyz.SetY(xyz.Y() + TRANSLATIONSCALE);
					trs.SetTranslation(gp_Vec(xyz.X(), xyz.Y(), xyz.Z()));
					vis_container[i].trsf = trs;
					vis_container[i].obj->SetLocalTransformation(trs);
					aCon->Redisplay(vis_container[i].obj);
					if (vis_container[i].type == PIPE_IO)
					{
						int m = vis_container[i].pipeindex;   // pipeindex를 얻음
						pipes[m].y += TRANSLATIONSCALE;
					}
				}

			}
			break;
		case 'S':
			pDoc->ReDisplay(true);
			for (int i = 0; i < vis_container.size(); i++)
			{
				if (vis_container[i].boxindex == boxindex)
				{

					//aCon->SetLocation(pDoc->Picked_Interactive, TopLoc_Location(myTrsf));
					gp_Trsf trs;
					gp_XYZ xyz = vis_container[i].trsf.TranslationPart();  // 기존의 translation을 얻ㅇ므
					xyz.SetY(xyz.Y() - TRANSLATIONSCALE);
					trs.SetTranslation(gp_Vec(xyz.X(), xyz.Y(), xyz.Z()));
					vis_container[i].trsf = trs;
					vis_container[i].obj->SetLocalTransformation(trs);
					aCon->Redisplay(vis_container[i].obj);

					if (vis_container[i].type == PIPE_IO)
					{
						int m = vis_container[i].pipeindex;   // pipeindex를 얻음
						pipes[m].y += -TRANSLATIONSCALE;
					}
				}

			}
			break;
		case 'Q':
			pDoc->ReDisplay(true);
			for (int i = 0; i < vis_container.size(); i++)
			{
				if (vis_container[i].boxindex == boxindex)
				{

					//aCon->SetLocation(pDoc->Picked_Interactive, TopLoc_Location(myTrsf));
					gp_Trsf trs;
					gp_XYZ xyz = vis_container[i].trsf.TranslationPart();  // 기존의 translation을 얻ㅇ므
					xyz.SetZ(xyz.Z() - TRANSLATIONSCALE);
					trs.SetTranslation(gp_Vec(xyz.X(), xyz.Y(), xyz.Z()));
					vis_container[i].trsf = trs;
					vis_container[i].obj->SetLocalTransformation(trs);
					aCon->Redisplay(vis_container[i].obj);
					if (vis_container[i].type == PIPE_IO)
					{
						int m = vis_container[i].pipeindex;   // pipeindex를 얻음
						pipes[m].z += -TRANSLATIONSCALE;
					}
				}

			}
			break;
		case 'E':
			pDoc->ReDisplay(true);
			for (int i = 0; i < vis_container.size(); i++)
			{
				if (vis_container[i].boxindex == boxindex)
				{

					//aCon->SetLocation(pDoc->Picked_Interactive, TopLoc_Location(myTrsf));
					gp_Trsf trs;
					gp_XYZ xyz = vis_container[i].trsf.TranslationPart();  // 기존의 translation을 얻ㅇ므
					xyz.SetZ(xyz.Z() + TRANSLATIONSCALE);
					trs.SetTranslation(gp_Vec(xyz.X(), xyz.Y(), xyz.Z()));
					vis_container[i].trsf = trs;
					vis_container[i].obj->SetLocalTransformation(trs);
					aCon->Redisplay(vis_container[i].obj);
					if (vis_container[i].type == PIPE_IO)
					{
						int m = vis_container[i].pipeindex;   // pipeindex를 얻음
						pipes[m].z += TRANSLATIONSCALE;
					}
				}

			}
			break;
		default:
			break;
		}
	}

//	pDoc->ReDisplay();
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
