
// TESTOCCDoc.h : CTESTOCCDoc Ŭ������ �������̽�
//


#pragma once

#define NUM_OF_IMPORT (10)


class CTESTOCCDoc : public CDocument
{
public:
	Handle(V3d_Viewer) myViewer;
	Handle(AIS_InteractiveContext) myAISContext;
	Handle(AIS_LocalContext) currentContext;
	/// STEP ������ �ε忡 �ʿ��� �͵�
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

	/// Selection�� �ʿ��� �͵� 
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
protected: // serialization������ ��������ϴ�.
	CTESTOCCDoc();
	DECLARE_DYNCREATE(CTESTOCCDoc)

// Ư���Դϴ�.
public:

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// �����Դϴ�.
public:
	virtual ~CTESTOCCDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// �˻� ó���⿡ ���� �˻� �������� �����ϴ� ����� �Լ�
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	afx_msg void OnImportImport32774();
};
