
// TESTOCCView.h : CTESTOCCView 클래스의 인터페이스
//

#pragma once
#include "grid.h"


class CTESTOCCView : public CView
{
	///
public:
	BOX CreateOBB(TopoDS_Shape shape);


public:
	Handle_V3d_View myView;
	Standard_Boolean myHlrModeIsOn;
	void OnInitialUpdate();


protected: // serialization에서만 만들어집니다.
	CTESTOCCView();
	DECLARE_DYNCREATE(CTESTOCCView)

// 특성입니다.
public:
	CTESTOCCDoc* GetDocument() const;

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 구현입니다.
public:
	virtual ~CTESTOCCView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);


	void drawboundary();
	TopoDS_Shape getcylinder(double x1, double x2, double y1, double y2, double z1, double z2, double radius);
	void Collect_BB();
	bool Collision_box_and_grid(int x, int y, int z);
	bool Collision_box_and_triangle(TopoDS_Shape triangle);
	int Collision_box_and_coord_without_pipe(gp_Pnt pnt);  // 리턴값은 boxindex
	point_d GetAbsCoord(TopoDS_Shape shape);
	void displaybox(double x, double y, double z);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // TESTOCCView.cpp의 디버그 버전
inline CTESTOCCDoc* CTESTOCCView::GetDocument() const
   { return reinterpret_cast<CTESTOCCDoc*>(m_pDocument); }
#endif
