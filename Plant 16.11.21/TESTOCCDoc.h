
// TESTOCCDoc.h : CTESTOCCDoc 클래스의 인터페이스
//


#pragma once

#define NUM_OF_IMPORT (10)


class CTESTOCCDoc : public CDocument
{
public:
	Handle(V3d_Viewer) myViewer;
	Handle(AIS_InteractiveContext) myAISContext;
	Handle(AIS_LocalContext) currentContext;
	/// STEP 복수개 로드에 필요한 것들
	Handle(TDocStd_Document) aDoc[NUM_OF_IMPORT];
	Handle(XCAFDoc_ShapeTool) myAssembly[NUM_OF_IMPORT];
	Handle(XCAFDoc_ColorTool) myColors[NUM_OF_IMPORT];
	TDF_LabelSequence frshapes[NUM_OF_IMPORT];
	STEPCAFControl_Reader reader[NUM_OF_IMPORT];
	int num_of_file;

	int numofcomponent[NUM_OF_IMPORT];
	int numofassembly[NUM_OF_IMPORT];
	int numofpart[NUM_OF_IMPORT];
	TDF_Label *Parts[NUM_OF_IMPORT];
	TDF_Label *Components[NUM_OF_IMPORT];
	TDF_Label *Assemblys[NUM_OF_IMPORT];
	///

	/// Selection에 필요한 것들 
	Handle(AIS_InteractiveObject) Picked_Interactive;
	TopoDS_Shape PickedShape;
	bool Picked = 0;
	///

	
	int Selection_Index;
	


	Handle(AIS_InteractiveContext)& GetAISContext(){ return myAISContext; }
	Handle(V3d_Viewer) GetViewer(){ return myViewer; }
	Handle(AIS_InteractiveContext)& GetInteractiveContext(){ return myAISContext; }
	void Import(const char* filename);
	void Selection(int sel_mode);
	void Display();
	void DisplayOnly(Handle(AIS_InteractiveObject) obj);
	void ReDisplay(bool erasepipe = false);
	void End_Selection();
protected: // serialization에서만 만들어집니다.
	CTESTOCCDoc();
	DECLARE_DYNCREATE(CTESTOCCDoc)

// 특성입니다.
public:

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 구현입니다.
public:
	virtual ~CTESTOCCDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 검색 처리기에 대한 검색 콘텐츠를 설정하는 도우미 함수
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	afx_msg void OnImportImport32774();
};
