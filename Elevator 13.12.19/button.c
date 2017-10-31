//===============================================================
// button.c - 버튼 입력
//===============================================================
#include <avr/io.h>
#include <util/delay.h>
#include "button.h"
	
#define BTN_INPORT	PIND	// 스위치 입력 포트
#define BTN_DDR	    DDRD	// 방향 포트
#define BTN_PORT	PORTD	// 버튼 포트
void BtnInit(void) 
{
	BTN_DDR &= ~(BTN_MASK);	// 스위치 입력핀을 입력으로 설정
	BTN_PORT |= BTN_MASK; // 내부 풀업 사용을 위해 버튼에 해다하는 PORT값을 1로 만듬.
}

//==================================================================================
// 스위치 입력을 받는다.(계속 누르고 있을 때는 입력이 없는 것으로 판단한다.) 
// 리턴 값 : 0 	 - 입력이 없음(계속 누르고 있는 상태포함)
//	    이외 - 스위치 입력(헤더파일 참조)
//==================================================================================
unsigned char BtnInput(void) {
	static unsigned char	psw = 0;		//이전 스위치 값
	unsigned char sw, sw1;

	sw = ~BTN_INPORT & BTN_MASK;			//스위치 입력을 받는다.
	while(1) {	        //연속 두 번 읽은 값이 같을 때까지 루프를 돈다.
	
		_delay_ms(2); 	//디바운싱 시간지연
		sw1 = ~BTN_INPORT & BTN_MASK;	//한 번 더 읽는다.
	
		if(sw == sw1)			//두 번 읽은 값이 같으면 신호가 
			break;			//안정화된 것으로 판단한다.
		sw = sw1;
	}
	if(sw == psw) return 0;			//이전 값과 같으므로 눌러진 상태
	psw = sw;				//이전 값을 저장한다.

	return sw;				//스위치 입력 값을 리턴
}

// 버튼을 계속 누르고 있는 지 판단하는 시간 : PRESS_THRESHOLD
// 디바운싱지연 이상 누르고 있으면 버튼을 계속 누른 것으로 판단

#define PRESS_THRESHOLD		16	
