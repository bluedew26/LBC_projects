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
#define KEYREPEAT	25     // 키보드 입력인식반복구간 (ms)
void Pushing(unsigned char dir);
int direction = 0; // 0 = 정지, 1 = 전진, 2 = 후진,  3 = 좌회전, 4 = 우회전

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

/////////////// 센서 관련 //////////////////
int Sonic[5] = {0}; // 0 - 중앙, 1 - 전방좌측, 2 - 전방우측, 3 - 후방좌측, 4 - 후방우측
int UR[3] = {0}; // 0 - 차체아래, 1 - 좌측, 2 - 우측
int TILT[3] = {0}; // 차체기울기, 좌측기울기, 우측기울기

// 출력 관련 //
CRect WIFI_rect; // 카메라1 출력창을 나타내는 Rect
Rect fr; // 기준위치 // 프레임의 센서선을 나타내기위해
IplImage* display_temp;
		
/////// 통신 관련 /////////
CCom m_pCom;
CCom m_pCom2;
bool is_connecting = false;
bool is_connecting2 = false;
bool sync = false; // 팔 연동 모드인가.

///////////// 자동주행 관련 //////////
int Timer_Auto; // 자동주행 타이머
void CALLBACK TimeProc_Auto(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
bool auto_mode = 0; // 자율주행중인가.


/////// 바퀴 주행관련 //////
stack<course> Stack;     // 스택 컨테이너 정의
course Course;			 // 이동경로를 담는 구조체 정의
int Timernum; // 타이머1
void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
bool image_status = 1; // 센서선 추가할지 여부

//// 모터 회전제어 관련 ///
byte MOTOR7_pos;

//////////////////// define과 전역변수 /////////////////
#define MOTOR_NUM 6 // 팔 모터 갯수
#define PATTERN_SAMPLE 20 // 샘플 갯수

////////// 카메라 및 영상처리관련 //////////
IplImage *pOrgImg;	// drc_WIFI에 의하여 받는 부분
IplImage *pOrgImg_drc3; // drc3.0에 의하여 받는 부분.
IplImage *tempCapture; // 패턴 정의시 띄우는 부분
IplImage *PatternImage[PATTERN_SAMPLE]; // 패턴 지정한 사각형에 해당.
IplImage *PatternDisplayImage; // 지정한 패턴을 표시하는부분
IplImage *PatternResultImage; // 패턴인식 중간결과물.
IplImage *DisplayImage;  // 실제 표시하는부분
int Timernum3; // 타이머3 (영상 프레임을 결정)
HDC dc_display; // 디스플레이 화면을 받는 DC
HDC dc_display2; // 디스플레이 화면2 (Drc3.0)을 받는 DC
bool Receive_Timer = 0; // 1일경우 화면을 받음 (프레임결정)
bool clear = 0; // 화면 갱신하는 변수.

// 경로복귀 //
int Timer_comeback;
byte command = 0;

///////// 패턴인식1 관련 //////////
CvRect rectRoi;		// 실제 패턴의 크기.
CRect m_Rect;
CRect Pattern_rect; // 실제 매칭시 나타나는 노란 사각형
Rect Matching_rect; // 매칭1에서 사용되는 렉트

double minVal = 0.0; // 최저 매칭 점수 (작을수록 정확)
double maxVal = 0.0; // 최고 매칭 점수
double minVal_min = 1.0;

bool matching_start = false; // 패턴 설정시 true로 바뀜. 그러면 매칭 시작.
byte sample_index = 0;

/////////////// DRC WIFI 부분 /////////////////////////
BYTE *pRemain	= NULL;
BYTE gBuf[MAX_FILE_LEN];
char buff[256];
CvFont _font; 
///////////////////////////////////////////////////////

//// DRC 부분 ///
bool startdrc3 = 0;
int Timer_drc3;
IplImage* DisplayImage2;
int CAM2_width = 0;
int CAM2_height = 0;

//////////////// SURF 알고리즘 관련 변수 ///////////////
#define OBJECT_NUM	6 /// 비교 오브젝트 갯수
CvRect rectRoi2;
IpPairVec matches;
IpVec ipts, ref_ipts[OBJECT_NUM];
CvPoint src_corners[OBJECT_NUM][4];
CvPoint dst_corners[OBJECT_NUM][4];
IplImage *Object[OBJECT_NUM]; // SURF알고리즘과 비교할 그림파일
IplImage *Object_Display;
int Object_Width;
int Object_Height;
int Timer_SURF; // SURF타이머 (매칭 주기 결정)

byte Object_index = 0; // 현재 추가된 오브젝트의 갯수
byte matched_index = 0; // 최초 매칭된 인덱스

bool matching_start2 = false; // SURF 알고리즘 사용 여부
bool SURF_start = false;
///////////////////////////////////////////////////////


// 한바이트의 데이터를 보내는 함수
static void Wait(DWORD dwMillisecond) // 밀리세컨드 대기 함수.
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
// 개별 팔제어 명령
static void send(byte data1, byte data2) 
{
	timeKillEvent(Timernum);
	unsigned char send[2] = {data1 + 11, data2}; // 11번 모터부터 시작
	m_pCom.WriteCommBlock(&send, 2);
	Timernum = timeSetEvent(KEYREPEAT, 0, &TimeProc, NULL, TIME_PERIODIC);
}
// 바퀴 각도제어 명령
static void send_angle(byte data1, byte data2) 
{
	unsigned char send[2] = {data1, data2}; // 7번 모터부터 시작
	m_pCom.WriteCommBlock(&send, 2);
}

static void send2(BYTE data) // 보드2에 대해 보냄.
{
	timeKillEvent(Timernum);
	char send = data;
	m_pCom2.WriteCommBlock(&send, 1);
	Timernum = timeSetEvent(KEYREPEAT, 0, &TimeProc, NULL, TIME_PERIODIC);
}

static void send2(byte data1, byte data2) // 보드2 개별 팔제어 명령
{
	unsigned char send[2] = {data1 + 10, data2}; // 10번 모터부터 시작
	m_pCom2.WriteCommBlock(&send, 2);
}

static void setspeed(byte data) // 팔제어 이동속도 설정
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


// CControllerDlg 메시지 처리기

BOOL CControllerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	SetTimer(5, 50, NULL);

	fr.x = 780; fr.y = 470; fr.width = 150; fr.height = 200;

	Init();
	Timernum3 = timeSetEvent(100, 0, &TimeProc, NULL, TIME_PERIODIC); // 50ms마다 WIFI프레임 갱신.
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	
	// DRC초기화 ///
	dc_display2 = GetDlgItem(IDC_PIC_CAM2)->GetDC()->m_hDC; // 디스플레이 2 (drc3.0의 dc를 얻음)
	/// DRC WIFI 초기화 ///////////////
	
	dc_display = GetDlgItem(IDC_PIC_CAM)->GetDC()->m_hDC;  // 디스플레이의 DC를 얻음.
	DRC_ProgramInit();// DRC API관련 자료구조 초기화 
	// 영상출력을 위한 Mat생성 
	pOrgImg = cvCreateImage(cvSize(160,120), IPL_DEPTH_8U, 3);
	cvSet(pOrgImg, cvScalarAll(1),NULL); 	
	DisplayImage = cvCreateImage(cvSize(m_nWidthDisplay, m_nHeightDisplay), IPL_DEPTH_8U, 3);
	cvJpegMat 	= cvCreateMat(1, MAX_FILE_LEN, CV_8UC1 );	
	FlagNewImg	= 0;
	SetTimer(3,100,NULL);// 50ms 마다 전송
	Drc_Rxmode = 0;

	//각종 변수의 초기화 
	IMG_W = m_nWidthDisplay; // 이미지 가로
	IMG_H = m_nHeightDisplay; // 이미지 세로

	cvInitFont(&_font, CV_FONT_HERSHEY_DUPLEX , .2, 0.5, 0, 1, 32);


	///////////////////////////////////////////////



	/////////////////////////////////////////////// */

	memset(focusing, 0, sizeof(bool) * MOTOR_NUM);   // 모터갯수 변경시 바꿀부분
	
	SetDlgItemText(IDC_SPEED, _T("15"));
	SetDlgItemText(IDC_DELAY, _T("600"));

	////////////////////// 진행경로 인식 부분 /////////////

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

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}


void CControllerDlg::OnPaint()
{
	CPaintDC dc(this);
	//// 프레임 그림 띄우는 부분 ///

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
	
	//// 센싱선 그리는부분 ////////
	if(clear)
	{
		CPen pen;
		pen.CreatePen( PS_SOLID, 3, RGB(255,0,0));// 빨간펜
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

	//// DRC_WIFI로부터 영상받고 영상처리하는 부분 /////////
	if(startdrc3)
	{
		
		CvvImage cv_processed2;	
		cvResize(pOrgImg_drc3, DisplayImage2);
			
	
		// 디스플레이 이미지 출력 //   
		cv_processed2.CopyOf(DisplayImage2, 1);
		
		cv_processed2.Show(dc_display2, 0, 0, CAM2_width, CAM2_height);
	}
	
	if(FlagNewImg)
	{
		
		///////////////// 패턴인식부분 /////////////
		if(matching_start)
		{
			minVal_min = 1.0; // 초기값 1
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
					Pattern_rect.left = matchLoc.x; // 매칭을 나타낼 Rect 설정.
					Pattern_rect.top = matchLoc.y;
					Pattern_rect.right = matchLoc.x + PatternImage[i]->width;
					Pattern_rect.bottom = matchLoc.y + PatternImage[i]->height;
					minVal_min = minVal; // 최소값 갱신
				}
				cvReleaseImage(&PatternResultImage);
			}
		}	
		/////////////////////////////////////////// 
		
		CvvImage cv_processed;	
		FlagNewImg = 0; // 재진입 방지 
		cvReleaseImage(&pOrgImg);
		pOrgImg = cvDecodeImage(cvJpegMat, 1);
		cvFlip(pOrgImg, pOrgImg, -1);
		cvResize(pOrgImg, DisplayImage);
		
		//ImageProcess(pOrgImg);
		ImageProcess(DisplayImage); // 필요시 영상처리를 수행한다.
	
	
		// 디스플레이 이미지 출력 //   
		cv_processed.CopyOf(DisplayImage, 1);
		//cv_processed.CopyOf(pOrgImg, 1);
		
		cv_processed.Show(dc_display, 0, 0, m_nWidthDisplay, m_nHeightDisplay);

	}
  
	CDialogEx::OnPaint();
}

byte missing_type = 0; // 매칭을 잃었을때 형식
int old_pos = 0; // 기존 사각형중앙 
int old_pos2 = 0;
int old_pos_surf = 0;
int old_pos_surf2 = 0;
void CControllerDlg::ImageProcess(IplImage *img){
	int SX = 5;
	int SY = 20;
	int LINEH = 18;
	// 카메라로 부터 받은 영상의 길이를 출력해 본다 
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
			Addstring1(_T("목표물 발견"));
		missing_type = 0; // missing이 아님
	}
	else if (matching_start)
	{
		if(old_pos)
		{
			if(old_pos > img->width*3/5)
			{
				if(direction == 3 || direction == 1 || direction == 4) //좌회전중이거나 전진중었으면
					missing_type = 3; // 우측으로 사라졌다.
				else if (direction == 2) // 후진중이었으면
					missing_type = 5; // 거리 로스
				else if(direction == 0) // 아무것도 안하고 있었으면
					missing_type = 3; // 물체가 이동함
			}
			else if (old_pos < img->width*2/5)
			{
				if(direction == 4 || direction == 1 || direction == 3) //우회전중이거나 전진중었으면
					missing_type = 2; // 좌측으로 사라졌다.		
				else if (direction == 2) // 후진중이었으면
					missing_type = 5; // 거리 로스
				else if(direction == 0) // 아무것도 안하고 있었으면
					missing_type = 2; // 물체가 이동함
			}
			else
			{
				if(direction == 2) // 후진중이었으면
					missing_type = 5; // 거리 로스
				else if(direction == 1)
					missing_type = 6; // 물체를 통과함
				else if(direction == 0)
					missing_type = 4;
				else
				{
					if(old_pos2 > img->height/2)
						missing_type = 6; // 카메라 아래로 사라짐 (물체 통과와 동일한 취급)
 					else
						missing_type = 5; // 거리 로스 (카메라 위로 사라짐)
				}
			}
			switch(missing_type)
			{
				case 2: Addstring1(_T("목표물이 좌측으로 사라짐")); break;
				case 3: Addstring1(_T("목표물이 우측으로 사라짐")); break;
				case 4: Addstring1(_T("목표물이 이동함")); break;
				case 5: Addstring1(_T("목표물과 거리가 멀어짐")); break;
				case 6: Addstring1(_T("목표물을 통과함")); break;
				default: break;
			}
			old_pos = 0; // 재진입 방지
		}
	}
	if (matching_start2) // SURF 알고리즘
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

	if (SURF_start) // 표시하는 함수.
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
				if(direction == 3 || direction == 1 || direction == 4) //좌회전중이거나 전진중었으면
					missing_type = 3; // 우측으로 사라졌다.
				else if (direction == 2) // 후진중이었으면
					missing_type = 5; // 거리 로스
				else if(direction == 0) // 아무것도 안하고 있었으면
					missing_type = 3; // 물체가 이동함
			}
			else if (old_pos_surf < img->width*2/5)
			{
				if(direction == 4 || direction == 1 || direction == 3) //우회전중이거나 전진중었으면
					missing_type = 2; // 좌측으로 사라졌다.		
				else if (direction == 2) // 후진중이었으면
					missing_type = 5; // 거리 로스
				else if(direction == 0) // 아무것도 안하고 있었으면
					missing_type = 2; // 물체가 이동함
			}
			else
			{
				if(direction == 2) // 후진중이었으면
					missing_type = 5; // 거리 로스
				else if(direction == 1)
					missing_type = 6; // 물체를 통과함
				else if(direction == 0)
					missing_type = 4;
				else
				{
					if(old_pos_surf2 > img->height/2)
						missing_type = 6; // 카메라 아래로 사라짐 (물체 통과와 동일한 취급)
 					else
						missing_type = 5; // 거리 로스 (카메라 위로 사라짐)
				}
			}
			switch(missing_type)
			{
				case 2: Addstring1(_T("목표물이 좌측으로 사라짐")); break;
				case 3: Addstring1(_T("목표물이 우측으로 사라짐")); break;
				case 4: Addstring1(_T("목표물이 이동함")); break;
				case 5: Addstring1(_T("목표물과 거리가 멀어짐")); break;
				case 6: Addstring1(_T("목표물을 통과함")); break;
				default: break;
			}
			old_pos_surf = 0; // 재진입 방지
		}
	}
	if(minVal_min < 0.03)
		cvRectangle(img, cvPoint(Matching_rect.x, Matching_rect.y), cvPoint(Matching_rect.x + Matching_rect.width, Matching_rect.y +Matching_rect.height), cvScalar(0,255,0), 2);
	
	// 화면 반전


	drawFPS(img);
/*
	sprintf(buff, "ImgSize= %d B", ImgSize); // MakeUartTxData
	cvPutText(img, buff, cvPoint(SX,SY+LINEH*2), &_font, CV_RGB(0, 250, 0));
	
	// 카메라로 부터 받은 영상의 크기를 출력해 본다 
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
	if(nIDEvent == 1) // 0번 카운터. DRC로부터 영상을 받음.
	{
		
		COPYDATASTRUCT msg;
		PORT_ADD addr;
		HANDLE handle = AfxGetInstanceHandle();//자신의 인스턴스 획득
		CWnd *pWnd = AfxGetMainWnd();//자신의 핸들 획득
		addr.nWnd = pWnd->m_hWnd;
	//	memcpy(addr.val , drcdata, sizeof(drcdata));
		msg.cbData = sizeof(addr);
		msg.dwData = UM_MOUSESTATUS;
		msg.lpData = (PVOID)&addr;
	
	
		//sendMessage 를 이용하여 DRC Black Manager 와의 통신을 위한 호출
		::SendMessage(hDRC, WM_COPYDATA, (WPARAM)handle,(LPARAM)&msg);

	//	UpdateData(FALSE);

	}

	else if (nIDEvent == 2) // 팔 연동 타이머
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
			Drc_WaitCnt++; // 자동증가 ...영상수신시 0으로 리셋 

			if(Drc_WaitCnt>20 && Drc_Retry_flag==0)
			{ // 일정시간 이상 영상수신이 안되면 다시 요청 			
				DRC_RequestImage();
				Drc_Retry_flag = 1;
			}
		}
		if(Drc_ConnState==1&&flag==0) 
		{
			flag = 1;
			GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);// 버튼 활성화 	
		}
	}
	
	else if (nIDEvent == 5)
	{
		if(!sync)
		{
			if(GetAsyncKeyState(0x44)) // a키
			{
				int a = GetDlgItemInt(IDC_EDIT1);
				send(0, a-16);
			}
			else if(GetAsyncKeyState(0x41)) // d키
			{
				int a = GetDlgItemInt(IDC_EDIT1);
				send(0, a+16);
			}
			else if(GetAsyncKeyState(0x58)) // w키
			{
				int a = GetDlgItemInt(IDC_EDIT3);
				send(4, a-5);
			}
			else if(GetAsyncKeyState(0x54)) // s키
			{
				int a = GetDlgItemInt(IDC_EDIT3);
				send(4, a+5);
			}
		}
	}
	// 일반적으로 //

	
	
	CDialogEx::OnTimer(nIDEvent);
}
	
HCURSOR CControllerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CControllerDlg::OnBnClickedStartcom()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if(!is_connecting)			// 통신이 연결중이 아닐 때
	{
		CString strTmp;			// 임시 문자열 선언
		SelectComDlg dlg;		// COM포트번호를 선택하는 Dialog 선언.
		dlg.DoModal();			// Dialog를 띄움
		strTmp = dlg.m_selected;   // Dialog의 콤보박스 내에서 선택한 COM포트 번호를 임시 문자열로 옮김
		m_pCom.SetComPort(strTmp, 57600, 8, 0, 0);  // 프로토콜 설정. COM포트는 Dialog에서 선택한 값
													   // Baudrate 115200
													   // 전송 비트 8비트
													   // Stop비트 1개 설정.
													   // 패리티 없음
		if(!m_pCom.OpenComPort())					   // COM포트를 연다.
			AfxMessageBox(_T("통신 연결에 실패했습니다."));  // 포트를 여는데 실패할 경우 메세지를 띄운다.
		else												// 포트를 여는데 성공하면
		{
			m_pCom.SetHwnd(this->m_hWnd);					// 이 클래스(Mainframe)의 윈도우 핸들을 받는다.
			is_connecting = true;							// 연결이 된 상태로 바꿈.
//			send(220);
			Addstring1(_T("(알림)블루투스 연결 완료"));
		}
	}
	else					// 이미 통신중이었다면
		AfxMessageBox(_T("이미 연결되어 있습니다!"));	// 메세지 띄움.
}
void CControllerDlg::OnBnClickedStartcom2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if(!is_connecting2)			// 통신이 연결중이 아닐 때
	{
		CString strTmp;			// 임시 문자열 선언
		SelectComDlg dlg;		// COM포트번호를 선택하는 Dialog 선언.
		dlg.DoModal();			// Dialog를 띄움
		strTmp = dlg.m_selected;   // Dialog의 콤보박스 내에서 선택한 COM포트 번호를 임시 문자열로 옮김
		m_pCom2.SetComPort(strTmp, 57600, 8, 0, 0);  // 프로토콜 설정. COM포트는 Dialog에서 선택한 값
													   // Baudrate 115200
													   // 전송 비트 8비트
													   // Stop비트 1개 설정.
													   // 패리티 없음
		if(!m_pCom2.OpenComPort())					   // COM포트를 연다.
			AfxMessageBox(_T("통신 연결에 실패했습니다."));  // 포트를 여는데 실패할 경우 메세지를 띄운다.
		else												// 포트를 여는데 성공하면
		{
			m_pCom2.SetHwnd(this->m_hWnd);					// 이 클래스(Mainframe)의 윈도우 핸들을 받는다.
			is_connecting2 = true;							// 연결이 된 상태로 바꿈.
			Addstring1(_T("(알림)컨트롤러 연결 완료"));
		}
	}
	else					// 이미 통신중이었다면
		AfxMessageBox(_T("이미 연결되어 있습니다!"));	// 메세지 띄움.
}

void CControllerDlg::OnBnClickedDisconnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if(is_connecting)					// 연결이 되어있던 상태라면
	{
		m_pCom.CloseConnection();		// 통신포트를 닫는다. 
		is_connecting = false;
		Addstring1(_T("(알림)연결 끊김"));
	}
	else
		Addstring1(_T("(알림)연결중이 아님"));
}


void CControllerDlg::OnBnClickedDisconnect2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if(is_connecting2)					// 연결이 되어있던 상태라면
	{
		m_pCom2.CloseConnection();		// 통신포트를 닫는다. 
		is_connecting2 = false;
		Addstring1(_T("(알림)연결 끊김"));
	}
	else
		Addstring1(_T("(알림)연결중이 아님"));
}


// DYNAMIXEL로부터 받은 데이터들을 출력해주는 함수.
afx_msg LRESULT CControllerDlg::OnReceive(WPARAM wParam, LPARAM lParam) // 모터 갯수에 따라 달라짐
{
	static int count = 0;
	unsigned short i;			  // 반복 인덱스
	CString str;
	for(i=0;i<wParam;i++)		  // wParam은 한번에 수신한 바이트 수로 추정됨. 그 횟수만큼 반복
	{
		if((unsigned char)rx_buffer[i] == 0xFF) count = 0; // 헤더가 올경우 카운트 시작
		
		if(!sync) // 싱크모드가 아닐딘경우 메인보드의 모터값을 받음.
		{
			switch(count)
			{
			case 1:
				Sonic[0] = (byte)rx_buffer[i];
				str.Format(_T("%dcm"), Sonic[0]); // 평균값을 표시함
				SetDlgItemText(IDC_SONIC1, str);
				break;
			case 2:
				Sonic[1] = (byte)rx_buffer[i] - 3;
				str.Format(_T("%dcm"), Sonic[1]); // 평균값을 표시함
				SetDlgItemText(IDC_SONIC2, str);
				break;
			case 3:
				Sonic[2] = (byte)rx_buffer[i] - 3;
				str.Format(_T("%dcm"), Sonic[2]); // 평균값을 표시함
				SetDlgItemText(IDC_SONIC3, str);
				InvalidateRect(CRect(fr.x, fr.y - 20, fr.x+30, fr.y),TRUE); // 갱신
				InvalidateRect(CRect(fr.x + fr.width - 30, fr.y - 20, fr.x+fr.width, fr.y),TRUE);
				InvalidateRect(CRect(fr.x, fr.y +fr.height, fr.x+30, fr.y + fr.height + 20),TRUE);
				InvalidateRect(CRect(fr.x + fr.width - 30, fr.y + fr.height, fr.x+fr.width, fr.y + fr.height + 20),TRUE);
				clear = 1;										  // 선 클리어
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
				str.Format(_T("%dcm"), Sonic[3]); // 평균값을 표시함
				SetDlgItemText(IDC_SONIC4, str);
				break;
			case 16:
				Sonic[4] = (byte)rx_buffer[i] - 3;
				str.Format(_T("%dcm"), Sonic[4]); // 평균값을 표시함
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
		} // 메인보드에 대한 수신.

		else //싱크모드
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
		} // 모형팔에 대한 수신.
		count++;
		
	}
	UpdateData(FALSE);

	return 0;
}


// 개별적으로 토크 ON/OFF하는 함수.

void CControllerDlg::OnBnClickedTorqueon() { send(250); }
void CControllerDlg::OnBnClickedTorqueoff(){ send(251); }


void CControllerDlg::OnBnClickedAddlist() // 모터 갯수에 따라 변동.
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
		AfxMessageBox(_T("숫자 범위를 벗어났거나 잘못된 값이 있습니다"));
	else
	{
		CString str;
		str.Format(_T("%03d %03d %03d %03d %03d %03d SPEED:%03d DELAY:%d"), a1, a2, a3, a4, a5, a6, speed, delay); // 오른쪽정렬 0으로채움
	
		m_List.InsertString(m_List.GetCount(),str);
		m_List.SetCurSel(m_List.GetCount() - 1);
	}
}

void CControllerDlg::OnBnClickedErase(){ m_List.ResetContent(); }
void CControllerDlg::OnBnClickedDelete(){ m_List.DeleteString(m_List.GetCurSel()); } 
// GetCurSel로 선택한부분의 번호를 받아 지움.







void CControllerDlg::OnBnClickedSimulation()
{
	int index = m_List.GetCount();
	unsigned int Motor1[20], Motor2[20], Motor3[20], Motor4[20], Motor5[20], Motor6[20], speed[20], delay[20];
	CString str;
	
	for(int i=0; i<index; i++)
	{
		m_List.GetText(i, str);
		Motor1[i] = _ttoi(str.Left(3));
		str.Delete(0, 4); // 앞에서 4칸 삭제.
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


		setspeed(speed[i]); // 속도 설정

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
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	Drc_RetrieveFrame(hDRC, pCopyDataStruct, pOrgImg_drc3);// 현 순간마다 pOrgImg 영상이 갱신  
	cvFlip(pOrgImg_drc3, pOrgImg_drc3, -1);
	InvalidateRect(WIFI_rect,FALSE);
	return CDialogEx::OnCopyData(pWnd, pCopyDataStruct);
}



void Pushing(unsigned char dir) // 편리하게 스택에 넣는 함수.
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
			case 242: c.Format(_T("전진"));	break;
			case 243: c.Format(_T("후진"));	break;
			case 244: c.Format(_T("좌회전"));	break;
			case 245: c.Format(_T("우회전"));	break;
			case 246: c.Format(_T("정지"));	break;
			default: break;
			}
			Liststring2.Format(_T("%4dms  %4s  저장 : %d"), Course.time*25, c, Course.saved);
			Addstring2(Liststring2); 
		}
		Course.time = 0;
		Course.direction = 242;
		Course.saved = 0;
		
	}
	direction = 1; //전진
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
			case 242: c.Format(_T("전진"));	break;
			case 243: c.Format(_T("후진"));	break;
			case 244: c.Format(_T("좌회전"));	break;
			case 245: c.Format(_T("우회전"));	break;
			case 246: c.Format(_T("정지"));	break;
			default: break;
			}
			Liststring2.Format(_T("%4dms  %4s  저장 : %d"), Course.time*25, c, Course.saved);
			Addstring2(Liststring2); 
		}
		Course.time = 0;
		Course.direction = 242;
		Course.saved = 0;
		
	}
	direction = 1; //전진
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
			case 242: c.Format(_T("전진"));	break;
			case 243: c.Format(_T("후진"));	break;
			case 244: c.Format(_T("좌회전"));	break;
			case 245: c.Format(_T("우회전"));	break;
			case 246: c.Format(_T("정지"));	break;
			default: break;
			}
			Liststring2.Format(_T("%4dms  %4s  저장 : %d"), Course.time*25, c, Course.saved);
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
			case 242: c.Format(_T("전진"));	break;
			case 243: c.Format(_T("후진"));	break;
			case 244: c.Format(_T("좌회전"));	break;
			case 245: c.Format(_T("우회전"));	break;
			case 246: c.Format(_T("정지"));	break;
			default: break;
			}
			Liststring2.Format(_T("%4dms  %4s  저장 : %d"), Course.time*25, c, Course.saved);
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
			case 242: c.Format(_T("전진"));	break;
			case 243: c.Format(_T("후진"));	break;
			case 244: c.Format(_T("좌회전"));	break;
			case 245: c.Format(_T("우회전"));	break;
			case 246: c.Format(_T("정지"));	break;
			default: break;
			}
			Liststring2.Format(_T("%4dms  %4s  저장 : %d"), Course.time*25, c, Course.saved);
			Addstring2(Liststring2);
		} 
		Course.time = 0;
		Course.direction = 245;
		
		Course.saved = 0;
	}
	send(245); // 앵글 리셋
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
		case 242: c.Format(_T("전진"));	break;
		case 243: c.Format(_T("후진"));	break;
		case 244: c.Format(_T("좌회전"));	break;
		case 245: c.Format(_T("우회전"));	break;
		case 246: c.Format(_T("정지"));	break;
		default: break;
		}
		Liststring2.Format(_T("%4dms  %4s  저장 : %d"), Course.time*25, c, Course.saved);
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
			else if (GetAsyncKeyState(VK_UP))   // 상
				Go_Forward();
			else if (GetAsyncKeyState(VK_DOWN)) // 하
				Go_Back();
			else if (GetAsyncKeyState(0x55)) // 하
				send(247);
			else				// 아무키도 안눌린상태
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



void CControllerDlg::OnBnClickedSavecourse()    // 경로저장 함수
{
	if(Stack.empty())
	{
		Addstring1(_T("(알림)스택이 비어있어서 저장할 수 없음"));
		return;
	}
	Addstring1(_T("(알림)경로 저장"));
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

void CControllerDlg::OnBnClickedComeback()     // 경로복귀 함수
{
	
	
	timeKillEvent(Timernum);    // 이동명령을 받는 타이머를 죽임.
	Timer_comeback = timeSetEvent(25, 0, TimeProc, NULL, TIME_PERIODIC);
	while(1)
	{
		if(Stack.empty())
		{
			Addstring1(_T("(알림)리스트가 비어있어 복귀할 수 없음"));
			break;
		}
		
		course temp = Stack.top();
		if(temp.saved)
		{
			temp.saved = 0; // 저장되있을경우 저장값을 0으로 만들고 다시 집어넣음
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
	cvSetMouseCallback("PatternDefine", OnMousePatternDefine, this); // cv식 콜백.
}

void CControllerDlg::OnMousePatternDefine(int iEvent, int iX, int iY, int iFlags, void* Userdata)
{
	CControllerDlg* pCControllerDlg = (CControllerDlg*)Userdata;
	switch(iEvent)
	{
	case EVENT_MOUSEMOVE:
		{
		}break;
	case EVENT_LBUTTONDOWN: // 누른시점의 좌표와
		{
			m_Rect.left = iX;
			m_Rect.top = iY;                 // rect 범위 정해줌.
		}break;
	case EVENT_LBUTTONUP: // 뗀시점의 좌표로 Rect를 만듬.
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
		cvResize(tempCapture, PatternImage[sample_index]); // 리사이즈
		cvDestroyWindow("PatternDefine"); //윈도우 끝낼떄.
		Invalidate(FALSE);	
		
		if(sample_index == 0)
			matching_start = true; // 매칭1시작
		CvvImage cv_pattern;	
		cvResize(PatternImage[sample_index], PatternDisplayImage);
		HDC dc_pattern = GetDlgItem(IDC_PIC_PATTERN_TARGET)->GetDC()->m_hDC;        
		cv_pattern.CopyOf(PatternDisplayImage, 1);
		cv_pattern.Show(dc_pattern, 0, 0, m_nWidthPatternDisplay, m_nHeightPatternDisplay);

		sample_index++;
		SetDlgItemInt(IDC_SAMPLE2, sample_index); // 샘플 갯수 증가
	}
	else
		AfxMessageBox(_T("샘플 갯수를 초과하였습니다."));

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
	m_nWidthDisplay = rect.right;		  // 디스플레이의 길이 설정
	m_nHeightDisplay = rect.bottom;	      // 디스플레이의 높이 설정
	GetDlgItem(IDC_PIC_PATTERN_TARGET)->GetClientRect(&rect);
	m_nWidthPatternDisplay = rect.right;
	m_nHeightPatternDisplay = rect.bottom;
	PatternDisplayImage = cvCreateImage(cvSize(m_nWidthPatternDisplay, m_nHeightPatternDisplay), IPL_DEPTH_8U, 3);
	
	GetDlgItem(IDC_PIC_PATTERN2)->GetClientRect(&rect); // 오브젝트 표시에 대한 사각형 정의
	Object_Width = rect.right;
	Object_Height = rect.bottom;
	Object_Display = cvCreateImage(cvSize(Object_Width, Object_Height), IPL_DEPTH_8U, 3);
	
	
	cvSubS(PatternDisplayImage, cvScalar(255,255,255), PatternDisplayImage);
	for(int i=0; i<OBJECT_NUM; i++)
		Object[i] = cvCreateImage(cvSize(5, 5), IPL_DEPTH_8U, 3); // 일종의 초기화
	for(int i=0; i<PATTERN_SAMPLE; i++)
		PatternImage[i] = cvCreateImage(cvSize(5,5), IPL_DEPTH_8U, 3);

	GetDlgItem(IDC_PIC_CAM2)->GetWindowRect(WIFI_rect);
	ScreenToClient(WIFI_rect);
}



void CControllerDlg::OnBnClickedButton1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	DRC_RequestImage(); // 카메라에게 영상 요청 
	StartButtonClick = 1;
}



void CControllerDlg::OnBnClickedArmSync() // 팔 동기화
{
	if(!sync) // 연동버튼 누르면
	{
		SetDlgItemText(IDC_ARM_SYNC, _T("연동 해제"));
		sync = true;
		SetTimer(2, 35, NULL);
		send(240); // 연동시작
		send2(240);
	}
	else
	{
		SetDlgItemText(IDC_ARM_SYNC, _T("컨트롤러 연동"));
		KillTimer(2);
		sync = false;
		send(241); // 연동중지
		send2(241);
		send(0, m_miniarmpos[0]);
		send(1, m_miniarmpos[1] + 13);
		send(2, m_miniarmpos[2]);
		send(3, m_miniarmpos[3]);
		send(4, m_miniarmpos[4] - 15);
		send(5, m_miniarmpos[5] + 5);

	}
}



void CControllerDlg::OnBnClickedSurf() // SURF 패턴정의 함수.
{
	tempCapture = cvCloneImage(pOrgImg);
	cvShowImage("PatternDefine", tempCapture);
	cvSetMouseCallback("PatternDefine", OnMousePatternDefine, this); // cv식 콜백.
	
}


void CControllerDlg::OnBnClickedSurfStop()
{
//	for(int i=0; i<Object_index; i++)
//		cvReleaseImage(&Object[i]);
	Object_index = 0; // 인덱스 0됨
	SetDlgItemInt(IDC_SAMPLE, Object_index);
	timeKillEvent(Timer_SURF); // 타이머죽임
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

// 카메라에게 영상을 요청함.
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
		int SenRegCnt 	= 3; //주의 
		pSen = (char *)malloc(3*SenRegCnt); 	
		pSen[S+0] = GROUP_A; 	pSen[S+1] = 0x6A; 	pSen[S+2] = 0x87; 	S += 3;
		pSen[S+0] = GROUP_A;	pSen[S+1] = 0x6A;	pSen[S+2] = 0x47;	S += 3;
		pSen[S+0] = GROUP_A;	pSen[S+1] = 0x6A;	pSen[S+2] = 0x87;	S += 3;
		}


	Drc_Cmd.CmdType		= TYPE_GET_REALTIME_IMG;
	Drc_Cmd.CmdDataLen	= sizeof(CMD_TYPE_IMG)+S;
	// DRC-WIFI 에 전송할 UART TX 의 크기를 25byte 로 임의 설정(Max 100byte)
	Drc_Cmd.UartTxLen	= UartTxDataLen;
	Drc_Cmd.TotalLen	= sizeof(DRC_TX_PKT) + Drc_Cmd.CmdDataLen + Drc_Cmd.UartTxLen;

	CMD_TYPE_IMG typeImg;
	memset(&typeImg, 0 , sizeof(CMD_TYPE_IMG));
	
	// 수신 받기원하는 영상의 크기를 설정(Max 640x480)

	///////////////// 영상크기설정 320x240, 640x480중 택1 ////////
	typeImg.aWidth		= 320/8;
	typeImg.aHeight 	= 240/8;	
	////////////////////////////////////////////////////////
	
	// 영상의 이미지 퀄리티를 설정 (1~0)
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


// DRC API관련 자료구조 초기화 
void CControllerDlg::DRC_ProgramInit(){

	// socket programming initA
	if (!AfxSocketInit())
		{
			AfxMessageBox(_T("AfxSocketInit FAIL"));
			OnOK();
		}
	Drc_Retry_flag = 0;
	Drc_ServerSocket.SetParent(this);

	if(!Drc_ServerSocket.Create()){// 소켓을 생성
		int iErr = GetLastError(); 
		printf("1 iErr %d\n",iErr);
		}
	
	CString mIPaddr	= _T("192.168.123.10"); 
	int m_iPort		= 6400;
	if(!Drc_ServerSocket.Connect(mIPaddr, m_iPort)){// 지정한 IP주소와 포트번호에 연결
		int iErr = GetLastError();  
		}
	// 송수신 패킷 자료구조 초기화 
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
		cvResize(tempCapture, Object[Object_index]); // 리사이즈
		cvDestroyWindow("PatternDefine"); //윈도우 끝낼떄.

		
		if(Object_index == 0)
		{
			SURF_start = 1;
			Timer_SURF = timeSetEvent(300, 0, &TimeProc, NULL, TIME_PERIODIC); // 0.3초마다
		}
		src_corners[Object_index][0] = cvPoint(0,0);
		src_corners[Object_index][1] = cvPoint(Object[Object_index]->width,0);
		src_corners[Object_index][2] = cvPoint(Object[Object_index]->width, Object[Object_index]->height);
		src_corners[Object_index][3] = cvPoint(0, Object[Object_index]->height);
		surfDetDes(Object[Object_index], ref_ipts[Object_index], false, 10, 10, 2, 0.000001f); // 이부분을 조정해 정밀도 바꾸기
		drawIpoints(Object[Object_index], ref_ipts[Object_index]);

		
		// 패턴이미지 출력
		CvvImage SURF_pattern;	
		cvResize(Object[Object_index], Object_Display);
		HDC dc_pattern2 = GetDlgItem(IDC_PIC_PATTERN2)->GetDC()->m_hDC;        
		SURF_pattern.CopyOf(Object_Display, 1);
		SURF_pattern.Show(dc_pattern2, 0, 0, Object_Width, Object_Height);
		Object_index++;
		SetDlgItemInt(IDC_SAMPLE, Object_index);
	}
	else
		AfxMessageBox(_T("샘플 갯수를 초과하였습니다."));
}


void CControllerDlg::OnBnClickedReceiveimage()
{
	
	if(!startdrc3) // 연동버튼 누르면
	{
		SetDlgItemText(IDC_RECEIVEIMAGE, _T("DRC 수신정지"));
		SetTimer(1,100,NULL);
		startdrc3 = 1;
	
		hDRC = Drc_CaptureFromCAM(drcdata);
		pOrgImg_drc3 = cvCreateImage(cvSize(160, 120), IPL_DEPTH_8U, 3);//영상 이미지 생성
		DisplayImage2 = cvCreateImage(cvSize(CAM2_width, CAM2_height), IPL_DEPTH_8U, 3);
	
		cvSet(pOrgImg, cvScalarAll(1),NULL); 	
	
		drcdata[PREAMP_G_N] = 4;
	
	}
	else
	{
		KillTimer(1);
		startdrc3 = 0;
		SetDlgItemText(IDC_RECEIVEIMAGE, _T("DRC 영상수신"));
	}
}


void CControllerDlg::OnBnClickedMathingInit()
{
	sample_index = 0; // 인덱스 0됨
	SetDlgItemInt(IDC_SAMPLE2, sample_index);
	CvvImage pattern;	
	cvSubS(PatternDisplayImage, cvScalar(255,255,255), PatternDisplayImage);
	HDC dc_pattern = GetDlgItem(IDC_PIC_PATTERN_TARGET)->GetDC()->m_hDC;        
	pattern.CopyOf(PatternDisplayImage, 1);
	pattern.Show(dc_pattern, 0, 0, m_nWidthPatternDisplay, m_nHeightPatternDisplay);
	matching_start = false;
	minVal_min = 1.0;
}


void CControllerDlg::OnBnClickedButtonPatternLoadpattern() // 패턴 불러오기
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
				//// 불러오기 구현부분 (파일명에 \\ 더 추가하기)
				char *file_name = new char[strFileName.GetLength() + 1];; // CString에서 char*로 변환
				strcpy(file_name, CT2A(strFileName));
		
				for(int i=strFileName.GetLength()-1; i>0; i--)
				{
					if (file_name[i] == '\\') // \는 두개로 표현해야함
						strFileName.Insert(i, _T("\\"));  // \추가
				}
				char *file_name2 = new char[strFileName.GetLength() + 1];
				strcpy(file_name2, CT2A(strFileName));
				delete[] file_name;
		
				//////////////////////////////////////////
				cvReleaseImage(&PatternImage[sample_index]);
				PatternImage[sample_index] = cvLoadImage(file_name2, 1);
	
				//////////// 동적할당 해제 //////
				delete[] file_name2;
				////////////////////////////////
				
				if(sample_index == 0)
					matching_start = true; // 매칭1시작

				if(pos == NULL || sample_index == PATTERN_SAMPLE - 1) // 마지막 파일이면
				{
					CvvImage cv_pattern;	
					cvResize(PatternImage[sample_index], PatternDisplayImage);
					HDC dc_pattern = GetDlgItem(IDC_PIC_PATTERN_TARGET)->GetDC()->m_hDC;        
					cv_pattern.CopyOf(PatternDisplayImage, 1);
					cv_pattern.Show(dc_pattern, 0, 0, m_nWidthPatternDisplay, m_nHeightPatternDisplay);
				}
				
				sample_index++;
				SetDlgItemInt(IDC_SAMPLE2, sample_index); // 샘플 갯수 증가
			}
			else
			{
				AfxMessageBox(_T("샘플 갯수를 초과하였습니다."));
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
				//// 불러오기 구현부분 (파일명에 \\ 더 추가하기)
				CString strFileName = fDlg.GetNextPathName(pos);
				char *file_name = new char[strFileName.GetLength() + 1];; // CString에서 char*로 변환
				strcpy(file_name, CT2A(strFileName));
		
				for(int i=strFileName.GetLength()-1; i>0; i--)
				{
					if (file_name[i] == '\\') // \는 두개로 표현해야함
						strFileName.Insert(i, _T("\\"));  // \추가
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
					Timer_SURF = timeSetEvent(300, 0, &TimeProc, NULL, TIME_PERIODIC); // 0.3초마다
				}
				src_corners[Object_index][0] = cvPoint(0,0);
				src_corners[Object_index][1] = cvPoint(Object[Object_index]->width,0);
				src_corners[Object_index][2] = cvPoint(Object[Object_index]->width, Object[Object_index]->height);
				src_corners[Object_index][3] = cvPoint(0, Object[Object_index]->height);
				surfDetDes(Object[Object_index], ref_ipts[Object_index], false, 10, 10, 2, 0.000001f); // 이부분을 조정해 정밀도 바꾸기
				//drawIpoints(Object[Object_index], ref_ipts[Object_index]);
				//I포인트는 그리지 않음
				if(pos == NULL || Object_index == OBJECT_NUM - 1) // 마지막 파일이면
				{
					CvvImage SURF_pattern;	
					cvResize(Object[Object_index], Object_Display);
					HDC dc_pattern2 = GetDlgItem(IDC_PIC_PATTERN2)->GetDC()->m_hDC;        
					SURF_pattern.CopyOf(Object_Display, 1);
					SURF_pattern.Show(dc_pattern2, 0, 0, Object_Width, Object_Height);
				}
				Object_index++;
				SetDlgItemInt(IDC_SAMPLE, Object_index); // 샘플 갯수 증가
			}
			else
			{
				AfxMessageBox(_T("샘플 갯수를 초과하였습니다."));
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
	AfxMessageBox(_T("내 문서에 저장되었습니다."));
}


void CControllerDlg::OnBnClickedSavesample2()
{
	char path[255] = {0};
	for(int i=0; i<Object_index; i++)
	{
		sprintf(path, "C:\\Users\\Administrator\\Documents\\surf_sample_%d.jpg", i+1);
		cvSaveImage((LPCSTR)path, Object[i]);	
	}
	AfxMessageBox(_T("내 문서에 저장되었습니다."));
}



byte mode = 0; // 0은 초기모드, 1은  
void CControllerDlg::OnBnClickedAutomode()
{
	
	if(!auto_mode) // 연동버튼 누르면
	{
		SetDlgItemText(IDC_AUTOMODE, _T("자동주행 해제"));
		Addstring1(_T("(자동주행)시작"));
		Wait(500);
		missing_type = 1;
		auto_mode = true;
		stop(); // 정지
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
		SetDlgItemText(IDC_AUTOMODE, _T("자동주행 모드"));
		Addstring1(_T("(자동주행)해제"));
		auto_mode = false;
		timeKillEvent(Timer_Auto);
		stop();
		mode = 0;
		send(238 - 11, 112);
		m_slider.SetPos(112);
	}
	// 자동주행 종료시 타이머 살려주기
}



#define 전방감지거리 25
#define 스캔시간	60
#define 빈공간감지거리 40
unsigned int t_time1 = 0; // 스캔중 타이머
unsigned int t_time2 = 0; // 회전중 타이머


byte left_timer = 0;
byte right_timer = 0;
byte start_value1 = 0;
byte start_value2 = 0;
byte end_value1 = 0;
byte end_value2 = 0;
bool left_empty = 0;
bool right_empty = 0;
int Waitcount = 0 ;
byte dir = 3; // 주행방향
void CALLBACK TimeProc_Auto(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if(mode == 0) // 기본모드
	{
		if(missing_type == 0 && matching_start)
		{
			if(old_pos < DisplayImage->width*4/5 && old_pos > DisplayImage->width*1/5) // 중앙쯤
			{
				Go_Forward_with_scan();
				if(dir != 0)
				{
					dir = 0;
					Addstring1(_T("(자동주행)목표물이 중앙으로 치우침, 전진"));
				}
			}
			else if(old_pos <= DisplayImage->width*1/5)
			{
				if(dir != 1)
				{
					dir = 1;
					Addstring1(_T("(자동주행)목표물이 카메라 좌측으로 치우침, 좌회전"));
				}
				Turn_Left();
			}
			else if(old_pos >= DisplayImage->width*4/5)
			{
				Turn_Right();
				if(dir != 2)
				{
					dir = 2;
					Addstring1(_T("(자동주행)목표물이 카메라 우측으로 치우침, 우회전"));
				}
			}
		}
		else if(missing_type == 6 && matching_start) // 물체를 지나가면
		{
			Addstring1(_T("(자동주행)목표물을 통과하여 자동주행을 종료함"));
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
				Addstring1(_T("(자동주행)목표물이 우측으로 사라짐, 우회전"));
			}
			Turn_Right();
		}
		else if(missing_type == 2 && matching_start)
		{
			if(dir != 1)
			{
				dir = 1;
				Addstring1(_T("(자동주행)목표물이 좌측으로 사라짐, 좌회전"));
			}
			Turn_Left();
		}
		else if(Sonic[1] <= 전방감지거리  || Sonic[2] <= 전방감지거리 || Sonic[0] <= 40)
		{
			Addstring1(_T("(자동주행)전방에 장애물 발견"));
			t_time1 = 스캔시간;
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
			
			
			// Put(스캔모드)
		}
		
		else
			Go_Forward_with_scan(); // 전진
	}
	else if(mode == 1) // 스캔모드
	{
#define 빈공간측정시간	20 // 1초
		if(t_time1 > 0)
		{	
			t_time1--;

			if(Sonic[1] >= 빈공간감지거리)
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
				left_timer = 빈공간측정시간;
				start_value1 = 0;
				left_empty = 0;
			}
			if(Sonic[2] >= 빈공간감지거리)
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
				right_timer = 빈공간측정시간;
				right_empty = 0;
			}
			
			if(left_timer == 0)
			{
				end_value1 = t_time1;
				Addstring1(_T("(자동주행)좌측에 빈 공간 발견"));
				
				stop();
				left_empty = 1;
				right_empty = 0;
				t_time1 = 0;
				t_time2 = (스캔시간 - (start_value1 + end_value1) / 2) / 2 + 10; // +5는 바퀴가 회전하는 시간 보정
				
				Waitcount = 20; 
				mode = 2; // 대기모드 1
				
				return;
				// 좌측에 빈공간 발견
			}
			if(right_timer == 0)
			{
				end_value2 = t_time1;
				Addstring1(_T("(자동주행)우측에 빈 공간 발견"));
				stop();
				Waitcount = 20; 
				
				left_empty = 0;
				right_empty = 1;
				t_time1 = 0; // 최적
				t_time2 = (스캔시간 - (start_value1 + end_value1) / 2) / 2 + 10; // +5는 바퀴가 회전하는 시간 보정
				mode = 2; // 회전모드
				
				return;
			}
		}
		else// 스캔 실패의 경우 (t_time == 0)
		{
			Addstring1(_T("(자동주행)양쪽 모두 빈공간을 발견하지 못함"));
				
			mode = 2;
			stop();
			if(Sonic[1] - Sonic[2] > 0) // 오른쪽 벽이 더 가까운경우
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
				Addstring1(_T("(자동주행)왼쪽으로 회전명령"));
			else if (right_empty == 1)
				Addstring1(_T("(자동주행)오른쪽으로 회전명령"));
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
				Addstring1(_T("(자동주행)열린길 발견. 회전중지"));
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
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
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
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	send(238 - 11, m_slider.GetPos());
	SetDlgItemInt(IDC_ATTRIBUTE, m_slider.GetPos());
	*pResult = 0;
}
