
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include "keypad.h"		//버튼 스위치 입력
#include "lcd.h"
#include "usart.h"
#include "Elevator.h"
#include "button.h"
#include "UltraSonic.h"
#include "dot_matrix.h"
#include "Mainloop.h"
#include "stepmotor.h"

extern volatile unsigned short distance ; // 주어진 거리값.
extern unsigned char global_duty;		  // PWM신호의 듀티
extern unsigned char g_mode;			  
// 현재의 엘레베이터 모드 0 - 정지및 이동상태, 1 - 위치조정, 2 - 캐빈도어열림, 3 - 대기중, 4 - 캐빈도어닫힘
extern unsigned char print_flag;		  // 하이퍼터미널 출력을 갱신하는 플래그.
unsigned char people = 0;				  // 탑승객 수 (명)

int main(void) 
{        
	unsigned char key, btn;  // 키, 버튼 입력값
	char string[20];  		// LCD 출력 문자열

	LcdInit();     	   // Lcd 초기화
	KeyInit();		   // 스위치 입력 초기화
	DMInit(); 		   // 도트 매트릭스 초기화
	Usart0Init(57600); // USART0초기화 (실질 2배속이므로 보레이트 115200 이다)
	Sonic_Init();	   // 초음파센서 초기화
	MainTimerInit();   // 메인 루프 타이머초기화 (오버플로 인터럽트 설정)
	BtnInit();		   // 버튼 초기화
	StepmInit();       // 스탭모터 초기화
	DDRB = 0xFF; 	   // LED Array Init
	
	sei();			   // 전역 인터럽트 허용
	DCMInit();		   // DC모터 초기화

	while(1) 		   // main 함수 루프
	{	
		key = KeyInput(); // 키패드 입력을 받음
		btn = BtnInput(); // 버튼 입력을 받음
		switch(key) 
		{
		   case SW0: AddData(1, DIR_NONE);break; // 1층에서는 올라갈수만 있으므로 방향성 없음
		   case SW1: AddData(2, DIR_UP);break;
		   case SW2: AddData(2, DIR_DOWN);break;
		   case SW3: AddData(3, DIR_UP);break;
		   case SW4: AddData(3, DIR_DOWN);break;
		   case SW5: AddData(4, DIR_UP);break;
		   case SW6: AddData(4, DIR_DOWN);break;
		   case SW7: AddData(5, DIR_UP);break;
		   case SW8: AddData(5, DIR_DOWN);break;
		   case SW9: AddData(6, DIR_NONE);break; // 6층에서는 내려갈수만 있으므로 방향성 없음.
		   case SW10: AddData(1, DIR_NONE);break; // 캐빈도어 내부 버튼들. 모두 방향성이 없음.
		   case SW11: AddData(2, DIR_NONE);break;
		   case SW12: AddData(3, DIR_NONE);break;
		   case SW13: AddData(4, DIR_NONE);break;
		   case SW14: AddData(5, DIR_NONE);break;
		   case SW15: AddData(6, DIR_NONE);break;
		   default:		//입력이 없거나,
			break;		//여러 스위치가 눌러진 경우 제외
		}
		switch(btn)
		{
		case BTN_SW0:				 // SW0이 눌린경우 듀티 증가
			if(global_duty <= 250) global_duty += 2; break;
		case BTN_SW1:				 // SW1이 눌린경우 듀티감소
			if(global_duty >= 10)  global_duty -= 2; break;
		case BTN_SW3:				
			if(people >= 9)	people--;	 // 탑승인원 초과 상태에서 SW3을 누르면 탑승객수 한명 감소
			else if(g_mode == 3) people++;	// 캐빈 도어가 열려있는 상태일떄 SW3을 누르면 탑승객수 증가
			break;	
		default:break;
		}
		if(people < 9) // 탑승객 초과상태가 아닐 떄
		{
			// 현재층, 목표층, 듀티값 출력
			LcdMove(0,0);
			sprintf(string, "F:%d N:%d Du:%d", GetCurrentFloor(),GetTargetFloor(),global_duty);
			LcdPuts(string);
			
			// 탑승객수, 도달까지 남은 펄스수 출력
			LcdMove(1,0);
			sprintf(string, "Pe:%d Pulse:%3d  ", people, distance);
			LcdPuts(string);
		}
		else // 탑승객수 초과상태일때
		{
			// 탑승객수 초과상태임을 알림
			LcdMove(0,0);
			LcdPuts("Exceeding       ");
			LcdMove(1,0);
			LcdPuts("capacity         ");
		}
		if(print_flag) // 하이퍼터미널 출력 플래그가 섰을 때 (매 200ms마다)
		{
			Print();   // 출력 갱신
			print_flag = 0; // 플래그 끔
		}
	}
}  6
