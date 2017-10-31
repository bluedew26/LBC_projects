
// TESTOCCDoc.cpp : CTESTOCCDoc 클래스의 구현
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "TESTOCC.h"
#endif

#include "TESTOCCDoc.h"
#include "PartListDialog.h"
#include <propkey.h>
#include "grid.h"
#include "TESTOCCView.h"
#include "PartListDialog.h"
#include "Mainfrm.h"
#include "ChildFrm.h"
#include "config.h"
#include "Astar.h"



// CTESTOCCDoc
IMPLEMENT_DYNCREATE(CTESTOCCDoc, CDocument)
BEGIN_MESSAGE_MAP(CTESTOCCDoc, CDocument)
	ON_COMMAND(ID_IMPORT_IMPORT32774, &CTESTOCCDoc::OnImportImport32774)
END_MESSAGE_MAP()
// CTESTOCCDoc 생성/소멸
CTESTOCCDoc::~CTESTOCCDoc()
{
}
BOOL CTESTOCCDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}
// CTESTOCCDoc serialization
void CTESTOCCDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 여기에 저장 코드를 추가합니다.
	}
	else
	{
		// TODO: 여기에 로딩 코드를 추가합니다.
	}
}
#ifdef SHARED_HANDLERS

// 축소판 그림을 지원합니다.
void CTESTOCCDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 문서의 데이터를 그리려면 이 코드를 수정하십시오.
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 검색 처리기를 지원합니다.
void CTESTOCCDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 문서의 데이터에서 검색 콘텐츠를 설정합니다.
	// 콘텐츠 부분은 ";"로 구분되어야 합니다.

	// 예: strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CTESTOCCDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS
// CTESTOCCDoc 진단
#ifdef _DEBUG
void CTESTOCCDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTESTOCCDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG



///


void Init()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CTESTOCCView* pView = (CTESTOCCView*)pChild->GetActiveView();

	pView->drawboundary();

}


CTESTOCCDoc::CTESTOCCDoc()
{
	Handle(Graphic3d_GraphicDriver) aGraphicDriver = ((CTESTOCCApp*)AfxGetApp())->GetGraphicDriver();

	TCollection_ExtendedString a3DName("Visu3D");
	myViewer = new V3d_Viewer(aGraphicDriver, a3DName.ToExtString());
//	myViewer = new V3d_Viewer(_T("awrq"));

	myViewer->SetDefaultLights();
	myViewer->SetLightOn();
	//myViewer->SetDefaultBackgroundColor(Quantity_NOC_BLUE1);//改变背景颜色

	myAISContext = new AIS_InteractiveContext(myViewer);  //创建一个交互文档
	myAISContext->DefaultDrawer()->UIsoAspect()->SetNumber(11);
	myAISContext->DefaultDrawer()->VIsoAspect()->SetNumber(11);

	//这里设置实体的显示模式
	myAISContext->SetDisplayMode(AIS_Shaded, Standard_False);

	//////////////////////////////////////
	Handle(XCAFApp_Application) anApp = XCAFApp_Application::GetApplication();

	for (int i = 0; i < NUM_OF_IMPORT; i++)
	{
		anApp->NewDocument("MDTV-XCAF", aDoc[i]);
	}

	/// selection ///
	Init();
	Display();
//	Selection(SELECT_PART);

	/// Dialog 띄우기 ///
	extern PartListDialog* pDlg;
	pDlg = new PartListDialog;
	pDlg->Create(IDD_PARTLIST);
	pDlg->ShowWindow(SW_SHOW);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	pDlg->Init();
}



////////////////////////////////////////////////

void CTESTOCCDoc::End_Selection()
{
//	myAISContext->CloseLocalContext();
}

void CTESTOCCDoc::Selection(int sel_mode)
{

	myAISContext->CloseLocalContext();    // 지금 버퍼를 닫고 새 버퍼를 연다?
	myAISContext->OpenLocalContext();
//
	static int previous_sel_mode = sel_mode;

	switch (sel_mode)
	{
	case SELECT_PART:
		myAISContext->ActivateStandardMode(TopAbs_SOLID);   /// Solid로 선택하면 각 파트별로 구분할 수 있다.
		break;
	case SELECT_ASSEMBLY:
		myAISContext->ActivateStandardMode(TopAbs_COMPOUND);
		break;
	}
	/*
	AIS_ListOfInteractive objects;
	myAISContext->DisplayedObjects(objects); // 이미 출력한 오브젝트들을 표시?
	AIS_ListIteratorOfListOfInteractive iobject(objects);
	while (iobject.More())
	{
		myAISContext->Deactivate(iobject.Value(), previous_sel_mode);
		myAISContext->Activate(iobject.Value(), sel_mode);
		iobject.Next();
	}
	previous_sel_mode = sel_mode;
	*/
}



void CTESTOCCDoc::Import(const char* filename)
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CTESTOCCView* pView = (CTESTOCCView*)pChild->GetActiveView();



	static int index = 0;
	num_of_file++;

	STEPCAFControl_Reader reader;
	///
	reader.SetColorMode(true);
	reader.SetNameMode(true);
	reader.SetLayerMode(true);
	///
	reader.ReadFile(filename);
	reader.Transfer(aDoc[index]);



	myAssembly[index] = XCAFDoc_DocumentTool::ShapeTool(aDoc[index]->Main()); // myAssembly는 라벨과 같다.
	myColors[index] = XCAFDoc_DocumentTool::ColorTool(aDoc[index]->Main());
	myAssembly[index]->GetShapes(frshapes[index]); // 모든 Part Shape를 받아옴




	int k = index;
	
	/// 이것들은 나중에 수정
	numofassembly[k] = 0;
	numofcomponent[k] = 0;
	numofpart[k] = 0;
	for (int i = 1; i <= frshapes[k].Length(); i++)
	{
		if (myAssembly[k]->IsComponent(frshapes[k].Value(i)))
			numofcomponent[k]++;
		else if (myAssembly[k]->IsAssembly(frshapes[k].Value(i)))
			numofassembly[k]++;
		else
			numofpart[k]++;
	}
//	printf("a : %d, b : %d, c : %d\n", numofcomponent[k], numofassembly[k], numofpart[k]);

	Parts[k] = new TDF_Label[numofpart[k]];
	Assemblys[k] = new TDF_Label[numofassembly[k]];
	Components[k] = new TDF_Label[numofcomponent[k]];

	int partindex = 0, compindex = 0, assmindex = 0;
	for (int i = 1; i <= frshapes[k].Length(); i++)
	{
		if (myAssembly[k]->IsComponent(frshapes[k].Value(i)))
		{
			Components[k][compindex] = frshapes[k](i);
			compindex++;
		}
		else if (myAssembly[k]->IsAssembly(frshapes[k].Value(i)))
		{
			Assemblys[k][assmindex] = frshapes[k](i);
			assmindex++;
		}
		else
		{
			Parts[k][partindex] = frshapes[k](i);
			partindex++;
		}
	}



	TopoDS_Shape Shape = myAssembly[k]->GetShape(frshapes[k](1));

	Handle(AIS_InteractiveObject) obj = new AIS_Shape(Shape);

	
	/// bounding box 얻기
	DisplayOnly(obj);
	Selection(SELECT_PART);
	myAISContext->Select(-20000, -20000, 20000, 20000, pView->myView);
	myAISContext->SetAutoActivateSelection(Standard_False);
	myAISContext->UnhilightSelected();

	vector<displayinfo> temp; // 파이프IO정보만을 임시로 담는 컨테이너
	gp_Pnt aPnt1, aPnt2;

	int boxindex = 0;
	int modelnum = 0;
	double radius;
	point_d centofpipe;
	for (myAISContext->InitSelected(); myAISContext->MoreSelected(); myAISContext->NextSelected())
	{
		bool ispipe = false;
		TopoDS_Shape ashape = myAISContext->SelectedShape();

		Handle(AIS_InteractiveObject) obj3 = new AIS_Shape(ashape);
		vis_container.push_back(displayinfo(obj3, MODEL, k, boxindex, modelnum));



		/// pipe인지 그냥 박스인지 판명하는 부분
		for (TopExp_Explorer ex(ashape, TopAbs_FACE); ex.More(); ex.Next())
		{

			TopoDS_Face currentFace = TopoDS::Face(ex.Current());
			BRepAdaptor_Surface surf(currentFace, Standard_True);
			if (surf.GetType() == GeomAbs_Cylinder)			// 실린더로 판명되면
			{
				gp_Cylinder Cylinder = surf.Cylinder();
				gp_Ax1 theAxis = Cylinder.Axis();
				gp_Dir aDirAxis = theAxis.Direction();

				gp_Lin theAxisLine(theAxis);
				aPnt1 = surf.Value(surf.FirstUParameter(), surf.FirstVParameter());
				aPnt2 = surf.Value(surf.LastUParameter(), surf.LastVParameter());

				Handle(Geom_Line) aGeomAxis = new Geom_Line(theAxis);
				GeomAPI_ProjectPointOnCurve aProj1(aPnt1, aGeomAxis);
				GeomAPI_ProjectPointOnCurve aProj2(aPnt2, aGeomAxis);

				aPnt1 = aProj1.Point(1);
				aPnt2 = aProj2.Point(1);
				double height = aProj1.Point(1).Distance(aProj2.Point(1));

				if (height >= (0.99)*PIPE_IO_LENGTH && height <= (1.01)*PIPE_IO_LENGTH) // 파이프 IO 발견!
				{
					ispipe = true;
					radius = Cylinder.Radius();
					centofpipe = pView->GetAbsCoord(ashape);
					// 반지름 정보 등을 구한뒤 컨테이너에 넣음.
					int cnt = 0;
					/*
					for (TopExp_Explorer ex2(ashape, TopAbs_EDGE); ex2.More(); ex2.Next())
					{
						TopoDS_Edge currentEdge = TopoDS::Edge(ex2.Current());
						BRepAdaptor_Curve curve(currentEdge);
						
						if (curve.GetType() == GeomAbs_Circle)
						{
							gp_Circ circ = curve.Circle();
							gp_Ax1 _ax = circ.Axis();
							Handle(Geom_Line) _axis = new Geom_Line(_ax);
							GeomAPI_ProjectPointOnCurve aProj1(aPnt1, _axis);

							printf("cnt = %d\n", cnt);

							if (cnt == 0)
								aPnt1 = gp_Pnt(circ.Location().X() + _ax.Direction().X()*PIPE_IO_LENGTH, circ.Location().Y() + _ax.Direction().Y()*PIPE_IO_LENGTH, circ.Location().Z() + _ax.Direction().Z()*PIPE_IO_LENGTH);
							if (cnt == 1)
								aPnt2 = gp_Pnt(circ.Location().X() - _ax.Direction().X()*PIPE_IO_LENGTH, circ.Location().Y() - _ax.Direction().Y()*PIPE_IO_LENGTH, circ.Location().Z() - _ax.Direction().Z()*PIPE_IO_LENGTH);
							
							
							cnt++;

					
							
					//		centofpipe = pView->GetAbsCoord(ashape);
					//		aPnt1.SetXYZ(gp_XYZ(centofpipe.x + _ax.Direction().X() * height / 2, centofpipe.y + _ax.Direction().Y() * height / 2, centofpipe.z + _ax.Direction().Z() * height / 2));
					//		aPnt2.SetXYZ(gp_XYZ(centofpipe.x - _ax.Direction().X() * height / 2, centofpipe.y - _ax.Direction().Y() * height / 2, centofpipe.z - _ax.Direction().Z() * height / 2));
						}
					}
					*/
					break;
				}
			}


		}
		BOX B = pView->CreateOBB(ashape);


		Handle(AIS_InteractiveObject) obj2 = new AIS_Shape(B.shape);
		displayinfo info;
		info.obj = obj2;
		info.fileindex = k;
		if (ispipe)
		{
			info.type = PIPE_IO;
			info.P1 = aPnt1;
			info.P2 = aPnt2;
			info.radius = radius;
//			info.center = centofpipe;
			info.modelnum = modelnum;
			B.isPipeIO = 1;
			info.obj->SetSelectionPriority(10);
			
		}
		else
		{
			info.type = BND_BOX;
			info.boxindex = boxindex;
			B.isPipeIO = 0;
			B.boxindex = boxindex;
			boxindex++;
			info.obj->SetSelectionPriority(-1);
		}
		boxes.push_back(B);						// 박스 정보 받음.
		vis_container.push_back(info);
		modelnum++;
	}

	for (int i = 0; i < vis_container.size(); i++)
	{
		if (vis_container[i].type == PIPE_IO)
		{
			gp_Pnt _P1, _P2;
			_P1 = vis_container[i].P1;
			_P2 = vis_container[i].P2;

			int _boxindex1 = pView->Collision_box_and_coord_without_pipe(_P1);
			int _boxindex2 = pView->Collision_box_and_coord_without_pipe(_P2);
			gp_Dir P1_dir = gp_Dir(gp_XYZ(_P1.X() - _P2.X(), _P1.Y() - _P2.Y(), _P1.Z() - _P2.Z()));
			gp_Dir P2_dir = gp_Dir(gp_XYZ(_P2.X() - _P1.X(), _P2.Y() - _P1.Y(), _P2.Z() - _P1.Z()));

			if (_boxindex1 != -1 && _boxindex2 != -1) // 둘다 bndbox에 겹쳐버리면
			{
				gp_Pnt ap1, ap2;
				int j = 0;
				while (1)
				{
					j++;
					ap1.SetX(_P1.X() + j*GRIDSIZE*P1_dir.X());
					ap1.SetY(_P1.Y() + j*GRIDSIZE*P1_dir.Y());
					ap1.SetZ(_P1.Z() + j*GRIDSIZE*P1_dir.Z());
					ap2.SetX(_P2.X() + j*GRIDSIZE*P2_dir.X());
					ap2.SetY(_P2.Y() + j*GRIDSIZE*P2_dir.Y());
					ap2.SetZ(_P2.Z() + j*GRIDSIZE*P2_dir.Z());
					_boxindex1 = pView->Collision_box_and_coord_without_pipe(ap1);
					_boxindex2 = pView->Collision_box_and_coord_without_pipe(ap2);
					if (_boxindex1 != -1 && _boxindex2 != -1)
						continue;
					else
						break;
				}
			}

			if (_boxindex1 != -1)   // P1과 박스들중에 겹치는게 있으면 (1번만 겹치면)
			{
				gp_Dir dir;

				dir.SetXYZ(gp_XYZ(_P2.X() - _P1.X(), _P2.Y() - _P1.Y(), _P2.Z() - _P1.Z()));
	//			point_d cent = vis_container[i].center;
	//			gp_Pnt mainPnt = gp_Pnt(cent.x + dir.X() * PIPE_IO_LENGTH, cent.y + dir.Y() * PIPE_IO_LENGTH, cent.z + dir.Z() * PIPE_IO_LENGTH);
	//			vis_container[i].MainPnt = mainPnt;

	//			printf("x : %lf y : %lf z : %lf \n", dir.X(), dir.Y(), dir.Z());
				
				vis_container[i].boxindex = _boxindex1;

				for (int m = 0; m < vis_container.size(); m++)   /// model의 boxnum도 맞춰줌.
				{
					if (vis_container[m].type == MODEL && vis_container[m].modelnum == vis_container[i].modelnum)
					{
						vis_container[m].boxindex = _boxindex1;
					}
				}

				vis_container[i].normal = dir;
				vis_container[i].pipeindex = pipes.size();
				pipes.push_back(Pipe_Info(vis_container[i].obj, vis_container[i].fileindex, _P2.X(), _P2.Y(), _P2.Z(), dir, radius));
			}
			else if (_boxindex2 != -1)
			{

				gp_Dir dir;
				
				dir.SetXYZ(gp_XYZ(_P1.X() - _P2.X(), _P1.Y() - _P2.Y(), _P1.Z() - _P2.Z()));
				vis_container[i].boxindex = _boxindex2;
	//			point_d cent = vis_container[i].center;
	//			gp_Pnt mainPnt = gp_Pnt(cent.x + dir.X() * PIPE_IO_LENGTH, cent.y + dir.Y() * PIPE_IO_LENGTH, cent.z + dir.Z() * PIPE_IO_LENGTH);
	//			vis_container[i].MainPnt = mainPnt;
				for (int m = 0; m < vis_container.size(); m++)   /// model의 boxnum도 맞춰줌.
				{
					if (vis_container[m].type == MODEL && vis_container[m].modelnum == vis_container[i].modelnum)
					{
						vis_container[m].boxindex = _boxindex2;
					}
				}

				vis_container[i].normal = dir;
				vis_container[i].pipeindex = pipes.size();
				pipes.push_back(Pipe_Info(vis_container[i].obj, vis_container[i].fileindex, _P1.X(), _P1.Y(), _P1.Z(), dir, radius));
			}
		}
	}

	extern PartListDialog* pDlg;
	pDlg->GetModelTree();
	pDlg->InsertbndboxList();
//	gp_Trsf myTrsf;
//	myTrsf.SetTranslation(transf[i]);
//	myAISContext->SetLocation(ais, TopLoc_Location(myTrsf));
//	myAISContext->Display(ais);
	index++;




}




void CTESTOCCDoc::Display()
{	
	/// 우선 지우기 ///
	myAISContext->CloseAllContexts();
	myAISContext->RemoveAll();

	///
	
	for (int i = 0; i < vis_container.size(); i++)
	{
		switch (vis_container[i].type)
		{
		case MODEL:
			if (displaymodel)
			{
				myAISContext->Display(vis_container[i].obj);
			}
			break;
		case BOUNDARY:
			if (displayboundary)
			{
				vis_container[i].obj->SetColor(Quantity_Color(0, 1, 0, Quantity_TOC_RGB)); // 색상 설정
			//	vis_container[i].obj->SetTransparency(0.8); // 투명도 설정
				myAISContext->Display(vis_container[i].obj);
			}
			break;
		case BND_BOX:
			if (displaybndbox)
			{
				vis_container[i].obj->SetColor(Quantity_Color(0, 0, 1, Quantity_TOC_RGB)); // 색상 설정
				vis_container[i].obj->SetTransparency(0.5); // 투명도 설정
				myAISContext->Display(vis_container[i].obj);
			}
			break;
		case PIPE_IO:
			if (displaybndbox)
			{
					vis_container[i].obj->SetColor(Quantity_Color(1, 0, 0, Quantity_TOC_RGB)); // 색상 설정
				//	vis_container[i].obj->SetTransparency(0.8); // 투명도 설정
					myAISContext->Display(vis_container[i].obj);
			}
			break;
		case ROUTED_PIPE:
			if (displaypipe)
			{
				vis_container[i].obj->SetColor(Quantity_Color(0.2, 0.8, 0, Quantity_TOC_RGB)); // 색상 설정
				myAISContext->Display(vis_container[i].obj);
			}
		default:
			break;
			
		}
	}
}

void CTESTOCCDoc::ReDisplay(bool erasepipe)
{
//	static bool _dispmodel = displaymodel;
//	static bool _dispboundary = displayboundary;
//	static bool _dispbndbox = displaybndbox;
//	static bool _disppipe = displaypipe;


	/// 우선 지우기 ///
//	myAISContext->CloseAllContexts();
//	myAISContext->RemoveAll();
//	myAISContext->CloseLocalContext();    // 지금 버퍼를 닫고 새 버퍼를 연다?
//	myAISContext->OpenLocalContext();
	///
	if (erasepipe)
	{
		for (int i = 0; i < vis_container.size(); i++)
		{
			if (vis_container[i].type == ROUTED_PIPE)
			{
				myAISContext->Erase(vis_container[i].obj);
			}
		}
		
		vector<displayinfo>::iterator it = vis_container.begin();
		while (it != vis_container.end())
		{
			if ((it)->type == ROUTED_PIPE)
			{
				vector<displayinfo>::iterator it2 = it;
				it--;
				vis_container.erase(it2);
			}

			it++;
		}
		return;
	}



	for (int i = 0; i < vis_container.size(); i++)
	{
		switch (vis_container[i].type)
		{
		case MODEL:
//			if (displaymodel != _dispmodel)
			{
				if (displaymodel == 1)
				{
					if (!myAISContext->IsDisplayed(vis_container[i].obj))
						myAISContext->Display(vis_container[i].obj);
				}
				else
				{
					if (myAISContext->IsDisplayed(vis_container[i].obj))
						myAISContext->Erase(vis_container[i].obj);
				}
			}
			break;
		case BOUNDARY:
//			if (displayboundary != _dispboundary)
			{
				if (displayboundary == 1)
				{
					if (!myAISContext->IsDisplayed(vis_container[i].obj))
						myAISContext->Display(vis_container[i].obj);
				}
				else
				{
					if (myAISContext->IsDisplayed(vis_container[i].obj))
						myAISContext->Erase(vis_container[i].obj);
				}
			}
			break;
		case BND_BOX:
//			if (displaybndbox != _dispbndbox)
			{
				if (displaybndbox == 1)
				{
					if (!myAISContext->IsDisplayed(vis_container[i].obj))
						myAISContext->Display(vis_container[i].obj);
				}
				else
				{
					if (myAISContext->IsDisplayed(vis_container[i].obj))
						myAISContext->Erase(vis_container[i].obj);
				}
			}
			break;
		case PIPE_IO:
//			if (displaybndbox != _dispbndbox)
			{
				if (displaybndbox == 1)
				{
					if (!myAISContext->IsDisplayed(vis_container[i].obj))
						myAISContext->Display(vis_container[i].obj);
				}
				else
				{
					if (myAISContext->IsDisplayed(vis_container[i].obj))
						myAISContext->Erase(vis_container[i].obj);
				}
			}
			break;
		case ROUTED_PIPE:
//			if (displaypipe != _disppipe)
			{
				if (displaypipe == 1)
				{
					if (!myAISContext->IsDisplayed(vis_container[i].obj))
					{
						vis_container[i].obj->SetColor(Quantity_Color(0.2, 0.8, 0, Quantity_TOC_RGB)); // 색상 설정
						myAISContext->Display(vis_container[i].obj);
					}
				}
				else
				{
					if (myAISContext->IsDisplayed(vis_container[i].obj))
						myAISContext->Erase(vis_container[i].obj);
				}
			}
		default:
			break;

		}
	}

//	_dispmodel = displaymodel;
//	_dispboundary = displayboundary;
//	_dispbndbox = displaybndbox;
//	_disppipe = displaypipe;

}


void CTESTOCCDoc::DisplayOnly(Handle(AIS_InteractiveObject) obj)
{
	/// 우선 지우기 ///
	myAISContext->CloseAllContexts();
	myAISContext->RemoveAll();
	///
	myAISContext->Display(obj);
}


void CTESTOCCDoc::OnImportImport32774()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("STEP Files |*.stp; *.step|"));
	if (IDOK == dlg.DoModal())
	{
		CString filename = dlg.GetPathName();
		

		/// unicode 환경에서 CString -> char* 변환
		wchar_t *str;
		char*    name;
		int charlen;
		
		str = filename.GetBuffer(filename.GetLength());
		charlen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
		name = new char[charlen];
		WideCharToMultiByte(CP_ACP, 0, str, -1, name, charlen, 0, 0);
		//////////////////////////////////////////////////

		/// Import 수행
		Import(name);


		delete name;
	}
	grid_init();
	Display();
	//printf("%s", (LPSTR(LPCTSTR(filename))));

	

}
