#include <stack>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include "afxcmn.h"
#include "afxwin.h"

using namespace std;
using namespace cv;

#define TYPE_GET_REALTIME_IMG		1
#define MAX_FILE_LEN	(1*1024*1024)
#define M_WAIT_HEADER0	0
#define M_WAIT_HEADER1	1
#define M_WAIT_HEADER2	2
#define M_GET_INFO		10
#define M_GET_REMAIN	20
typedef signed char 	s8; typedef unsigned char 	u8; typedef unsigned short 	u16; typedef signed short 	s16; typedef unsigned int 	u32;
#pragma pack(1)

// DRC-WIFI관련 구조체 선언부분 ///
typedef struct {u8 aWidth;u8	aHeight;u8 	Quality;u8	year;u8	month;u8 day;u8	hour;u8	min;u8	sec;u8	msec;u8	aRecord;}CMD_TYPE_IMG;
typedef struct {u8 HEAD[3];u32 TotalLen;u16 CmdType;u32 CmdDataLen;u32 UartTxLen;}DRC_TX_PKT;
typedef struct {u8 iWidth;u8 iHeight;u32 iOffset;u32 iSize;u32 iJpegLen;u8 iVersion;u8 iRecord;}ACK_TYPE_IMG;
typedef struct {u8 HEAD[3];u32 TotalLen;u16 AckType;u32 AckDataLen;u32 UartRxLen;}DRC_RX_PKT;
///


// 경로 인식 클래스 //
class course    // 지나간 경로를 파악하기 위한 구조체 (스택에 넣을것)
{
public:
	unsigned char direction; // 방향 'f' 'b'등  넣을것
	unsigned int time; // 경과시간
	bool saved;		 // 이 지점을 저장할지 여부.
};
///

// CControllerDlg 대화 상자
class CControllerDlg : public CDialogEx
{
// 기본 구현 함수 //
public:
	enum { IDD = IDD_CONTROLLER_DIALOG };
	CControllerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
protected:
	HICON m_hIcon;
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.
/////////////////////////////
//////////// DRC 3.0 관련 //////////////
public:
	BYTE drcdata[64];
	HWND hDRC;
///////////////////////////////////////
	

//////// 패턴매칭을 위한 멤버 /////////////

	int	m_nWidth;		// original image width
	int	m_nHeight;		// original image height
	int	m_nWidthDisplay;	// display image width
	int m_nHeightDisplay;	// display image
	int	m_nWidthPatternDisplay;	// display image width
	int m_nHeightPatternDisplay;	// display image height
	
	void Init();
	static void OnMousePatternDefine(int iEvent, int iX, int iY, int iFlags, void* Userdata);
///////////////////////////////////////////////////
protected:
	afx_msg LRESULT OnReceive(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedStartcom();
	afx_msg void OnBnClickedDisconnect();
	afx_msg void OnBnClickedTorqueon();
	afx_msg void OnBnClickedTorqueoff();
	afx_msg void OnBnClickedAddlist();
	afx_msg void OnBnClickedErase();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedSimulation();
	afx_msg void OnBnClickedSavecourse();
	afx_msg void OnBnClickedComeback();
	afx_msg void OnBnClickedSpeedcontrol();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedButtonPatterndefine();
	afx_msg void OnBnClickedButtonPatternsave();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedStartcom2();
	afx_msg void OnBnClickedDisconnect2();
	afx_msg void OnBnClickedSurf();
	afx_msg void OnBnClickedSurfStop();
	afx_msg void OnBnClickedArmSync();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnBnClickedSurfpatternSave();
	afx_msg void OnBnClickedReceiveimage();
	afx_msg void OnBnClickedMathingInit();
	afx_msg void OnBnClickedButtonPatternLoadpattern();
	afx_msg void OnBnClickedButtonPatternLoadsurf();
	afx_msg void OnBnClickedSavesample();
	afx_msg void OnBnClickedSavesample2();
	afx_msg void OnBnClickedAutomode();
	afx_msg void OnBnClickedSensingLine();
	afx_msg void OnNMCustomdrawTiltControl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton2();
	///////// 멤버 변수 ////////////////
	CString m_volt;
	CListBox m_List;
	bool focusing[6];
	CSliderCtrl m_slider;
	int m_miniarmpos[6];

	
	afx_msg void OnBnClickedButton3();
	afx_msg void OnNMReleasedcaptureTiltControl(NMHDR *pNMHDR, LRESULT *pResult);

	//////////// DRC WIFI 관련 ////////////////
	int Drc_WaitCnt;	// 영상수신을 기다리는 시간 ...OnTimer에서 자동증가 ...영상수신시 0으로 리셋 
	int Drc_Retry_flag;	//재요청 후 응답 여부 ...두번 연속 재요청을 방지함.
	int Drc_Rxmode;
	DrcSocket Drc_ServerSocket;
	DRC_TX_PKT Drc_Cmd;
	DRC_RX_PKT Drc_RxD;
	int Drc_ConnState; // 연결시 자동으로 셋 
	CvMat *cvJpegMat; 	// 영상출력을 위한 Mat생성 
	int ImgSize;		// 영상파일의 크기 
	int FlagNewImg; 	// 새로운 이미지 수신시 셋 
	int IMG_W;		// 영상의 가로폭 
	int IMG_H;		// 영상의 세로폭 
	BYTE *UartTxData;		// UART TX DATA
	int UartTxDataLen;		// UART TX DATA LEN
	BYTE *UartRxData;		// UART RX DATA
	int UartRxDataLen;		// UART RX DATA LEN
	int StartButtonClick; // 시작버튼 클릭시 1 
	ACK_TYPE_IMG tAck;
	int JOG_DIR_1, JOG_LEN_1;
	int JOG_DIR_2, JOG_LEN_2;
	int JOG_BTN1;
	int m_JOG_LEN_1, m_JOG_LEN_2;
	void DRC_ProgramInit();
	void DRC_Connect();
	void DRC_Close();
	void DRC_SendCallBack();
	void DRC_ReceiveCallBack();
	void DRC_RequestImage(void);
	void ReDrawImage(int SX, int SY, int EX, int EY);
	void ImageProcess(IplImage *img);
	////////////////////////////////////////////////////////



};

