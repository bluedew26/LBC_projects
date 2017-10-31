#include "Mainloop.h"
#include "dot_matrix.h"
#include "Elevator.h"
#include "stepmotor.h"
#include "UltraSonic.h"
#include <avr/io.h>
#include <avr/interrupt.h>

int pcurrent_floor;			// 현재층이 변동했는지 확인하기 위한 이전 값
unsigned short pdistance;   // 400ms이전의 남은 펄스값을 저장하는 변수.
unsigned char print_flag = 0; // 하이퍼터미널로의 출력을 요청하는 플래그, 수행시간이 다소 길기떄문에 인터럽트 내부보다는 main에서 처리한다.
extern int current_floor;		// 현재 층
extern unsigned char e_stop;    // 비상정지가 눌렸는지 여부
extern unsigned char g_mode;    // 현재 엘레베이터 모드 (0~4)
extern unsigned char target_dis;// 초음파 센서로 조정할 때 목표 거리값
extern unsigned char dis;		// 초음파 센서로 조정할 때 현재 거리값
extern unsigned char people;    // 탑승객 수
extern unsigned short distance; // 도달까지 남은 pulse수


void MainTimerInit()
{
	TIMSK |= (1<<TOIE0); // 타이머1 오버플로 인터럽트를 허용한다 (본래 타이머1은 PWM신호용이었지만, 재활용인 셈)
}



ISR(TIMER0_OVF_vect)
{
	static unsigned char count1 = 0; 
	static unsigned int  count2 = 0; // 2초간
	static unsigned int  count3 = 0; // 200ms마다 하이퍼터미널 출력 갱신을 위한 카운트
	static unsigned int  count4 = 0; // 400ms이전의 도달까지 남은 pulse수를 저장하여 비교하기 위한 카운트
	///////////////////////
	// 200ms마다 반복하는 구문
	if(++count3 == 100)
	{
		print_flag = 1; // 하이퍼터미널 출력을 요청하는플래그를 세움 (처리는 main에서)
		count3 = 0;
	}
	/////////////////////

	// 비상정지 예외 //
	if(e_stop) // 비상정지중일때는
		return; // 하이퍼터미널 출력을 제외하고 아무일도 안함
	///////////////////

	// 20ms마다 반복하는 구문 //
	if(++count1 == 10)
	{
		pulse(); // 초음파 발사
		count1 = 0;
	}
	////////////////////////////

	/// g_mode에 따른 시퀀스를 위한 카운트 감소 //
	if(count2)
		count2--;
	//////////////////////////////////////////////
	
	////////// 목표층에 도달했는지 수시로 확인 /////
	is_arrived();
	////////////////////////////////////////////////

	//// 도트 매트릭스 행 갱신 ///////////
	DM_NextRow(); // 2ms주기로 행 갱신
	/////////////////////////////////////

///// 층이 변할때 LED 표시에 변화를 주는 부분 /////////////
	if(pcurrent_floor != current_floor)
	{
		pcurrent_floor = current_floor;
		PORTB &= ~(0xCF); // LED에 array 해당하는 비트를 우선 끈다.
		switch(pcurrent_floor)
		{
			case 1: PORTB |= 0x01; break;
			case 2: PORTB |= 0x02; break;
			case 3: PORTB |= 0x04; break;
			case 4: PORTB |= 0x08; break;
			case 5: PORTB |= 0x40; break;
			case 6: PORTB |= 0x80; break;
			default: break;
		}	// 현재 층에 해당하는 LED를 켬.
	}
////////////////////////////////////////////


//////////// g_mode 변동에 관한 것 ////////////////
	if(g_mode == 1) // 초음파 조정모드일떄
	{
		dis = GetDistance();  // 초음파센서값을 계속 받으면서
		if(dis < target_dis-1) // 거리값이 목표거리값 (도달해야할 층의 중앙위치) 부근에 위치할때까지 위나 아래로 이동한다.
			E_Up(S_DEF);
		else if (dis > target_dis+1)
			E_Down(S_DEF);
		else                 // 목표거리값과 일치하면
		{
			Stop(); 		 // 일단 엘리베이터를 정지시키고
			g_mode = 2; 	 // 조정완료, 캐빈도어를 여는모드로 들어감.
			count2 = 500;	 // 1초동안 열림
		}
	}
	else if(g_mode == 2)
	{
		if(count2 % 4 == 1) // 매 8ms마다
			StepmUp(); 		// 캐빈도어를 염. (스텝모터 정회전)
		if(count2 == 0)
		{
			g_mode = 3; 
			count2 = 1500; // 캐빈도어 열려있는 상태로 3초 유지
			if(people) 	   // 사람이 있었으면
				people--;  // 사람 한명 줄음
		}
	}
	else if(g_mode == 3)
	{
		if(count2 == 0 && people < 9)  // 사람수가 9명 미만이어야 문이 닫힘. (9명부터 정원초과)
		{
			g_mode = 4;				   // 캐빈도어가 닫히는 모드
			count2 = 500;			   // 1초간 닫힘
			
		}
	}
	else if(g_mode == 4)
	{
		if(count2 % 4 == 1) // 매 8ms마다
			StepmDn();		// 캐빈도어를 닫음 (스탭모터 역회전)
		if(count2 == 0)	    // 캐빈도어가 다 닫히면
		{
			g_mode = 0; 	// 모드0으로 변경
			SetNextFloor(); // 다음 목표층이 어디인지 파악한다.
			GoNextFloor();  // 다음 목표층으로 이동한다.
		}
	}
//////////////////////////////////////////////////////////////
	
	/// 400ms마다 반복하는 구문 -> (바닥이나 천장에 박히는것을 방지하기 위한 대책)
	if(++count4 == 200)
	{
		if(pdistance == distance && GetDirection() != 0 && g_mode == 0) // 모드0에 정지상태가 아님에도 400ms동안 변화가 없었다면
		{
			distance = 0;
			current_floor = GetTargetFloor(); // 현재층을 목표층으로 취급해 강제로 is_arrive를 호출시켜 초음파센서로 조정을 시작한다.
		}
		count4 = 0;
		pdistance = distance;
	}
	//////////////////////////

}
