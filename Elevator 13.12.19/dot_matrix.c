/////////////////////////////////////////////////////////////////////////////////
// 파일명 : dot_matrix.c													   //
// 역할 : 도트매트릭스를 제어하는 함수를 모아놓은 파일						   //
/////////////////////////////////////////////////////////////////////////////////

#include "dot_matrix.h"
#include <avr/io.h>

#define DM_COLUMN_PORT	PORTA
#define DM_ROW_PORT		PORTE
#define DM_COLUMN_IO	DDRA
#define DM_ROW_IO		DDRE

volatile static int row = 0;  // 현재 출력행

static char UP_pattern[6] = {0x18, 0x3c, 0x7E, 0xFF, 0x18, 0x18}; // 위쪽 화살표
static char DOWN_pattern[6] = {0x18, 0x18, 0xFF, 0x7E, 0x3c, 0x18}; // 아래쪽 화살표
static char NO_pattern[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 아래쪽 화살표

static char *DM_pattern = NO_pattern; // 도트매트릭스 패턴을 받는 포인터. 초기값은 R 패턴.

void DM_ON_UP() { DM_pattern = UP_pattern;  row = 0;} // 도트매트릭스에 문자 R을 출력 (+ 출력행을 초기화)
void DM_ON_DOWN() { DM_pattern = DOWN_pattern;  row = 0;} // 도트매트릭스에 문자 Y를 출력
void DM_OFF() {DM_pattern = NO_pattern; row = 0;}
/////////////////////////////////////////////////////////////////////////////////////
// 함수 DM_NextRow()															   //
// 역할 : 출력패턴을 다음행으로 바꾸는 것 뿐이지만 실질적으로 2ms주기마다 		   //
//		  반복호출되어 도트매트릭스 패턴을 표시하는 역할을 하는 함수이다.		   //
/////////////////////////////////////////////////////////////////////////////////////

void DMInit()
{
	DM_COLUMN_IO = 0xFF;
	DM_ROW_IO = 0xFC; // 하위 비트 2개는 통신에 사용됨.
}


void DM_NextRow() 
{
	if(++row == 6) 		// 현재 출력행을 증가시킴 (0~7사이의 값을 가짐)
	{
		row = 0;		
	}
	DM_ROW_PORT &= 0x03; // 하위 2비트를 제외하고 초기화.
	DM_ROW_PORT |= (0x01<<(row + 2));       // row+1번째행 출력
	DM_COLUMN_PORT = ~(DM_pattern[row]); // 현재 출력행의 반전형태를 대입 (논리0일때 LED가 켜지기 때문)
}


	
