#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "UltraSonic.h"

static unsigned char sonic_value = 0;	// 센서 측정값
volatile unsigned int pulse_count = 0;  // 수신 펄스 길이에 따라 카운트 되는 값.

ISR(TIMER2_COMP_vect) // 58us마다
{
	pulse_count++;
}

ISR(INT0_vect)
{
	static unsigned char toggle = 0;
    if(toggle == 0) // 카운터 시작하고 외부 인터럽트(INT4)를 다음에는 Falling Edge에거 걸리도록 함.
    {
        pulse_count = 0; // 측정된 이전 거리값 초기화
        TIMSK |= (1 << OCIE2); // 타이머카운터 2 비교일치인터럽트 허용
        TCNT2 = 0;    // 카운트 초기화
        EICRA = 0x02; // 외부인터럽트0 하강엣지에서 작동.
        toggle = 1;
    }
    else // 카운터를 정지시키고 외부 인터럽트(INT4)를 다음에는 Riging Edge에서 걸리도록 함.
    {
        TIMSK &= ~(1 << OCIE2); // 타이머 카운터2 비교일치 인터럽트 불허
		sonic_value = pulse_count; // *0.5cm
        EICRA = 0x03; //  외부인터럽트0 상승엣지에서 작동.
		toggle = 0;
    }                          
             
}

void Sonic_Init()
{
	TCCR2 = (1 << WGM21) | (1 << CS21);		 // 타이머2 CTC모드, OC2핀 미사용, 분주비 8
	OCR2 = 57;       						 // 29us마다 타이머0 비교일치 인터럽트 발생. (펄스 58us당 1cm, 29us는 0.5cm)
	DDRD |= (1<<PD7);// PD7 출력.

	// 인터럽트
	EIMSK |= (1<<INT0); //외부인터럽트 0 허용
    EICRA |= 0x03; //외부인터럽트 0 상승엣지에서 작동.
}

void pulse()
{
	PORTD |= (1<<PD7); // PD7에 15us의 펄스를 줌.
	_delay_us(15);
	PORTD &= ~((1<<PD7));
}

unsigned char GetDistance()
{
	return sonic_value;
}
