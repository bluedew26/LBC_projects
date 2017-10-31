///////////////////////////////////////////////////
//  "keypad.c" : Keyboard Matrix �������̽� 
//  ��Ʈ ��     F7  F6  F5  F4  F3  F2  F1  F0  
//  Ű �е�     I3  I2  I1  I0  O3  O2  O1  O0
//   I : Ű�е� �Է� �� / O : Ű�е� ��� ��      
///////////////////////////////////////////////////
#include <avr/io.h>             
#include <util/delay.h>
#include <stdio.h>
#include "keypad.h" 

#define N_COL    4	//��ĵ �� Ű�е� �� �� 
#define KEY_OUT  PORTF	//keyboard ��� ��Ʈ
#define KEY_IN   PINF	//keyboard �Է� ��Ʈ         
#define KEY_DIR  DDRF	//keyboard ����� ���� �������� 

static unsigned char key_scan(void);

void KeyInit(void){
        KEY_OUT = 0xF0;	// ��� �ʱ� ������ 0�� ���
        KEY_DIR = 0x0F;	// �����Ϻ�->�Է�, �����Ϻ�->��� 
}        

//////////////////////////////////////////////////
//  Ű�е��� ��ĵ �ڵ� ���� �����Ѵ�.
//    ���� �� : 0�� �ƴ� �� : ��ĵ �ڵ� ��
//              0�� ��      : �Է� ���� ����
//////////////////////////////////////////////////            
unsigned char KeyInput(void) {
	static unsigned char pin = 0;	//���� Ű��
	unsigned char in, in1;
        
	    in = key_scan();            //Ű��ĵ�Ͽ� ���� ����           
        while(1) {
        	//��ٿ�� �ð�����
        	_delay_ms(2); 
        	in1 = key_scan(); //�ѹ� �� ����
        	if(in == in1) break;
        	in = in1;
        }
        if( !(in & 0xF0) ) {	//������ Ű�� ����  
        	pin = 0;
        	return 0;
        }
        if(pin == in) {  //���� Ű�� ��� ������ ����	
        	return 0;//���ο� Ű �Է��� ���� ������ �Ǵ�
        }
        pin = in;    //Ű���� ����	      
        
        return in;   //�ڵ��ȯ
}        
	
//��ĵ�ڵ尪�� ������
static unsigned char key_scan(void) {
	unsigned char  out, i, in;
	
	out = 0x01; //3-������ ��ĵ
	for(i=0; i<N_COL; i++) {
		KEY_OUT = ~out;//�����Ʈ�� ��ĵ����� ����
		asm("nop"::);  //1����Ŭ ����
		in = (~KEY_IN) & 0xF0; //�Էµ� ���� ���� 4��Ʈ ����
		if(in) {       //Ű�Է� ����
		    in += out; //��°� �Է��� �����Ͽ� �ڵ����
		    break;     //����� �����Ƿ� ����Ż��	
		}
		out <<= 1; //���� ��ĵ�ڵ�� ����	
	}
	return in;         //��ĵ�ڵ尪 ����       	
}		

