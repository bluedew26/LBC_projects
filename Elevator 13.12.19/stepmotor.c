//=======================================================
// stepmotor.c : 스텝모터 구동 모듈
//=======================================================

#include <avr/io.h>
#include "stepmotor.h"

#define N_PATTERN	4		// 펄스 패턴 개수

// 여자 방식에 대한 펄스열
static unsigned char pattern1[N_PATTERN]  = 	// PORTG의 패턴
           {0x00, 0x04, 0x02, 0x01};
static unsigned char pattern2[N_PATTERN]  =   // PD6의 패턴
           {(1<<PD6), 0x00, 0x00, 0x00};
static short  index;		// pulse 배열의 인덱스
static short position;		// 현재 위치

//========================================================
//	스텝모터 모듈을 초기화한다.
//	입력 : type - 스텝모터 구동방식을 선택
//				ONE_PHASE : 1상여자 방식 선택
//				그외 경우 :   2상여자 방식 선택
//========================================================	

void StepmInit() {
	DDRG = 0xFF; // 포트G 출력
	DDRD |= (1<<PD6); // PD6을 출력.
	index = 0;			// 인텍스의 초기화
	position = 0;			// 위치의 초기화 
	PORTG = pattern1[index];	// 첫 펄스 패턴을 출력한다.
	PORTD &= ~(1<<PD6);
	PORTD |= pattern2[index]; 
}

//=======================================================
//	현재 스텝모터의 위치를 반환한다.
//	반환 값 : 현재 스텝모터의 위치
//=======================================================

short StepmCurrentPos() 
{
	return position;
}

//=======================================================
//	스텝모터의 위치를 1스텝 정방향으로 이동한다.
//=======================================================

void StepmUp(void) {
	// 인텍스를 증가, 순환시킨다.
	index++;
	if(index == N_PATTERN)
		index = 0;

	PORTG = pattern1[index];	// 포트에 펄스를 내보낸다.
	PORTD &= ~(1<<PD6);
	PORTD |= pattern2[index]; 
	position++;					// 1스텝 위치 증가
}




//=======================================================
//	스텝모터의 위치를 1스텝 역방향으로 이동한다.
//=======================================================

void StepmDn(void) {
	if(index == 0)
		index = N_PATTERN;

	index--;
	PORTG = pattern1[index];	// 포트에 펄스를 내보낸다.
	PORTD &= ~(1<<PD6);
	PORTD |= pattern2[index]; 
	position--;					// 1스텝 위치 증가
}
