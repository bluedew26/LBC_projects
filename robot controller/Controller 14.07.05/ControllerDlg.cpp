#include "stdafx.h"
#include "Controller.h"
#include "ControllerDlg.h"
#include "afxdialogex.h"
#include "SelectComDlg.h"
#include <MMSystem.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
extern 
#define KEYREPEAT	25     // Ű���� �Է��νĹݺ����� (ms)
void Pushing(unsigned char dir);
int direction = 0; // 0 = ����, 1 = ����, 2 = ����,  3 = ��ȸ��, 4 = ��ȸ��

CString Liststring2;
CListBox List_Auto;
CListBox List_Course;

static void Addstring1(LPCTSTR string)
{
	List_Auto.InsertString(List_Auto.GetCount(),string);
	List_Auto.SetCurSel(List_Auto.GetCount() - 1);
}
static void Addstring2(CString string)
{
	List_Course.InsertString(List_Course.GetCount(),string);
	List_Course.SetCurSel(List_Course.GetCount() - 1);
}

/////////////// ���� ���� //////////////////
int Sonic[5] = {0}; // 0 - �߾�, 1 - ��������, 2 - �������, 3 - �Ĺ�����, 4 - �Ĺ����
int UR[3] = {0}; // 0 - ��ü�Ʒ�, 1 - ����, 2 - ����
int TILT[3] = {0}; // ��ü����, ��������, ��������

// ��� ���� //
CRect WIFI_rect; // ī�޶�1 ���â�� ��Ÿ���� Rect
Rect fr; // ������ġ // �������� �������� ��Ÿ��������
IplImage* display_temp;
		
/////// ��� ���� /////////
CCom m_pCom;
CCom m_pCom2;
bool is_connecting = false;
bool is_connecting2 = false;
bool sync = false; // �� ���� ����ΰ�.

///////////// �ڵ����� ���� //////////
int Timer_Auto; // �ڵ����� Ÿ�̸�
void CALLBACK TimeProc_Auto(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
bool auto_mode = 0; // �����������ΰ�.


/////// ���� ������� //////
stack<course> Stack;     // ���� �����̳� ����
course Course;			 // �̵���θ� ��� ����ü ����
int Timernum; // Ÿ�̸�1
void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
bool image_status = 1; // ������ �߰����� ����

//// ���� ȸ������ ���� ///
byte MOTOR7_pos;

//////////////////// define�� �������� /////////////////
#define MOTOR_NUM 6 // �� ���� ����
#define PATTERN_SAMPLE 20 // ���� ����

////////// ī�޶� �� ����ó������ //////////
IplImage *pOrgImg;	// drc_WIFI�� ���Ͽ� �޴� �κ�
IplImage *pOrgImg_drc3; // drc3.0�� ���Ͽ� �޴� �κ�.
IplImage *tempCapture; // ���� ���ǽ� ���� �κ�
IplImage *PatternImage[PATTERN_SAMPLE]; // ���� ������ �簢���� �ش�.
IplImage *PatternDisplayImage; // ������ ������ ǥ���ϴºκ�
IplImage *PatternResultImage; // �����ν� �߰������.
IplImage *DisplayImage;  // ���� ǥ���ϴºκ�
int Timernum3; // Ÿ�̸�3 (���� �������� ����)
HDC dc_display; // ���÷��� ȭ���� �޴� DC
HDC dc_display2; // ���÷��� ȭ��2 (Drc3.0)�� �޴� DC
bool Receive_Timer = 0; // 1�ϰ�� ȭ���� ���� (�����Ӱ���)
bool clear = 0; // ȭ�� �����ϴ� ����.

// ��κ��� //
int Timer_comeback;
byte command = 0;

///////// �����ν�1 ���� //////////
CvRect rectRoi;		// ���� ������ ũ��.
CRect m_Rect;
CRect Pattern_rect; // ���� ��Ī�� ��Ÿ���� ��� �簢��
Rect Matching_rect; // ��Ī1���� ���Ǵ� ��Ʈ

double minVal = 0.0; // ���� ��Ī ���� (�������� ��Ȯ)
double maxVal = 0.0; // �ְ� ��Ī ����
double minVal_min = 1.0;

bool matching_start = false; // ���� ������ true�� �ٲ�. �׷��� ��Ī ����.
byte sample_index = 0;

/////////////// DRC WIFI �κ� /////////////////////////
BYTE *pRemain	= NULL;
BYTE gBuf[MAX_FILE_LEN];
char buff[256];
CvFont _font; 
///////////////////////////////////////////////////////

//// DRC �κ� ///
bool startdrc3 = 0;
int Timer_drc3;
IplImage* DisplayImage2;
int CAM2_width = 0;
int CAM2_height = 0;

//////////////// SURF �˰��� ���� ���� ///////////////
#define OBJECT_NUM	6 /// �� ������Ʈ ����
CvRect rectRoi2;
IpPairVec matches;
IpVec ipts, ref_ipts[OBJECT_NUM];
CvPoint src_corners[OBJECT_NUM][4];
CvPoint dst_corners[OBJECT_NUM][4];
IplImage *Object[OBJECT_NUM]; // SURF�˰���� ���� �׸�����
IplImage *Object_Display;
int Object_Width;
int Object_Height;
int Timer_SURF; // SURFŸ�̸� (��Ī �ֱ� ����)

byte Object_index = 0; // ���� �߰��� ������Ʈ�� ����
byte matched_index = 0; // ���� ��Ī�� �ε���

bool matching_start2 = false; // SURF �˰��� ��� ����
bool SURF_start = false;
///////////////////////////////////////////////////////


// �ѹ���Ʈ�� �����͸� ������ �Լ�
static void Wait(DWORD dwMillisecond) // �и������� ��� �Լ�.
{
	MSG msg;
	DWORD dwStart;
	dwStart = GetTickCount();

	while(GetTickCount() - dwStart < dwMillisecond)
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

static void send(BYTE data)
{
	char send = data;
	m_pCom.WriteCommBlock(&send, 1);
}	
// ���� ������ ���
static void send(byte data1, byte data2) 
{
	timeKillEvent(Timernum);
	unsigned char send[2] = {data1 + 11, data2}; // 11�� ���ͺ��� ����
	m_pCom.WriteCommBlock(&send, 2);
	Timernum = timeSetEvent(KEYREPEAT, 0, &TimeProc, NULL, TIME_PERIODIC);
}
// ���� �������� ���
static void send_angle(byte data1, byte data2) 
{
	unsigned char send[2] = {data1, data2}; // 7�� ���ͺ��� ����
	m_pCom.WriteCommBlock(&send, 2);
}

static void send2(BYTE data) // ����2�� ���� ����.
{
	timeKillEvent(Timernum);
	char send = data;
	m_pCom2.WriteCommBlock(&send, 1);
	Timernum = timeSetEvent(KEYREPEAT, 0, &TimeProc, NULL, TIME_PERIODIC);
}

static void send2(byte data1, byte data2) // ����2 ���� ������ ���
{
	unsigned char send[2] = {data1 + 10, data2}; // 10�� ���ͺ��� ����
	m_pCom2.WriteCommBlock(&send, 2);
}

static void setspeed(byte data) // ������ �̵��ӵ� ����
{
	timeKillEvent(Timernum);
	unsigned char send[2] = {0xFE, data};
	m_pCom.WriteCommBlock(&send, 2);
	Timernum = timeSetEvent(KEYREPEAT, 0, &TimeProc, NULL, TIME_PERIODIC);
}

///////////////////////////////////////////










class CAboutDlg : public CDialogEx{public:CAboutDlg();enum { IDD = IDD_ABOUTBOX };protected:virtual void DoDataExchange(CDataExchange* pDX);protected:	DECLARE_MESSAGE_MAP()};
CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD){}
void CAboutDlg::DoDataExchange(CDataExchange* pDX){CDialogEx::DoDataExchange(pDX);}
void CControllerDlg::OnSysCommand(UINT nID, LPARAM lParam){if ((nID & 0xFFF0) == IDM_ABOUTBOX){	CAboutDlg dlgAbout;	dlgAbout.DoModal();	}else{	CDialogEx::OnSysCommand(nID, lParam);}}
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


CControllerDlg::CControllerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CControllerDlg::IDD, pParent)
	, m_volt(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CControllerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT11, m_volt);
	DDX_Control(pDX, IDC_LIST1, m_List);
	DDX_Control(pDX, IDC_TILT_CONTROL, m_slider);
	DDX_Control(pDX, IDC_AUTO_LIST, List_Course);
	DDX_Control(pDX, IDC_AUTO_LIST2, List_Auto);
}

BEGIN_MESSAGE_MAP(CControllerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_STARTCOM, &CControllerDlg::OnBnClickedStartcom)
	ON_BN_CLICKED(IDC_DISCONNECT, &CControllerDlg::OnBnClickedDisconnect)
	ON_WM_MOUSEWHEEL()
	ON_BN_CLICKED(IDC_TORQUEON, &CControllerDlg::OnBnClickedTorqueon)
	ON_BN_CLICKED(IDC_TORQUEOFF, &CControllerDlg::OnBnClickedTorqueoff)
	ON_MESSAGE(WM_RECEIVEDATA, &CControllerDlg::OnReceive)
	ON_BN_CLICKED(IDC_ADDLIST, &CControllerDlg::OnBnClickedAddlist)
	ON_BN_CLICKED(IDC_ERASE, &CControllerDlg::OnBnClickedErase)
	ON_BN_CLICKED(IDC_DELETE, &CControllerDlg::OnBnClickedDelete)
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SIMULATION, &CControllerDlg::OnBnClickedSimulation)
	ON_WM_COPYDATA()
	ON_BN_CLICKED(IDC_SAVECOURSE, &CControllerDlg::OnBnClickedSavecourse)
	ON_BN_CLICKED(IDC_COMEBACK, &CControllerDlg::OnBnClickedComeback)
	ON_BN_CLICKED(IDC_BUTTON_PATTERNDEFINE, &CControllerDlg::OnBnClickedButtonPatterndefine)
	ON_BN_CLICKED(IDC_BUTTON_PATTERNSAVE, &CControllerDlg::OnBnClickedButtonPatternsave)
	ON_BN_CLICKED(IDC_BUTTON1, &CControllerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_STARTCOM2, &CControllerDlg::OnBnClickedStartcom2)
	ON_BN_CLICKED(IDC_DISCONNECT2, &CControllerDlg::OnBnClickedDisconnect2)
	ON_BN_CLICKED(IDC_ARM_SYNC, &CControllerDlg::OnBnClickedArmSync)
	ON_BN_CLICKED(IDC_SURF, &CControllerDlg::OnBnClickedSurf)
	ON_BN_CLICKED(IDC_SURF_STOP, &CControllerDlg::OnBnClickedSurfStop)
	ON_BN_CLICKED(IDC_SURFPATTERN_SAVE, &CControllerDlg::OnBnClickedSurfpatternSave)
	ON_BN_CLICKED(IDC_RECEIVEIMAGE, &CControllerDlg::OnBnClickedReceiveimage)
	ON_BN_CLICKED(IDC_MATHING_INIT, &CControllerDlg::OnBnClickedMathingInit)
	ON_BN_CLICKED(IDC_BUTTON_PATTERN_LOADPATTERN, &CControllerDlg::OnBnClickedButtonPatternLoadpattern)
	ON_BN_CLICKED(IDC_BUTTON_PATTERN_LOADSURF, &CControllerDlg::OnBnClickedButtonPatternLoadsurf)
	ON_BN_CLICKED(IDC_SAVESAMPLE, &CControllerDlg::OnBnClickedSavesample)
	ON_BN_CLICKED(IDC_SAVESAMPLE2, &CControllerDlg::OnBnClickedSavesample2)
	ON_BN_CLICKED(IDC_AUTOMODE, &CControllerDlg::OnBnClickedAutomode)
	ON_BN_CLICKED(IDC_SENSING_LINE, &CControllerDlg::OnBnClickedSensingLine)
	ON_BN_CLICKED(IDC_BUTTON2, &CControllerDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CControllerDlg::OnBnClickedButton3)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_TILT_CONTROL, &CControllerDlg::OnNMReleasedcaptureTiltControl)
	ON_BN_CLICKED(IDC_BUTTON3, &CControllerDlg::OnBnClickedButton3)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_TILT_CONTROL, &CControllerDlg::OnNMReleasedcaptureTiltControl)
END_MESSAGE_MAP()


// CControllerDlg �޽��� ó����

BOOL CControllerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	SetTimer(5, 50, NULL);

	fr.x = 780; fr.y = 470; fr.width = 150; fr.height = 200;

	Init();
	Timernum3 = timeSetEvent(100, 0, &TimeProc, NULL, TIME_PERIODIC); // 50ms���� WIFI������ ����.
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	
	// DRC�ʱ�ȭ ///
	dc_display2 = GetDlgItem(IDC_PIC_CAM2)->GetDC()->m_hDC; // ���÷��� 2 (drc3.0�� dc�� ����)
	/// DRC WIFI �ʱ�ȭ ///////////////
	
	dc_display = GetDlgItem(IDC_PIC_CAM)->GetDC()->m_hDC;  // ���÷����� DC�� ����.
	DRC_ProgramInit();// DRC API���� �ڷᱸ�� �ʱ�ȭ 
	// ��������� ���� Mat���� 
	pOrgImg = cvCreateImage(cvSize(160,120), IPL_DEPTH_8U, 3);
	cvSet(pOrgImg, cvScalarAll(1),NULL); 	
	DisplayImage = cvCreateImage(cvSize(m_nWidthDisplay, m_nHeightDisplay), IPL_DEPTH_8U, 3);
	cvJpegMat 	= cvCreateMat(1, MAX_FILE_LEN, CV_8UC1 );	
	FlagNewImg	= 0;
	SetTimer(3,100,NULL);// 50ms ���� ����
	Drc_Rxmode = 0;

	//���� ������ �ʱ�ȭ 
	IMG_W = m_nWidthDisplay; // �̹��� ����
	IMG_H = m_nHeightDisplay; // �̹��� ����

	cvInitFont(&_font, CV_FONT_HERSHEY_DUPLEX , .2, 0.5, 0, 1, 32);


	///////////////////////////////////////////////



	/////////////////////////////////////////////// */

	memset(focusing, 0, sizeof(bool) * MOTOR_NUM);   // ���Ͱ��� ����� �ٲܺκ�
	
	SetDlgItemText(IDC_SPEED, _T("15"));
	SetDlgItemText(IDC_DELAY, _T("600"));

	////////////////////// ������ �ν� �κ� /////////////

	Timernum = timeSetEvent(KEYREPEAT, 0, &TimeProc, NULL, TIME_PERIODIC);
	Course.direction = 246;
	Course.saved = 1;
	Course.time = 0;
	///////////////////////////////////////////////
	


	m_slider.SetRange(32, 192);
	m_slider.SetRangeMin(32);
	m_slider.SetRangeMax(192);
	m_slider.SetPos(112);
	m_slider.SetTicFreq(2);

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}


void CControllerDlg::OnPaint()
{
	CPaintDC dc(this);
	//// ������ �׸� ���� �κ� ///

	CDC MemDC;
	CDC *pDC = GetDC();
	MemDC.CreateCompatibleDC(pDC);
	
	CBitmap Bitmap;
	Bitmap.LoadBitmap(IDB_FRAME);
	
	CBitmap *pOldBitmap = (CBitmap *)MemDC.SelectObject(&Bitmap);

	pDC->BitBlt(fr.x, fr.y, fr.width, fr.height, &MemDC, 0, 0, SRCCOPY);

	MemDC.SelectObject(pOldBitmap);
	ReleaseDC(pDC);
	///////////////////////////////
	
	//// ���̼� �׸��ºκ� ////////
	if(clear)
	{
		CPen pen;
		pen.CreatePen( PS_SOLID, 3, RGB(255,0,0));// ������
		CPen *oldPen = dc.SelectObject(&pen);
		if(Sonic[1] <= 20)
		{
			dc.MoveTo(fr.x, fr.y - Sonic[1]);
			dc.LineTo(fr.x+30, fr.y - Sonic[1]);
			
			clear = 0;
		}
		if(Sonic[2] <= 20)
		{
			dc.MoveTo(fr.x + fr.width - 30, fr.y - Sonic[2]);
			dc.LineTo(fr.x + fr.width, fr.y - Sonic[2]);
			
			clear = 0;
		}
		if(Sonic[3] <= 20)
		{
			dc.MoveTo(fr.x, fr.y + fr.height + Sonic[3]);
			dc.LineTo(fr.x + 30, fr.y + fr.height + Sonic[3]);
			
			clear = 0;
		}
		if(Sonic[4] <= 20)
		{
			dc.MoveTo(fr.x + fr.width - 30, fr.y + fr.height + Sonic[4]);
			dc.LineTo(fr.x + fr.width, fr.y + fr.height + Sonic[4]);
			
			clear = 0;
		}

		dc.SelectObject(oldPen);
	}
	
	///////////////////////////////

	//// DRC_WIFI�κ��� ����ް� ����ó���ϴ� �κ� /////////
	if(startdrc3)
	{
		
		CvvImage cv_processed2;	
		cvResize(pOrgImg_drc3, DisplayImage2);
			
	
		// ���÷��� �̹��� ��� //   
		cv_processed2.CopyOf(DisplayImage2, 1);
		
		cv_processed2.Show(dc_display2, 0, 0, CAM2_width, CAM2_height);
	}
	
	if(FlagNewImg)
	{
		
		///////////////// �����νĺκ� /////////////
		if(matching_start)
		{
			minVal_min = 1.0; // �ʱⰪ 1
			Pattern_rect.SetRectEmpty();
			for(int i=0; i<sample_index; i++)
			{
				int width1 = pOrgImg->width - PatternImage[i]->width + 1;
				int height1 = pOrgImg->height - PatternImage[i]->height + 1;
		
				if(width1 <= 0 || height1 <= 0)
					continue;

				PatternResultImage = cvCreateImage(cvSize(width1, height1), IPL_DEPTH_32F, 1);
				cvMatchTemplate(pOrgImg, PatternImage[i], PatternResultImage, CV_TM_SQDIFF_NORMED);
	
				CvPoint minLoc;
				CvPoint maxLoc;
				CvPoint matchLoc;
				double scoreResult = 0.0;
				cvMinMaxLoc(PatternResultImage, &minVal, &maxVal, &minLoc, &maxLoc);

				matchLoc = minLoc;
				scoreResult = minVal;
	
				if(minVal < minVal_min)
				{
					Pattern_rect.left = matchLoc.x; // ��Ī�� ��Ÿ�� Rect ����.
					Pattern_rect.top = matchLoc.y;
					Pattern_rect.right = matchLoc.x + PatternImage[i]->width;
					Pattern_rect.bottom = matchLoc.y + PatternImage[i]->height;
					minVal_min = minVal; // �ּҰ� ����
				}
				cvReleaseImage(&PatternResultImage);
			}
		}	
		/////////////////////////////////////////// 
		
		CvvImage cv_processed;	
		FlagNewImg = 0; // ������ ���� 
		cvReleaseImage(&pOrgImg);
		pOrgImg = cvDecodeImage(cvJpegMat, 1);
		cvFlip(pOrgImg, pOrgImg, -1);
		cvResize(pOrgImg, DisplayImage);
		
		//ImageProcess(pOrgImg);
		ImageProcess(DisplayImage); // �ʿ�� ����ó���� �����Ѵ�.
	
	
		// ���÷��� �̹��� ��� //   
		cv_processed.CopyOf(DisplayImage, 1);
		//cv_processed.CopyOf(pOrgImg, 1);
		
		cv_processed.Show(dc_display, 0, 0, m_nWidthDisplay, m_nHeightDisplay);

	}
  
	CDialogEx::OnPaint();
}

byte missing_type = 0; // ��Ī�� �Ҿ����� ����
int old_pos = 0; // ���� �簢���߾� 
int old_pos2 = 0;
int old_pos_surf = 0;
int old_pos_surf2 = 0;
void CControllerDlg::ImageProcess(IplImage *img){
	int SX = 5;
	int SY = 20;
	int LINEH = 18;
	// ī�޶�� ���� ���� ������ ���̸� ����� ���� 
	if(minVal_min < 0.03)
	{
		double dWidthRatio = (double)m_nWidthDisplay/(double)pOrgImg->width;
		double dHeightRatio = (double)m_nHeightDisplay/(double)pOrgImg->height;
		Matching_rect.x = (int)(Pattern_rect.left * dWidthRatio);
		Matching_rect.y = (int)(Pattern_rect.top * dHeightRatio);
		Matching_rect.width = (int)((Pattern_rect.right - Pattern_rect.left) * dWidthRatio);
		Matching_rect.height = (int)((Pattern_rect.bottom - Pattern_rect.top) * dHeightRatio);
		
		old_pos = Matching_rect.x + Matching_rect.width / 2;
		old_pos2 = Matching_rect.y + Matching_rect.height / 2;
		if(missing_type != 0)
			Addstring1(_T("��ǥ�� �߰�"));
		missing_type = 0; // missing�� �ƴ�
	}
	else if (matching_start)
	{
		if(old_pos)
		{
			if(old_pos > img->width*3/5)
			{
				if(direction == 3 || direction == 1 || direction == 4) //��ȸ�����̰ų� �����߾�����
					missing_type = 3; // �������� �������.
				else if (direction == 2) // �������̾�����
					missing_type = 5; // �Ÿ� �ν�
				else if(direction == 0) // �ƹ��͵� ���ϰ� �־�����
					missing_type = 3; // ��ü�� �̵���
			}
			else if (old_pos < img->width*2/5)
			{
				if(direction == 4 || direction == 1 || direction == 3) //��ȸ�����̰ų� �����߾�����
					missing_type = 2; // �������� �������.		
				else if (direction == 2) // �������̾�����
					missing_type = 5; // �Ÿ� �ν�
				else if(direction == 0) // �ƹ��͵� ���ϰ� �־�����
					missing_type = 2; // ��ü�� �̵���
			}
			else
			{
				if(direction == 2) // �������̾�����
					missing_type = 5; // �Ÿ� �ν�
				else if(direction == 1)
					missing_type = 6; // ��ü�� �����
				else if(direction == 0)
					missing_type = 4;
				else
				{
					if(old_pos2 > img->height/2)
						missing_type = 6; // ī�޶� �Ʒ��� ����� (��ü ����� ������ ���)
 					else
						missing_type = 5; // �Ÿ� �ν� (ī�޶� ���� �����)
				}
			}
			switch(missing_type)
			{
				case 2: Addstring1(_T("��ǥ���� �������� �����")); break;
				case 3: Addstring1(_T("��ǥ���� �������� �����")); break;
				case 4: Addstring1(_T("��ǥ���� �̵���")); break;
				case 5: Addstring1(_T("��ǥ���� �Ÿ��� �־���")); break;
				case 6: Addstring1(_T("��ǥ���� �����")); break;
				default: break;
			}
			old_pos = 0; // ������ ����
		}
	}
	if (matching_start2) // SURF �˰���
	{
		surfDetDes(img, ipts, false, 10, 10, 2, 0.000001f);
		for(int i=0; i<Object_index; i++)
		{
			getMatches(ipts,ref_ipts[i],matches);
			if (translateCorners(matches, src_corners[i], dst_corners[i]))
			{
				matched_index = i;
				break;
			}
		}
		matching_start2 = 0;
	}

	if (SURF_start) // ǥ���ϴ� �Լ�.
	{
		if (translateCorners(matches, src_corners[matched_index], dst_corners[matched_index]))
		{
		// Draw box around object
			int min_x = 1000, max_x = 0;
			int min_y = 1000, max_y = 0;
			for(int i = 0; i < 4; i++ )
			{
				
				CvPoint r1 = dst_corners[matched_index][i%4];
				CvPoint r2 = dst_corners[matched_index][(i+1)%4];
				cvLine( img, cvPoint(r1.x, r1.y),
				cvPoint(r2.x, r2.y), cvScalar(255,255,255), 3 );
				if(r1.x < min_x)
					min_x = r1.x;
				if(r1.x > max_x)
					max_x = r1.x;
				if(r1.y < min_y)
					min_y = r1.y;
				if(r1.y > max_x)
					max_y = r1.y;
			
			
			}
			old_pos_surf = (min_x+max_x)/2;
			old_pos_surf2 = (min_y+max_y)/2;


			for (unsigned int i = 0; i < matches.size(); ++i)
				drawIpoint(img, matches[i].first);
		}

		if(old_pos_surf)
		{
			if(old_pos_surf > img->width*3/5)
			{
				if(direction == 3 || direction == 1 || direction == 4) //��ȸ�����̰ų� �����߾�����
					missing_type = 3; // �������� �������.
				else if (direction == 2) // �������̾�����
					missing_type = 5; // �Ÿ� �ν�
				else if(direction == 0) // �ƹ��͵� ���ϰ� �־�����
					missing_type = 3; // ��ü�� �̵���
			}
			else if (old_pos_surf < img->width*2/5)
			{
				if(direction == 4 || direction == 1 || direction == 3) //��ȸ�����̰ų� �����߾�����
					missing_type = 2; // �������� �������.		
				else if (direction == 2) // �������̾�����
					missing_type = 5; // �Ÿ� �ν�
				else if(direction == 0) // �ƹ��͵� ���ϰ� �־�����
					missing_type = 2; // ��ü�� �̵���
			}
			else
			{
				if(direction == 2) // �������̾�����
					missing_type = 5; // �Ÿ� �ν�
				else if(direction == 1)
					missing_type = 6; // ��ü�� �����
				else if(direction == 0)
					missing_type = 4;
				else
				{
					if(old_pos_surf2 > img->height/2)
						missing_type = 6; // ī�޶� �Ʒ��� ����� (��ü ����� ������ ���)
 					else
						missing_type = 5; // �Ÿ� �ν� (ī�޶� ���� �����)
				}
			}
			switch(missing_type)
			{
				case 2: Addstring1(_T("��ǥ���� �������� �����")); break;
				case 3: Addstring1(_T("��ǥ���� �������� �����")); break;
				case 4: Addstring1(_T("��ǥ���� �̵���")); break;
				case 5: Addstring1(_T("��ǥ���� �Ÿ��� �־���")); break;
				case 6: Addstring1(_T("��ǥ���� �����")); break;
				default: break;
			}
			old_pos_surf = 0; // ������ ����
		}
	}
	if(minVal_min < 0.03)
		cvRectangle(img, cvPoint(Matching_rect.x, Matching_rect.y), cvPoint(Matching_rect.x + Matching_rect.width, Matching_rect.y +Matching_rect.height), cvScalar(0,255,0), 2);
	
	// ȭ�� ����


	drawFPS(img);
/*
	sprintf(buff, "ImgSize= %d B", ImgSize); // MakeUartTxData
	cvPutText(img, buff, cvPoint(SX,SY+LINEH*2), &_font, CV_RGB(0, 250, 0));
	
	// ī�޶�� ���� ���� ������ ũ�⸦ ����� ���� 
	sprintf(buff, "IMG_W=%3d, IMG_H=%3d", m_nWidthDisplay, m_nHeightDisplay); 
	cvPutText(img, buff, cvPoint(SX,SY+LINEH*3), &_font, CV_RGB(0, 250, 0));
*/
	
	sprintf(buff, "Minval=%.3lf", minVal_min); 
	cvPutText(img, buff, cvPoint(SX,SY+LINEH*4), &_font, CV_RGB(0, 250, 0));
	
	cvLine(img, cvPoint(img->width / 2, img->height - 40), cvPoint(img->width/2, img->height), CV_RGB(100,150,0), 2);

	if(image_status)
	{
		if(Sonic[0] <= 40)
		{
			if(TILT[0] >= 75 && TILT[0] <= 125)
			{
				int h = img->height/2 + (40 - Sonic[0]) * (img->height/2) / 20;
				cvLine(img, cvPoint(img->width/2 - 30, h), cvPoint(img->width/2 + 30, h), CV_RGB(255, 0, 0), 3);
			}
		}
		if(Sonic[1] <= 20)
		{
			int h = (img->height/2) + (img->height/2) * (20 - Sonic[1]) / 20;	
			int h2 = (img->height/2) + (img->height/2) * (15) / 20;	
			cvLine(img, cvPoint(0, h), cvPoint(50, h), CV_RGB(255, 0, 0), 3);
			cvLine(img, cvPoint(0, h2), cvPoint(50, h2), CV_RGB(0, 255, 0), 3);
		}
		if(Sonic[2] <= 20)
		{
			int h = (img->height/2) + (img->height/2) * (20 - Sonic[2]) / 20;	
			int h2 = (img->height/2) + (img->height/2) * (15) / 20;	
			
			cvLine(img, cvPoint(img->width - 50, h), cvPoint(img->width, h), CV_RGB(255, 0, 0), 3);
			cvLine(img, cvPoint(img->width - 50, h2), cvPoint(img->width, h2), CV_RGB(0, 255, 0), 3);
		
		}
		/*
		if(UR[1] >= 50)
		{
			int w = (img->width/2) + (img->width/2) * (0.5 + ((UR[1] - 50) / 90.0));
			if(w > img->width) w = img->width - 3;
			int w2 = (img->width/2) + (img->width/2) * (0.5 + ((75 - 50) / 90.0));
			cvLine(img, cvPoint(w2, img->height/2 - 30), cvPoint(w2, img->height/2 + 30), CV_RGB(0, 255, 0), 3);
			cvLine(img, cvPoint(w, img->height/2 - 30), cvPoint(w, img->height/2 + 30), CV_RGB(255, 0, 0), 3);
		}
		if(UR[2] >= 50)
		{
			int w = (img->width/2) - (img->width/2) * (0.5 + ((UR[2] - 50) / 90.0));
			if(w < 0) w = 3;
			int w2 = (img->width/2) - (img->width/2) * (0.5 + ((75 - 50) / 90.0));
			cvLine(img, cvPoint(w2, img->height/2 - 30), cvPoint(w2, img->height/2 + 30), CV_RGB(0, 255, 0), 3);
			cvLine(img, cvPoint(w, img->height/2 - 30), cvPoint(w, img->height/2 + 30), CV_RGB(255, 0, 0), 3);

		}
		*/
			
	}
}
void CControllerDlg::OnTimer(UINT_PTR nIDEvent)
{
	static int flag = 0;
	if(nIDEvent == 1) // 0�� ī����. DRC�κ��� ������ ����.
	{
		
		COPYDATASTRUCT msg;
		PORT_ADD addr;
		HANDLE handle = AfxGetInstanceHandle();//�ڽ��� �ν��Ͻ� ȹ��
		CWnd *pWnd = AfxGetMainWnd();//�ڽ��� �ڵ� ȹ��
		addr.nWnd = pWnd->m_hWnd;
	//	memcpy(addr.val , drcdata, sizeof(drcdata));
		msg.cbData = sizeof(addr);
		msg.dwData = UM_MOUSESTATUS;
		msg.lpData = (PVOID)&addr;
	
	
		//sendMessage �� �̿��Ͽ� DRC Black Manager ���� ����� ���� ȣ��
		::SendMessage(hDRC, WM_COPYDATA, (WPARAM)handle,(LPARAM)&msg);

	//	UpdateData(FALSE);

	}

	else if (nIDEvent == 2) // �� ���� Ÿ�̸�
	{
		send(0, m_miniarmpos[0]);
		send(1, m_miniarmpos[1]);
		send(2, m_miniarmpos[2]);
		send(3, m_miniarmpos[3]);
		send(4, m_miniarmpos[4] - 14);
		send(5, m_miniarmpos[5] + 5);
	}
	
	else if (nIDEvent == 3)
	{
		if(StartButtonClick==1)
		{		
			Drc_WaitCnt++; // �ڵ����� ...������Ž� 0���� ���� 

			if(Drc_WaitCnt>20 && Drc_Retry_flag==0)
			{ // �����ð� �̻� ��������� �ȵǸ� �ٽ� ��û 			
				DRC_RequestImage();
				Drc_Retry_flag = 1;
			}
		}
		if(Drc_ConnState==1&&flag==0) 
		{
			flag = 1;
			GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);// ��ư Ȱ��ȭ 	
		}
	}
	
	else if (nIDEvent == 5)
	{
		if(!sync)
		{
			if(GetAsyncKeyState(0x44)) // aŰ
			{
				int a = GetDlgItemInt(IDC_EDIT1);
				send(0, a-16);
			}
			else if(GetAsyncKeyState(0x41)) // dŰ
			{
				int a = GetDlgItemInt(IDC_EDIT1);
				send(0, a+16);
			}
			else if(GetAsyncKeyState(0x58)) // wŰ
			{
				int a = GetDlgItemInt(IDC_EDIT3);
				send(4, a-5);
			}
			else if(GetAsyncKeyState(0x54)) // sŰ
			{
				int a = GetDlgItemInt(IDC_EDIT3);
				send(4, a+5);
			}
		}
	}
	// �Ϲ������� //

	
	
	CDialogEx::OnTimer(nIDEvent);
}
	
HCURSOR CControllerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CControllerDlg::OnBnClickedStartcom()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(!is_connecting)			// ����� �������� �ƴ� ��
	{
		CString strTmp;			// �ӽ� ���ڿ� ����
		SelectComDlg dlg;		// COM��Ʈ��ȣ�� �����ϴ� Dialog ����.
		dlg.DoModal();			// Dialog�� ���
		strTmp = dlg.m_selected;   // Dialog�� �޺��ڽ� ������ ������ COM��Ʈ ��ȣ�� �ӽ� ���ڿ��� �ű�
		m_pCom.SetComPort(strTmp, 57600, 8, 0, 0);  // �������� ����. COM��Ʈ�� Dialog���� ������ ��
													   // Baudrate 115200
													   // ���� ��Ʈ 8��Ʈ
													   // Stop��Ʈ 1�� ����.
													   // �и�Ƽ ����
		if(!m_pCom.OpenComPort())					   // COM��Ʈ�� ����.
			AfxMessageBox(_T("��� ���ῡ �����߽��ϴ�."));  // ��Ʈ�� ���µ� ������ ��� �޼����� ����.
		else												// ��Ʈ�� ���µ� �����ϸ�
		{
			m_pCom.SetHwnd(this->m_hWnd);					// �� Ŭ����(Mainframe)�� ������ �ڵ��� �޴´�.
			is_connecting = true;							// ������ �� ���·� �ٲ�.
//			send(220);
			Addstring1(_T("(�˸�)������� ���� �Ϸ�"));
		}
	}
	else					// �̹� ������̾��ٸ�
		AfxMessageBox(_T("�̹� ����Ǿ� �ֽ��ϴ�!"));	// �޼��� ���.
}
void CControllerDlg::OnBnClickedStartcom2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(!is_connecting2)			// ����� �������� �ƴ� ��
	{
		CString strTmp;			// �ӽ� ���ڿ� ����
		SelectComDlg dlg;		// COM��Ʈ��ȣ�� �����ϴ� Dialog ����.
		dlg.DoModal();			// Dialog�� ���
		strTmp = dlg.m_selected;   // Dialog�� �޺��ڽ� ������ ������ COM��Ʈ ��ȣ�� �ӽ� ���ڿ��� �ű�
		m_pCom2.SetComPort(strTmp, 57600, 8, 0, 0);  // �������� ����. COM��Ʈ�� Dialog���� ������ ��
													   // Baudrate 115200
													   // ���� ��Ʈ 8��Ʈ
													   // Stop��Ʈ 1�� ����.
													   // �и�Ƽ ����
		if(!m_pCom2.OpenComPort())					   // COM��Ʈ�� ����.
			AfxMessageBox(_T("��� ���ῡ �����߽��ϴ�."));  // ��Ʈ�� ���µ� ������ ��� �޼����� ����.
		else												// ��Ʈ�� ���µ� �����ϸ�
		{
			m_pCom2.SetHwnd(this->m_hWnd);					// �� Ŭ����(Mainframe)�� ������ �ڵ��� �޴´�.
			is_connecting2 = true;							// ������ �� ���·� �ٲ�.
			Addstring1(_T("(�˸�)��Ʈ�ѷ� ���� �Ϸ�"));
		}
	}
	else					// �̹� ������̾��ٸ�
		AfxMessageBox(_T("�̹� ����Ǿ� �ֽ��ϴ�!"));	// �޼��� ���.
}

void CControllerDlg::OnBnClickedDisconnect()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(is_connecting)					// ������ �Ǿ��ִ� ���¶��
	{
		m_pCom.CloseConnection();		// �����Ʈ�� �ݴ´�. 
		is_connecting = false;
		Addstring1(_T("(�˸�)���� ����"));
	}
	else
		Addstring1(_T("(�˸�)�������� �ƴ�"));
}


void CControllerDlg::OnBnClickedDisconnect2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if(is_connecting2)					// ������ �Ǿ��ִ� ���¶��
	{
		m_pCom2.CloseConnection();		// �����Ʈ�� �ݴ´�. 
		is_connecting2 = false;
		Addstring1(_T("(�˸�)���� ����"));
	}
	else
		Addstring1(_T("(�˸�)�������� �ƴ�"));
}


// DYNAMIXEL�κ��� ���� �����͵��� ������ִ� �Լ�.
afx_msg LRESULT CControllerDlg::OnReceive(WPARAM wParam, LPARAM lParam) // ���� ������ ���� �޶���
{
	static int count = 0;
	unsigned short i;			  // �ݺ� �ε���
	CString str;
	for(i=0;i<wParam;i++)		  // wParam�� �ѹ��� ������ ����Ʈ ���� ������. �� Ƚ����ŭ �ݺ�
	{
		if((unsigned char)rx_buffer[i] == 0xFF) count = 0; // ����� �ð�� ī��Ʈ ����
		
		if(!sync) // ��ũ��尡 �ƴҵ��� ���κ����� ���Ͱ��� ����.
		{
			switch(count)
			{
			case 1:
				Sonic[0] = (byte)rx_buffer[i];
				str.Format(_T("%dcm"), Sonic[0]); // ��հ��� ǥ����
				SetDlgItemText(IDC_SONIC1, str);
				break;
			case 2:
				Sonic[1] = (byte)rx_buffer[i] - 3;
				str.Format(_T("%dcm"), Sonic[1]); // ��հ��� ǥ����
				SetDlgItemText(IDC_SONIC2, str);
				break;
			case 3:
				Sonic[2] = (byte)rx_buffer[i] - 3;
				str.Format(_T("%dcm"), Sonic[2]); // ��հ��� ǥ����
				SetDlgItemText(IDC_SONIC3, str);
				InvalidateRect(CRect(fr.x, fr.y - 20, fr.x+30, fr.y),TRUE); // ����
				InvalidateRect(CRect(fr.x + fr.width - 30, fr.y - 20, fr.x+fr.width, fr.y),TRUE);
				InvalidateRect(CRect(fr.x, fr.y +fr.height, fr.x+30, fr.y + fr.height + 20),TRUE);
				InvalidateRect(CRect(fr.x + fr.width - 30, fr.y + fr.height, fr.x+fr.width, fr.y + fr.height + 20),TRUE);
				clear = 1;										  // �� Ŭ����
				break;	
			case 4:
				TILT[0] = (byte)rx_buffer[i];
				break;
			case 7:
				m_volt.Format(_T("%.1fV"), (unsigned char)rx_buffer[i] / 10.0);
				break;
			case 8:
				SetDlgItemInt(IDC_EDIT1, (byte)rx_buffer[i]);
				break;
			case 9:
				SetDlgItemInt(IDC_EDIT2, (byte)rx_buffer[i]);
				break;
			case 10:
				SetDlgItemInt(IDC_EDIT3, (byte)rx_buffer[i]);
				break;
			case 11:
				SetDlgItemInt(IDC_EDIT4, (byte)rx_buffer[i]);
				break;
			case 12:
				SetDlgItemInt(IDC_EDIT5, (byte)rx_buffer[i]);
				break;
			case 13:
				SetDlgItemInt(IDC_EDIT12, (byte)rx_buffer[i]);
				break;
			case 15:
				Sonic[3] = (byte)rx_buffer[i] - 3;
				str.Format(_T("%dcm"), Sonic[3]); // ��հ��� ǥ����
				SetDlgItemText(IDC_SONIC4, str);
				break;
			case 16:
				Sonic[4] = (byte)rx_buffer[i] - 3;
				str.Format(_T("%dcm"), Sonic[4]); // ��հ��� ǥ����
				SetDlgItemText(IDC_SONIC5, str);
				break;
		
			case 17:
				UR[0] = (byte)rx_buffer[i];
				break;
			case 18:
				UR[1] = (byte)rx_buffer[i];
				break;
			case 19:
				UR[2] = (byte)rx_buffer[i];
				break; 
			default:
				break;	
			}
		} // ���κ��忡 ���� ����.

		else //��ũ���
		{
			switch(count)
			{
			case 1:
				m_miniarmpos[0] = (unsigned char)rx_buffer[i];
				SetDlgItemInt(IDC_EDIT1, m_miniarmpos[0]);
				break;
			case 2:
				m_miniarmpos[1] = (unsigned char)rx_buffer[i];
				SetDlgItemInt(IDC_EDIT2, m_miniarmpos[1]);
				break;
			case 3:
				m_miniarmpos[2] = (unsigned char)rx_buffer[i];
				SetDlgItemInt(IDC_EDIT3, m_miniarmpos[2]);
				break;
			case 4:
				m_miniarmpos[3] = (unsigned char)rx_buffer[i];
				SetDlgItemInt(IDC_EDIT4, m_miniarmpos[3]);
				break;
			case 5:
				m_miniarmpos[4] = (unsigned char)rx_buffer[i];
				SetDlgItemInt(IDC_EDIT5, m_miniarmpos[4]);
				break;
			case 6:
				m_miniarmpos[5] = (unsigned char)rx_buffer[i];
				SetDlgItemInt(IDC_EDIT12, m_miniarmpos[5]);
				break;
			default:
				break;	
			}
		} // �����ȿ� ���� ����.
		count++;
		
	}
	UpdateData(FALSE);

	return 0;
}


// ���������� ��ũ ON/OFF�ϴ� �Լ�.

void CControllerDlg::OnBnClickedTorqueon() { send(250); }
void CControllerDlg::OnBnClickedTorqueoff(){ send(251); }


void CControllerDlg::OnBnClickedAddlist() // ���� ������ ���� ����.
{
	bool invalid = false;
	int a1,a2,a3,a4,a5,a6, delay, speed;
	
	if(!sync)
	{
		a1 = GetDlgItemInt(IDC_EDIT1);
		a2 = GetDlgItemInt(IDC_EDIT2);
		a3 = GetDlgItemInt(IDC_EDIT3);
		a4 = GetDlgItemInt(IDC_EDIT4);
		a5 = GetDlgItemInt(IDC_EDIT5);
		a6 = GetDlgItemInt(IDC_EDIT12);
	}
	else
	{
		a1 = m_miniarmpos[0];
		a2 = m_miniarmpos[1];
		a3 = m_miniarmpos[2];
		a4 = m_miniarmpos[3];
		a5 = m_miniarmpos[4] - 14;
		a6 = m_miniarmpos[5] + 23;
	}
	speed = GetDlgItemInt(IDC_SPEED);
	
	if(!(speed >= 0 && speed <= 100)) invalid = true;
	delay = GetDlgItemInt(IDC_DELAY);
	if(delay <= 0) invalid = true;

	if(invalid)
		AfxMessageBox(_T("���� ������ ����ų� �߸��� ���� �ֽ��ϴ�"));
	else
	{
		CString str;
		str.Format(_T("%03d %03d %03d %03d %03d %03d SPEED:%03d DELAY:%d"), a1, a2, a3, a4, a5, a6, speed, delay); // ���������� 0����ä��
	
		m_List.InsertString(m_List.GetCount(),str);
		m_List.SetCurSel(m_List.GetCount() - 1);
	}
}

void CControllerDlg::OnBnClickedErase(){ m_List.ResetContent(); }
void CControllerDlg::OnBnClickedDelete(){ m_List.DeleteString(m_List.GetCurSel()); } 
// GetCurSel�� �����Ѻκ��� ��ȣ�� �޾� ����.







void CControllerDlg::OnBnClickedSimulation()
{
	int index = m_List.GetCount();
	unsigned int Motor1[20], Motor2[20], Motor3[20], Motor4[20], Motor5[20], Motor6[20], speed[20], delay[20];
	CString str;
	
	for(int i=0; i<index; i++)
	{
		m_List.GetText(i, str);
		Motor1[i] = _ttoi(str.Left(3));
		str.Delete(0, 4); // �տ��� 4ĭ ����.
		Motor2[i] = _ttoi(str.Left(3));
		str.Delete(0, 4);
		Motor3[i] = _ttoi(str.Left(3));
		str.Delete(0, 4);
		Motor4[i] = _ttoi(str.Left(3));
		str.Delete(0, 4);
		Motor5[i] = _ttoi(str.Left(3));
		str.Delete(0, 4);
		Motor6[i] = _ttoi(str.Left(3));
		str.Delete(0, 10);
		speed[i] = _ttoi(str.Left(3));
		str.Delete(0, 10);
		delay[i] = _ttoi(str);
		
	}
	for(int i=0; i<index; i++)
	{


		setspeed(speed[i]); // �ӵ� ����

		//Wait(delay[i]);
		m_List.SetCurSel(i);
		timeKillEvent(Timernum);	
		Wait(25);
		send(0, Motor1[i]);
		send(1, Motor2[i]);
		send(2, Motor3[i]);
		send(3, Motor4[i]);
		send(4, Motor5[i]);
		send(5, Motor6[i]);
		Wait(delay[i] - 25);
		Timernum = timeSetEvent(KEYREPEAT, 0, TimeProc, NULL, TIME_PERIODIC);
		
	}
}




BOOL CControllerDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	Drc_RetrieveFrame(hDRC, pCopyDataStruct, pOrgImg_drc3);// �� �������� pOrgImg ������ ����  
	cvFlip(pOrgImg_drc3, pOrgImg_drc3, -1);
	InvalidateRect(WIFI_rect,FALSE);
	return CDialogEx::OnCopyData(pWnd, pCopyDataStruct);
}



void Pushing(unsigned char dir) // ���ϰ� ���ÿ� �ִ� �Լ�.
{
}

void Go_Forward() {
	send(242);
	if(Course.direction != 242)
	{
		if(Course.time > 1)
		{
			Stack.push(Course);
			CString c;
			switch(Course.direction)
			{
			case 242: c.Format(_T("����"));	break;
			case 243: c.Format(_T("����"));	break;
			case 244: c.Format(_T("��ȸ��"));	break;
			case 245: c.Format(_T("��ȸ��"));	break;
			case 246: c.Format(_T("����"));	break;
			default: break;
			}
			Liststring2.Format(_T("%4dms  %4s  ���� : %d"), Course.time*25, c, Course.saved);
			Addstring2(Liststring2); 
		}
		Course.time = 0;
		Course.direction = 242;
		Course.saved = 0;
		
	}
	direction = 1; //����
}

void Go_Forward_with_scan() {
	send(248);
	if(Course.direction != 242)
	{
		if(Course.time > 1)
		{
			Stack.push(Course);
			CString c;
			switch(Course.direction)
			{
			case 242: c.Format(_T("����"));	break;
			case 243: c.Format(_T("����"));	break;
			case 244: c.Format(_T("��ȸ��"));	break;
			case 245: c.Format(_T("��ȸ��"));	break;
			case 246: c.Format(_T("����"));	break;
			default: break;
			}
			Liststring2.Format(_T("%4dms  %4s  ���� : %d"), Course.time*25, c, Course.saved);
			Addstring2(Liststring2); 
		}
		Course.time = 0;
		Course.direction = 242;
		Course.saved = 0;
		
	}
	direction = 1; //����
}


void Go_Back() {
	send(243);
	if(Course.direction != 243)
	{
		if(Course.time > 1)
		{
			Stack.push(Course);
			CString c;
			switch(Course.direction)
			{
			case 242: c.Format(_T("����"));	break;
			case 243: c.Format(_T("����"));	break;
			case 244: c.Format(_T("��ȸ��"));	break;
			case 245: c.Format(_T("��ȸ��"));	break;
			case 246: c.Format(_T("����"));	break;
			default: break;
			}
			Liststring2.Format(_T("%4dms  %4s  ���� : %d"), Course.time*25, c, Course.saved);
			Addstring2(Liststring2); 
		}
		Course.time = 0;
		Course.direction = 243;
		Course.saved = 0;
	}
	direction = 2;
}

void Turn_Left() {
	if(Course.direction != 244)
	{
		if(Course.time > 1)
		{
			Stack.push(Course);
			CString c;
			switch(Course.direction)
			{
			case 242: c.Format(_T("����"));	break;
			case 243: c.Format(_T("����"));	break;
			case 244: c.Format(_T("��ȸ��"));	break;
			case 245: c.Format(_T("��ȸ��"));	break;
			case 246: c.Format(_T("����"));	break;
			default: break;
			}
			Liststring2.Format(_T("%4dms  %4s  ���� : %d"), Course.time*25, c, Course.saved);
			Addstring2(Liststring2); 
		}
		Course.time = 0;
		Course.direction = 244;
		Course.saved = 0;
	}
	send(244);
	direction = 3;
}

void Turn_Right() {
	if(Course.direction != 245)
	{
		if(Course.time > 1)
		{
			Stack.push(Course);
			CString c;
			switch(Course.direction)
			{
			case 242: c.Format(_T("����"));	break;
			case 243: c.Format(_T("����"));	break;
			case 244: c.Format(_T("��ȸ��"));	break;
			case 245: c.Format(_T("��ȸ��"));	break;
			case 246: c.Format(_T("����"));	break;
			default: break;
			}
			Liststring2.Format(_T("%4dms  %4s  ���� : %d"), Course.time*25, c, Course.saved);
			Addstring2(Liststring2);
		} 
		Course.time = 0;
		Course.direction = 245;
		
		Course.saved = 0;
	}
	send(245); // �ޱ� ����
	direction = 4;
}

void stop()
{
	if(Course.time > 1 && Course.direction != 246)
	{
		Stack.push(Course);
		CString c;
		switch(Course.direction)
		{
		case 242: c.Format(_T("����"));	break;
		case 243: c.Format(_T("����"));	break;
		case 244: c.Format(_T("��ȸ��"));	break;
		case 245: c.Format(_T("��ȸ��"));	break;
		case 246: c.Format(_T("����"));	break;
		default: break;
		}
		Liststring2.Format(_T("%4dms  %4s  ���� : %d"), Course.time*25, c, Course.saved);
		Addstring2(Liststring2); 
		Course.time = 0;
		Course.direction = 246;
	}
	if(Course.time > 5)
		Course.time = 5;
	send(246);
	direction = 0;
}
void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{	
	if(uID == Timernum)
	{
		if(!auto_mode)
		{
			if(GetAsyncKeyState(VK_LEFT))  
				Turn_Left();
			else if (GetAsyncKeyState(VK_RIGHT))  
				Turn_Right();
			else if (GetAsyncKeyState(VK_UP))   // ��
				Go_Forward();
			else if (GetAsyncKeyState(VK_DOWN)) // ��
				Go_Back();
			else if (GetAsyncKeyState(0x55)) // ��
				send(247);
			else				// �ƹ�Ű�� �ȴ�������
			{
				stop();
			}
		}
		Course.time++;
	}
	else if(uID == Timernum3)
	{
		Receive_Timer = 1;
	}
	else if(uID == Timer_SURF)
	{
		matching_start2 = 1;
	}
	else if(uID == Timer_comeback)
	{
		send(command);
	}
}



void CControllerDlg::OnBnClickedSavecourse()    // ������� �Լ�
{
	if(Stack.empty())
	{
		Addstring1(_T("(�˸�)������ ����־ ������ �� ����"));
		return;
	}
	Addstring1(_T("(�˸�)��� ����"));
	course temp = Stack.top();
	temp.saved = 1;
	Stack.push(temp);
	CString str3;
	
	List_Course.GetText(List_Course.GetCount() - 1, str3);
	str3.Delete(str3.GetLength() - 1, 1);
	str3.Insert(str3.GetLength(), _T("1"));
	List_Course.DeleteString(List_Course.GetCount()-1);
	List_Course.InsertString(List_Course.GetCount(), str3);
	List_Course.SetCurSel(List_Course.GetCount() - 1);
}

void CControllerDlg::OnBnClickedComeback()     // ��κ��� �Լ�
{
	
	
	timeKillEvent(Timernum);    // �̵������ �޴� Ÿ�̸Ӹ� ����.
	Timer_comeback = timeSetEvent(25, 0, TimeProc, NULL, TIME_PERIODIC);
	while(1)
	{
		if(Stack.empty())
		{
			Addstring1(_T("(�˸�)����Ʈ�� ����־� ������ �� ����"));
			break;
		}
		
		course temp = Stack.top();
		if(temp.saved)
		{
			temp.saved = 0; // ������������ ���尪�� 0���� ����� �ٽ� �������
			CString str3;
	
			List_Course.GetText(List_Course.GetCount() - 1, str3);
			str3.Delete(str3.GetLength() - 1, 1);
			str3.Insert(str3.GetLength(), _T("0"));
			List_Course.DeleteString(List_Course.GetCount()-1);
			List_Course.InsertString(List_Course.GetCount(), str3);
			List_Course.SetCurSel(List_Course.GetCount() - 1);
			Stack.pop();
			Stack.push(temp);
			break;
		}
		Stack.pop();

		switch(temp.direction)
		{
		case 242:	command = 243; break;
		case 243:	command = 242; break;
		case 244:	command = 245; break;
		case 245:	command = 244; break;
		case 246:	command = 246; break;
		default: break;
		}

		Wait(25*(temp.time + 1));
		
		CString c;
		int a= List_Course.GetCount();
		List_Course.GetText(a - 1, c);
		if(_ttoi(c.Right(1)) == 0)
		{
			List_Course.DeleteString(a - 1);
			List_Course.SetCurSel(a - 1);
		}
	}
	Timernum = timeSetEvent(KEYREPEAT, 0, TimeProc, NULL, TIME_PERIODIC);
	command = 0;
	timeKillEvent(Timer_comeback);
}



void CControllerDlg::OnBnClickedButtonPatterndefine()
{
	tempCapture = cvCloneImage(pOrgImg);
	cvShowImage("PatternDefine", tempCapture);
	cvSetMouseCallback("PatternDefine", OnMousePatternDefine, this); // cv�� �ݹ�.
}

void CControllerDlg::OnMousePatternDefine(int iEvent, int iX, int iY, int iFlags, void* Userdata)
{
	CControllerDlg* pCControllerDlg = (CControllerDlg*)Userdata;
	switch(iEvent)
	{
	case EVENT_MOUSEMOVE:
		{
		}break;
	case EVENT_LBUTTONDOWN: // ���������� ��ǥ��
		{
			m_Rect.left = iX;
			m_Rect.top = iY;                 // rect ���� ������.
		}break;
	case EVENT_LBUTTONUP: // �������� ��ǥ�� Rect�� ����.
		{
			// stop draw
			m_Rect.right = iX;
			m_Rect.bottom = iY;
			IplImage* temp = cvCloneImage(tempCapture);
			cvRectangle(temp, cvPoint(m_Rect.left, m_Rect.top), cvPoint(m_Rect.right, m_Rect.bottom), cvScalar(255), 2);
			cvShowImage("PatternDefine", temp);
			
		}break;
	}
}

void CControllerDlg::OnBnClickedButtonPatternsave()
{
	rectRoi.x = m_Rect.left;
	rectRoi.y = m_Rect.top;
	rectRoi.width = m_Rect.right - m_Rect.left;
	rectRoi.height = m_Rect.bottom - m_Rect.top;

	// get pattern image
	if(sample_index < PATTERN_SAMPLE)
	{
		cvSetImageROI(tempCapture, cvRect(rectRoi.x, rectRoi.y, rectRoi.width, rectRoi.height));
		cvReleaseImage(&PatternImage[sample_index]);
		PatternImage[sample_index] = cvCreateImage(cvSize(rectRoi.width, rectRoi.height), IPL_DEPTH_8U, 3);
		cvResize(tempCapture, PatternImage[sample_index]); // ��������
		cvDestroyWindow("PatternDefine"); //������ ������.
		Invalidate(FALSE);	
		
		if(sample_index == 0)
			matching_start = true; // ��Ī1����
		CvvImage cv_pattern;	
		cvResize(PatternImage[sample_index], PatternDisplayImage);
		HDC dc_pattern = GetDlgItem(IDC_PIC_PATTERN_TARGET)->GetDC()->m_hDC;        
		cv_pattern.CopyOf(PatternDisplayImage, 1);
		cv_pattern.Show(dc_pattern, 0, 0, m_nWidthPatternDisplay, m_nHeightPatternDisplay);

		sample_index++;
		SetDlgItemInt(IDC_SAMPLE2, sample_index); // ���� ���� ����
	}
	else
		AfxMessageBox(_T("���� ������ �ʰ��Ͽ����ϴ�."));

}
void CControllerDlg::Init()
{
	// init Display rect and create image
	CRect rect;

	display_temp = cvCreateImage(cvSize(1024,768), IPL_DEPTH_8U, 3);

	GetDlgItem(IDC_PIC_CAM2)->GetClientRect(&rect);
	CAM2_width = rect.right;
	CAM2_height = rect.bottom;

	GetDlgItem(IDC_PIC_CAM)->GetClientRect(&rect);
	m_nWidthDisplay = rect.right;		  // ���÷����� ���� ����
	m_nHeightDisplay = rect.bottom;	      // ���÷����� ���� ����
	GetDlgItem(IDC_PIC_PATTERN_TARGET)->GetClientRect(&rect);
	m_nWidthPatternDisplay = rect.right;
	m_nHeightPatternDisplay = rect.bottom;
	PatternDisplayImage = cvCreateImage(cvSize(m_nWidthPatternDisplay, m_nHeightPatternDisplay), IPL_DEPTH_8U, 3);
	
	GetDlgItem(IDC_PIC_PATTERN2)->GetClientRect(&rect); // ������Ʈ ǥ�ÿ� ���� �簢�� ����
	Object_Width = rect.right;
	Object_Height = rect.bottom;
	Object_Display = cvCreateImage(cvSize(Object_Width, Object_Height), IPL_DEPTH_8U, 3);
	
	
	cvSubS(PatternDisplayImage, cvScalar(255,255,255), PatternDisplayImage);
	for(int i=0; i<OBJECT_NUM; i++)
		Object[i] = cvCreateImage(cvSize(5, 5), IPL_DEPTH_8U, 3); // ������ �ʱ�ȭ
	for(int i=0; i<PATTERN_SAMPLE; i++)
		PatternImage[i] = cvCreateImage(cvSize(5,5), IPL_DEPTH_8U, 3);

	GetDlgItem(IDC_PIC_CAM2)->GetWindowRect(WIFI_rect);
	ScreenToClient(WIFI_rect);
}



void CControllerDlg::OnBnClickedButton1()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	DRC_RequestImage(); // ī�޶󿡰� ���� ��û 
	StartButtonClick = 1;
}



void CControllerDlg::OnBnClickedArmSync() // �� ����ȭ
{
	if(!sync) // ������ư ������
	{
		SetDlgItemText(IDC_ARM_SYNC, _T("���� ����"));
		sync = true;
		SetTimer(2, 35, NULL);
		send(240); // ��������
		send2(240);
	}
	else
	{
		SetDlgItemText(IDC_ARM_SYNC, _T("��Ʈ�ѷ� ����"));
		KillTimer(2);
		sync = false;
		send(241); // ��������
		send2(241);
		send(0, m_miniarmpos[0]);
		send(1, m_miniarmpos[1] + 13);
		send(2, m_miniarmpos[2]);
		send(3, m_miniarmpos[3]);
		send(4, m_miniarmpos[4] - 15);
		send(5, m_miniarmpos[5] + 5);

	}
}



void CControllerDlg::OnBnClickedSurf() // SURF �������� �Լ�.
{
	tempCapture = cvCloneImage(pOrgImg);
	cvShowImage("PatternDefine", tempCapture);
	cvSetMouseCallback("PatternDefine", OnMousePatternDefine, this); // cv�� �ݹ�.
	
}


void CControllerDlg::OnBnClickedSurfStop()
{
//	for(int i=0; i<Object_index; i++)
//		cvReleaseImage(&Object[i]);
	Object_index = 0; // �ε��� 0��
	SetDlgItemInt(IDC_SAMPLE, Object_index);
	timeKillEvent(Timer_SURF); // Ÿ�̸�����
	CvvImage SURF_pattern;	
	cvSubS(Object_Display, cvScalar(255,255,255), Object_Display);
	HDC dc_pattern2 = GetDlgItem(IDC_PIC_PATTERN2)->GetDC()->m_hDC;        
	SURF_pattern.CopyOf(Object_Display, 1);
	SURF_pattern.Show(dc_pattern2, 0, 0, Object_Width, Object_Height);
	SURF_start = 0;
}






void CControllerDlg::DRC_Connect(){
Drc_ConnState = 1;
}
void CControllerDlg::DRC_Close(){	}
void CControllerDlg::DRC_SendCallBack(){	}
void CControllerDlg::DRC_ReceiveCallBack()
{
	int iBufSize = 1024;
	int i = 0;

	BYTE *pRxD = (BYTE *)&Drc_RxD;

	int sum=0;
	int tmp=0;
	static int k=0;
	static int REMAINS = 0;
	static int OldWidth = 0;

	tmp = Drc_ServerSocket.Receive(gBuf, iBufSize);

	if(tmp!=SOCKET_ERROR)
	{
		for(i=0;i<tmp;i++)
		{
			switch(Drc_Rxmode){
					
				case M_WAIT_HEADER0:
					k=0;
					if(gBuf[i]==0x55) {
						Drc_Rxmode=M_WAIT_HEADER1;
						pRxD[k++] = gBuf[i];
						}
					break;

				case M_WAIT_HEADER1:
					if(gBuf[i]==0x33){
						Drc_Rxmode=M_WAIT_HEADER2;
						pRxD[k++] = gBuf[i];
						}
					else
						Drc_Rxmode=M_WAIT_HEADER0;	
					break;
					
				case M_WAIT_HEADER2:
					if(gBuf[i]==0xAA){
						Drc_Rxmode=M_GET_INFO;
						pRxD[k++] = gBuf[i];	
			//			memset((u8 *)&Drc_RxD, 0, sizeof(DRC_RX_PKT));
						}
					else
						Drc_Rxmode=M_WAIT_HEADER0;						
					break;
				
				case M_GET_INFO:
					pRxD[k++] = gBuf[i];				
					if(k >= sizeof(DRC_RX_PKT)){
						Drc_Rxmode	= M_GET_REMAIN;
						REMAINS = Drc_RxD.AckDataLen + Drc_RxD.UartRxLen;
						pRemain	= (BYTE *)malloc(REMAINS);
			//			memset(pRemain, 0, REMAINS );
						k=0;
						}
					break;
						
				case M_GET_REMAIN:
					
					pRemain[k++] = gBuf[i];	
					if(k >= REMAINS) //  receive finish
					{
						// decode ack data
						memcpy((BYTE *)&tAck, pRemain, sizeof(ACK_TYPE_IMG));				
						ImgSize	= tAck.iJpegLen;
						// raw image data copy
						memcpy((u8 *)(cvJpegMat->data.ptr),	pRemain+sizeof(ACK_TYPE_IMG), ImgSize);
						Drc_Rxmode		= M_WAIT_HEADER0; // mode change
						 				
						Drc_WaitCnt 	= 0;	// reset wait counter 

						Drc_Retry_flag 	= 0;
						if(!startdrc3)
							DRC_RequestImage();			// ask next image to Camera
						else
							return;
						FlagNewImg 	= 1; 		// new image	
						if(Receive_Timer)
						{

							// Call OnPaint()
							FlagNewImg 	= 1; 		// new image	
							if(OldWidth == IMG_W)
								OldWidth = IMG_W;
							else
							{
								CRect rect;
								GetDlgItem(IDC_PIC_CAM)->GetWindowRect(rect);
								ScreenToClient(rect);
								InvalidateRect(rect,FALSE);
							}
							Receive_Timer = 0;
						}
							
						if(pRemain != NULL)
						{	
							free(pRemain);
							pRemain = NULL;
						}
					}
					break;
					
				}
		}
	}

}

// ī�޶󿡰� ������ ��û��.
void CControllerDlg::DRC_RequestImage(void)
{
	static int i = 0;

	static int SENFLAG=1;
	int S = 0;
	char *pSen;
	if(SENFLAG==1){
		int GROUP_A = 0;
		int GROUP_B = 1;
		int GROUP_C = 2;
		int SenRegCnt 	= 3; //���� 
		pSen = (char *)malloc(3*SenRegCnt); 	
		pSen[S+0] = GROUP_A; 	pSen[S+1] = 0x6A; 	pSen[S+2] = 0x87; 	S += 3;
		pSen[S+0] = GROUP_A;	pSen[S+1] = 0x6A;	pSen[S+2] = 0x47;	S += 3;
		pSen[S+0] = GROUP_A;	pSen[S+1] = 0x6A;	pSen[S+2] = 0x87;	S += 3;
		}


	Drc_Cmd.CmdType		= TYPE_GET_REALTIME_IMG;
	Drc_Cmd.CmdDataLen	= sizeof(CMD_TYPE_IMG)+S;
	// DRC-WIFI �� ������ UART TX �� ũ�⸦ 25byte �� ���� ����(Max 100byte)
	Drc_Cmd.UartTxLen	= UartTxDataLen;
	Drc_Cmd.TotalLen	= sizeof(DRC_TX_PKT) + Drc_Cmd.CmdDataLen + Drc_Cmd.UartTxLen;

	CMD_TYPE_IMG typeImg;
	memset(&typeImg, 0 , sizeof(CMD_TYPE_IMG));
	
	// ���� �ޱ���ϴ� ������ ũ�⸦ ����(Max 640x480)

	///////////////// ����ũ�⼳�� 320x240, 640x480�� ��1 ////////
	typeImg.aWidth		= 320/8;
	typeImg.aHeight 	= 240/8;	
	////////////////////////////////////////////////////////
	
	// ������ �̹��� ����Ƽ�� ���� (1~0)
	typeImg.Quality 	= 70;	
	////////////////////////////////////////////////////////////


	typeImg.aRecord		= 0;
	
	char *pData = (char *)malloc(Drc_Cmd.TotalLen); 
	memcpy(pData, 						(BYTE *)&Drc_Cmd, 	sizeof(DRC_TX_PKT));	// make "DRC_TX_PKT"
	memcpy(pData+sizeof(DRC_TX_PKT), 	(BYTE *)&typeImg, 	sizeof(CMD_TYPE_IMG));	// make "CMD_TYPE_IMG"
	if(SENFLAG==1){
		memcpy(pData+sizeof(DRC_TX_PKT)+sizeof(CMD_TYPE_IMG), 	pSen, 	S);	// make "SENSOR init"
		SENFLAG = 0;
		free(pSen);
		}
	int tmp = Drc_ServerSocket.Send(pData, Drc_Cmd.TotalLen);
	
	if(tmp == SOCKET_ERROR){
		StartButtonClick = 0;
		AfxMessageBox(_T("Can't Send  !"));
		
	}
	free(pData);

}


// DRC API���� �ڷᱸ�� �ʱ�ȭ 
void CControllerDlg::DRC_ProgramInit(){

	// socket programming initA
	if (!AfxSocketInit())
		{
			AfxMessageBox(_T("AfxSocketInit FAIL"));
			OnOK();
		}
	Drc_Retry_flag = 0;
	Drc_ServerSocket.SetParent(this);

	if(!Drc_ServerSocket.Create()){// ������ ����
		int iErr = GetLastError(); 
		printf("1 iErr %d\n",iErr);
		}
	
	CString mIPaddr	= _T("192.168.123.10"); 
	int m_iPort		= 6400;
	if(!Drc_ServerSocket.Connect(mIPaddr, m_iPort)){// ������ IP�ּҿ� ��Ʈ��ȣ�� ����
		int iErr = GetLastError();  
		}
	// �ۼ��� ��Ŷ �ڷᱸ�� �ʱ�ȭ 
	memset((u8 *)&Drc_Cmd, 0, sizeof(DRC_TX_PKT));
	memset((u8 *)&Drc_RxD, 0, sizeof(DRC_RX_PKT));

	Drc_Cmd.HEAD[0]		= 0x55;
	Drc_Cmd.HEAD[1]		= 0x33;
	Drc_Cmd.HEAD[2]		= 0xAA;
	Drc_Rxmode		= M_WAIT_HEADER0;
	Drc_WaitCnt		= 0;
	Drc_ConnState 	= 0;
	
}


void CControllerDlg::OnBnClickedSurfpatternSave()
{
	
	// check rect range
	rectRoi2.x = m_Rect.left;
	rectRoi2.y = m_Rect.top;
	rectRoi2.width = m_Rect.right - m_Rect.left;
	rectRoi2.height = m_Rect.bottom - m_Rect.top;

	// get pattern image
	if(Object_index < OBJECT_NUM)
	{
		cvReleaseImage(&Object[Object_index]);
		cvSetImageROI(tempCapture, cvRect(rectRoi2.x, rectRoi2.y, rectRoi2.width, rectRoi2.height));
		Object[Object_index] = cvCreateImage(cvSize(rectRoi2.width, rectRoi2.height), IPL_DEPTH_8U, 3);
		cvResize(tempCapture, Object[Object_index]); // ��������
		cvDestroyWindow("PatternDefine"); //������ ������.

		
		if(Object_index == 0)
		{
			SURF_start = 1;
			Timer_SURF = timeSetEvent(300, 0, &TimeProc, NULL, TIME_PERIODIC); // 0.3�ʸ���
		}
		src_corners[Object_index][0] = cvPoint(0,0);
		src_corners[Object_index][1] = cvPoint(Object[Object_index]->width,0);
		src_corners[Object_index][2] = cvPoint(Object[Object_index]->width, Object[Object_index]->height);
		src_corners[Object_index][3] = cvPoint(0, Object[Object_index]->height);
		surfDetDes(Object[Object_index], ref_ipts[Object_index], false, 10, 10, 2, 0.000001f); // �̺κ��� ������ ���е� �ٲٱ�
		drawIpoints(Object[Object_index], ref_ipts[Object_index]);

		
		// �����̹��� ���
		CvvImage SURF_pattern;	
		cvResize(Object[Object_index], Object_Display);
		HDC dc_pattern2 = GetDlgItem(IDC_PIC_PATTERN2)->GetDC()->m_hDC;        
		SURF_pattern.CopyOf(Object_Display, 1);
		SURF_pattern.Show(dc_pattern2, 0, 0, Object_Width, Object_Height);
		Object_index++;
		SetDlgItemInt(IDC_SAMPLE, Object_index);
	}
	else
		AfxMessageBox(_T("���� ������ �ʰ��Ͽ����ϴ�."));
}


void CControllerDlg::OnBnClickedReceiveimage()
{
	
	if(!startdrc3) // ������ư ������
	{
		SetDlgItemText(IDC_RECEIVEIMAGE, _T("DRC ��������"));
		SetTimer(1,100,NULL);
		startdrc3 = 1;
	
		hDRC = Drc_CaptureFromCAM(drcdata);
		pOrgImg_drc3 = cvCreateImage(cvSize(160, 120), IPL_DEPTH_8U, 3);//���� �̹��� ����
		DisplayImage2 = cvCreateImage(cvSize(CAM2_width, CAM2_height), IPL_DEPTH_8U, 3);
	
		cvSet(pOrgImg, cvScalarAll(1),NULL); 	
	
		drcdata[PREAMP_G_N] = 4;
	
	}
	else
	{
		KillTimer(1);
		startdrc3 = 0;
		SetDlgItemText(IDC_RECEIVEIMAGE, _T("DRC �������"));
	}
}


void CControllerDlg::OnBnClickedMathingInit()
{
	sample_index = 0; // �ε��� 0��
	SetDlgItemInt(IDC_SAMPLE2, sample_index);
	CvvImage pattern;	
	cvSubS(PatternDisplayImage, cvScalar(255,255,255), PatternDisplayImage);
	HDC dc_pattern = GetDlgItem(IDC_PIC_PATTERN_TARGET)->GetDC()->m_hDC;        
	pattern.CopyOf(PatternDisplayImage, 1);
	pattern.Show(dc_pattern, 0, 0, m_nWidthPatternDisplay, m_nHeightPatternDisplay);
	matching_start = false;
	minVal_min = 1.0;
}


void CControllerDlg::OnBnClickedButtonPatternLoadpattern() // ���� �ҷ�����
{
	CFileDialog fDlg(true, NULL, NULL, OFN_ALLOWMULTISELECT,_T("Image Files (*.jpg; *.png; *.bmp; *.gif)|*.jpg; *.png; *.bmp; *.gif||"));
	CString fileName;
	fDlg.m_ofn.lpstrFile = fileName.GetBuffer( 4096 );
	fDlg.m_ofn.nMaxFile = 4096;      
	int iRet = fDlg.DoModal();
	if(iRet != IDOK)
		return;
	else
	{
		for(POSITION pos = fDlg.GetStartPosition(); pos!=NULL;)
		{
			if(sample_index < PATTERN_SAMPLE)
			{
				CString strFileName = fDlg.GetNextPathName(pos);
				//// �ҷ����� �����κ� (���ϸ� \\ �� �߰��ϱ�)
				char *file_name = new char[strFileName.GetLength() + 1];; // CString���� char*�� ��ȯ
				strcpy(file_name, CT2A(strFileName));
		
				for(int i=strFileName.GetLength()-1; i>0; i--)
				{
					if (file_name[i] == '\\') // \�� �ΰ��� ǥ���ؾ���
						strFileName.Insert(i, _T("\\"));  // \�߰�
				}
				char *file_name2 = new char[strFileName.GetLength() + 1];
				strcpy(file_name2, CT2A(strFileName));
				delete[] file_name;
		
				//////////////////////////////////////////
				cvReleaseImage(&PatternImage[sample_index]);
				PatternImage[sample_index] = cvLoadImage(file_name2, 1);
	
				//////////// �����Ҵ� ���� //////
				delete[] file_name2;
				////////////////////////////////
				
				if(sample_index == 0)
					matching_start = true; // ��Ī1����

				if(pos == NULL || sample_index == PATTERN_SAMPLE - 1) // ������ �����̸�
				{
					CvvImage cv_pattern;	
					cvResize(PatternImage[sample_index], PatternDisplayImage);
					HDC dc_pattern = GetDlgItem(IDC_PIC_PATTERN_TARGET)->GetDC()->m_hDC;        
					cv_pattern.CopyOf(PatternDisplayImage, 1);
					cv_pattern.Show(dc_pattern, 0, 0, m_nWidthPatternDisplay, m_nHeightPatternDisplay);
				}
				
				sample_index++;
				SetDlgItemInt(IDC_SAMPLE2, sample_index); // ���� ���� ����
			}
			else
			{
				AfxMessageBox(_T("���� ������ �ʰ��Ͽ����ϴ�."));
				return;
			}
		}
	}
}


void CControllerDlg::OnBnClickedButtonPatternLoadsurf()
{
	CFileDialog fDlg(true, NULL, NULL, OFN_ALLOWMULTISELECT,_T("Image Files (*.jpg; *.png; *.bmp; *.gif)|*.jpg; *.png; *.bmp; *.gif||"));
	CString fileName;
	fDlg.m_ofn.lpstrFile = fileName.GetBuffer( 4096 );
	fDlg.m_ofn.nMaxFile = 4096;      
	int iRet = fDlg.DoModal();
	if(iRet != IDOK)
		return;
	else
	{
		for(POSITION pos = fDlg.GetStartPosition(); pos!=NULL;)
		{
			if(Object_index < OBJECT_NUM)
			{
				//// �ҷ����� �����κ� (���ϸ� \\ �� �߰��ϱ�)
				CString strFileName = fDlg.GetNextPathName(pos);
				char *file_name = new char[strFileName.GetLength() + 1];; // CString���� char*�� ��ȯ
				strcpy(file_name, CT2A(strFileName));
		
				for(int i=strFileName.GetLength()-1; i>0; i--)
				{
					if (file_name[i] == '\\') // \�� �ΰ��� ǥ���ؾ���
						strFileName.Insert(i, _T("\\"));  // \�߰�
				}
				char *file_name2 = new char[strFileName.GetLength() + 1];
				strcpy(file_name2, CT2A(strFileName));
				delete[] file_name;
		
				/////////////////////////////////////

				cvReleaseImage(&Object[Object_index]);
				Object[Object_index] = cvLoadImage(file_name2);

				delete[] file_name2;

				if(Object_index == 0)
				{
					SURF_start = 1;
					Timer_SURF = timeSetEvent(300, 0, &TimeProc, NULL, TIME_PERIODIC); // 0.3�ʸ���
				}
				src_corners[Object_index][0] = cvPoint(0,0);
				src_corners[Object_index][1] = cvPoint(Object[Object_index]->width,0);
				src_corners[Object_index][2] = cvPoint(Object[Object_index]->width, Object[Object_index]->height);
				src_corners[Object_index][3] = cvPoint(0, Object[Object_index]->height);
				surfDetDes(Object[Object_index], ref_ipts[Object_index], false, 10, 10, 2, 0.000001f); // �̺κ��� ������ ���е� �ٲٱ�
				//drawIpoints(Object[Object_index], ref_ipts[Object_index]);
				//I����Ʈ�� �׸��� ����
				if(pos == NULL || Object_index == OBJECT_NUM - 1) // ������ �����̸�
				{
					CvvImage SURF_pattern;	
					cvResize(Object[Object_index], Object_Display);
					HDC dc_pattern2 = GetDlgItem(IDC_PIC_PATTERN2)->GetDC()->m_hDC;        
					SURF_pattern.CopyOf(Object_Display, 1);
					SURF_pattern.Show(dc_pattern2, 0, 0, Object_Width, Object_Height);
				}
				Object_index++;
				SetDlgItemInt(IDC_SAMPLE, Object_index); // ���� ���� ����
			}
			else
			{
				AfxMessageBox(_T("���� ������ �ʰ��Ͽ����ϴ�."));
				break;
			}
		}
	}
}


void CControllerDlg::OnBnClickedSavesample()
{
	char path[255] = {0};
	for(int i=0; i<sample_index; i++)
	{
		sprintf(path, "C:\\Users\\Administrator\\Documents\\sample_%d.jpg", i+1);
		cvSaveImage((LPCSTR)path, PatternImage[i]);	
	}
	AfxMessageBox(_T("�� ������ ����Ǿ����ϴ�."));
}


void CControllerDlg::OnBnClickedSavesample2()
{
	char path[255] = {0};
	for(int i=0; i<Object_index; i++)
	{
		sprintf(path, "C:\\Users\\Administrator\\Documents\\surf_sample_%d.jpg", i+1);
		cvSaveImage((LPCSTR)path, Object[i]);	
	}
	AfxMessageBox(_T("�� ������ ����Ǿ����ϴ�."));
}



byte mode = 0; // 0�� �ʱ���, 1��  
void CControllerDlg::OnBnClickedAutomode()
{
	
	if(!auto_mode) // ������ư ������
	{
		SetDlgItemText(IDC_AUTOMODE, _T("�ڵ����� ����"));
		Addstring1(_T("(�ڵ�����)����"));
		Wait(500);
		missing_type = 1;
		auto_mode = true;
		stop(); // ����
		Timer_Auto = timeSetEvent(25, 0, &TimeProc_Auto, NULL, TIME_PERIODIC);
		send(238 - 11, 100);
		m_slider.SetPos(100);
		
		CString str3;
	
		List_Course.GetText(List_Course.GetCount() - 1, str3);
		str3.Delete(str3.GetLength() - 1, 1);
		str3.Insert(str3.GetLength(), _T("1"));
		List_Course.DeleteString(List_Course.GetCount()-1);
		List_Course.InsertString(List_Course.GetCount(), str3);
		List_Course.SetCurSel(List_Course.GetCount() - 1);

	}
	else
	{
		SetDlgItemText(IDC_AUTOMODE, _T("�ڵ����� ���"));
		Addstring1(_T("(�ڵ�����)����"));
		auto_mode = false;
		timeKillEvent(Timer_Auto);
		stop();
		mode = 0;
		send(238 - 11, 112);
		m_slider.SetPos(112);
	}
	// �ڵ����� ����� Ÿ�̸� ����ֱ�
}



#define ���氨���Ÿ� 25
#define ��ĵ�ð�	60
#define ����������Ÿ� 40
unsigned int t_time1 = 0; // ��ĵ�� Ÿ�̸�
unsigned int t_time2 = 0; // ȸ���� Ÿ�̸�


byte left_timer = 0;
byte right_timer = 0;
byte start_value1 = 0;
byte start_value2 = 0;
byte end_value1 = 0;
byte end_value2 = 0;
bool left_empty = 0;
bool right_empty = 0;
int Waitcount = 0 ;
byte dir = 3; // �������
void CALLBACK TimeProc_Auto(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if(mode == 0) // �⺻���
	{
		if(missing_type == 0 && matching_start)
		{
			if(old_pos < DisplayImage->width*4/5 && old_pos > DisplayImage->width*1/5) // �߾���
			{
				Go_Forward_with_scan();
				if(dir != 0)
				{
					dir = 0;
					Addstring1(_T("(�ڵ�����)��ǥ���� �߾����� ġ��ħ, ����"));
				}
			}
			else if(old_pos <= DisplayImage->width*1/5)
			{
				if(dir != 1)
				{
					dir = 1;
					Addstring1(_T("(�ڵ�����)��ǥ���� ī�޶� �������� ġ��ħ, ��ȸ��"));
				}
				Turn_Left();
			}
			else if(old_pos >= DisplayImage->width*4/5)
			{
				Turn_Right();
				if(dir != 2)
				{
					dir = 2;
					Addstring1(_T("(�ڵ�����)��ǥ���� ī�޶� �������� ġ��ħ, ��ȸ��"));
				}
			}
		}
		else if(missing_type == 6 && matching_start) // ��ü�� ��������
		{
			Addstring1(_T("(�ڵ�����)��ǥ���� ����Ͽ� �ڵ������� ������"));
			auto_mode = false;
			timeKillEvent(Timer_Auto);
			stop();
			mode = 0;
			send(238 - 11, 124);
		}
		else if(missing_type == 3 && matching_start)
		{
			if(dir != 2)
			{
				dir = 2;
				Addstring1(_T("(�ڵ�����)��ǥ���� �������� �����, ��ȸ��"));
			}
			Turn_Right();
		}
		else if(missing_type == 2 && matching_start)
		{
			if(dir != 1)
			{
				dir = 1;
				Addstring1(_T("(�ڵ�����)��ǥ���� �������� �����, ��ȸ��"));
			}
			Turn_Left();
		}
		else if(Sonic[1] <= ���氨���Ÿ�  || Sonic[2] <= ���氨���Ÿ� || Sonic[0] <= 40)
		{
			Addstring1(_T("(�ڵ�����)���濡 ��ֹ� �߰�"));
			t_time1 = ��ĵ�ð�;
			t_time2 = 0;
			left_timer = 220;
			right_timer = 220;
			left_empty = 0;
			right_empty = 0;
			start_value1 = 0;
			start_value2 = 0;
			end_value1 = 0;
			end_value2 = 0;
			stop();
			Wait(150);
			send(247);
			mode = 1;
			
			
			// Put(��ĵ���)
		}
		
		else
			Go_Forward_with_scan(); // ����
	}
	else if(mode == 1) // ��ĵ���
	{
#define ����������ð�	20 // 1��
		if(t_time1 > 0)
		{	
			t_time1--;

			if(Sonic[1] >= ����������Ÿ�)
			{
				left_timer--;
				if(left_empty == 0)
				{
					left_empty = 1;
					start_value1 = t_time1;
				}
			}
			else
			{
				left_timer = ����������ð�;
				start_value1 = 0;
				left_empty = 0;
			}
			if(Sonic[2] >= ����������Ÿ�)
			{
				right_timer--;
				if(right_empty == 0)
				{
					right_empty = 1;
					start_value2 = t_time1;
				}
			}
			else
			{
				start_value2 = 0;
				right_timer = ����������ð�;
				right_empty = 0;
			}
			
			if(left_timer == 0)
			{
				end_value1 = t_time1;
				Addstring1(_T("(�ڵ�����)������ �� ���� �߰�"));
				
				stop();
				left_empty = 1;
				right_empty = 0;
				t_time1 = 0;
				t_time2 = (��ĵ�ð� - (start_value1 + end_value1) / 2) / 2 + 10; // +5�� ������ ȸ���ϴ� �ð� ����
				
				Waitcount = 20; 
				mode = 2; // ����� 1
				
				return;
				// ������ ����� �߰�
			}
			if(right_timer == 0)
			{
				end_value2 = t_time1;
				Addstring1(_T("(�ڵ�����)������ �� ���� �߰�"));
				stop();
				Waitcount = 20; 
				
				left_empty = 0;
				right_empty = 1;
				t_time1 = 0; // ����
				t_time2 = (��ĵ�ð� - (start_value1 + end_value1) / 2) / 2 + 10; // +5�� ������ ȸ���ϴ� �ð� ����
				mode = 2; // ȸ�����
				
				return;
			}
		}
		else// ��ĵ ������ ��� (t_time == 0)
		{
			Addstring1(_T("(�ڵ�����)���� ��� ������� �߰����� ����"));
				
			mode = 2;
			stop();
			if(Sonic[1] - Sonic[2] > 0) // ������ ���� �� �������
			{
				t_time2 = 73;
				left_empty = 1; right_empty = 0;
			}
			else
			{
				t_time2 = 73;
				left_empty = 0; right_empty = 1;
			}
			Waitcount = 20; 		
		}

	}
	else if (mode == 2)
	{
		if(Waitcount)
			Waitcount--;
		else
		{
			if(left_empty == 1)
				Addstring1(_T("(�ڵ�����)�������� ȸ�����"));
			else if (right_empty == 1)
				Addstring1(_T("(�ڵ�����)���������� ȸ�����"));
			mode = 3;
		}
	}
	else if (mode == 3)
	{
		if(t_time2 > 0)
		{
			if(left_empty == 1)
				Turn_Left();
			else if (right_empty == 1)
				Turn_Right();
			t_time2--;
			if((Sonic[0] <= 33 ) && t_time2 >  23)
			{
				t_time2 = 23;
				Addstring1(_T("(�ڵ�����)������ �߰�. ȸ������"));
			}
		}
		else
		{
			mode = 4;
			left_empty = 0;
			right_empty = 0;
			stop();
			Waitcount = 20;
		}
	}
	else if (mode == 4)
	{
		if(Waitcount)
			Waitcount--;
		else
			mode = 0;
	}
}



void CControllerDlg::OnBnClickedSensingLine()
{
	if(image_status)
		image_status = 0;
	else
		image_status = 1;
}




void CControllerDlg::OnBnClickedButton2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	send(238 - 11, 112);
	m_slider.SetPos(112);
}



void CControllerDlg::OnBnClickedButton3()
{
	send(0, 128);
	send(1, 20);
	send(2, 62);
	send(3, 209);
	send(4, 188);
	send(5, 110);
}


void CControllerDlg::OnNMReleasedcaptureTiltControl(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	send(238 - 11, m_slider.GetPos());
	SetDlgItemInt(IDC_ATTRIBUTE, m_slider.GetPos());
	*pResult = 0;
}
