#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__

void Sonic_Init(); // 초음파 센서 초기화 함수
void pulse();      // 송신 펄스를 주는 함수
unsigned char GetDistance(); // 초음파센서값을 받는 함수.
#endif
