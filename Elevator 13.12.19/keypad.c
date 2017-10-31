///////////////////////////////////////////////////
//  "keypad.c" : Keyboard Matrix 인터페이스 
//  포트 핀     F7  F6  F5  F4  F3  F2  F1  F0  
//  키 패드     I3  I2  I1  I0  O3  O2  O1  O0
//   I : 키패드 입력 핀 / O : 키패드 출력 핀      
///////////////////////////////////////////////////
#include <avr/io.h>             
#include <util/delay.h>
#include <stdio.h>
#include "keypad.h" 

#define N_COL    4	//스캔 할 키패드 열 수 
#define KEY_OUT  PORTF	//keyboard 출력 포트
#define KEY_IN   PINF	//keyboard 입력 포트         
#define KEY_DIR  DDRF	//keyboard 입출력 방향 레지스터 

static unsigned char key_scan(void);

void KeyInit(void){
        KEY_OUT = 0xF0;	// 출력 초기 값으로 0을 출력
        KEY_DIR = 0x0F;	// 상위니블->입력, 하위니블->출력 
}        

//////////////////////////////////////////////////
//  키패드의 스캔 코드 값을 리턴한다.
//    리턴 값 : 0이 아닐 때 : 스캔 코드 값
//              0일 때      : 입력 값이 없음
//////////////////////////////////////////////////            
unsigned char KeyInput(void) {
	static unsigned char pin = 0;	//이전 키값
	unsigned char in, in1;
        
	    in = key_scan();            //키스캔하여 값을 읽음           
        while(1) {
        	//디바운싱 시간지연
        	_delay_ms(2); 
        	in1 = key_scan(); //한번 더 읽음
        	if(in == in1) break;
        	in = in1;
        }
        if( !(in & 0xF0) ) {	//눌러진 키가 없음  
        	pin = 0;
        	return 0;
        }
        if(pin == in) {  //같은 키를 계속 누르고 있음	
        	return 0;//새로운 키 입력이 없는 것으로 판단
        }
        pin = in;    //키값을 저장	      
        
        return in;   //코드반환
}        
	
//스캔코드값을 리턴함
static unsigned char key_scan(void) {
	unsigned char  out, i, in;
	
	out = 0x01; //3-열부터 스캔
	for(i=0; i<N_COL; i++) {
		KEY_OUT = ~out;//출력포트에 스캔출력을 낸다
		asm("nop"::);  //1사이클 지연
		in = (~KEY_IN) & 0xF0; //입력된 값의 상위 4비트 취함
		if(in) {       //키입력 있음
		    in += out; //출력과 입력을 조합하여 코드생성
		    break;     //출력이 있으므로 루프탈출	
		}
		out <<= 1; //다음 스캔코드로 변경	
	}
	return in;         //스캔코드값 리턴       	
}		

