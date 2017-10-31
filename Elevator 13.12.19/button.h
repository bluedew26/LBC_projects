//=================================================
// 버튼 입력 헤더파일 button.h
//=================================================
#ifndef __BUTTON_H__
#define __BUTTON_H__

// 입력 버튼 정의
#define BTN_SW0 	0x02		// 스위치 0을 누름
#define BTN_SW1		0x04		// 스위치 1을 누름
#define BTN_SW2		0x08		// 스위치 2를 누름
#define BTN_SW3		0x20		// 스위치 3을 누름

#define NO_BTN		0x00		// 입력이 없음

#define BTN_MASK	(BTN_SW0 | BTN_SW1 | BTN_SW2 | BTN_SW3)

//함수 정의
void BtnInit(void);
unsigned char BtnInput(void);

#endif
