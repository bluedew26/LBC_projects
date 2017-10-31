#include <util/delay.h>
#include <avr/io.h>
#include "sensor.h"

void sensor_init(void)
{
//	EIMSK = (1 << INT5) | (1 << INT6) | (1 << INT7); //5,6,7 인터럽트 허용
 //   EICRB = (3 << ISC50) | (3 << ISC60) | (3 << ISC70); //5,6,7인터럽트 상승엣지.


	DDRA = 0b11100000;
	TCCR2 = (1 << WGM21) | (2 << CS20);		 // 타이머2 CTC모드, OC0핀 미사용, 분주비 8
	OCR2 = 116;       						 // 56us마다 타이머2 비교일치 인터럽트 발생. (56us당 1cm)
}
extern unsigned char sensor_num;

void SONIC_ON(unsigned char num)
{
	switch(num)
	{
	case 1:
		PORTA |= 0x20; // PA5
		_delay_us(11);
		PORTA &= ~(0x20);
		sensor_num = 1;
		break;
	case 2:
		PORTA |= 0x40; // PA6
		_delay_us(11);
		PORTA &= ~(0x40);
		sensor_num = 2;
		break;
	case 3:
		PORTA |= 0x80; // PA7
		_delay_us(11);
		PORTA &= ~(0x80);
		sensor_num = 3;
		break;
	default:
		sensor_num = 0;
		break;
	}
}


unsigned char maxval(unsigned char* arr)
{
	unsigned char max = arr[0];
	for(int i=1; i<SAMPLE_NUM; i++) // 최근 5개에서 최대값과 최소값을 뻄
	{
		if(arr[i] > max)
			max = arr[i];
	}
	return max;
}
unsigned char minval(unsigned char* arr)
{
	unsigned char min = arr[0];
	for(int i=1; i<SAMPLE_NUM; i++)
	{
		if(arr[i] < min)
			min = arr[i];
	}
	return min;
}

unsigned char mean(unsigned char* arr) // 최대 / 최소값을 제외한 평균을 구함
{
	unsigned int sum = 0;
	for(int i=0; i<SAMPLE_NUM; i++)
		sum += arr[i];
	unsigned char result = (unsigned char)((sum - minval(arr) - maxval(arr)) / (SAMPLE_NUM - 2));
	if(result >= 254) result = 253;
	
	return result; // 3으로 나눔 (추후변경가능)
}
