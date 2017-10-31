
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include "keypad.h"		//��ư ����ġ �Է�
#include "lcd.h"
#include "usart.h"
#include "Elevator.h"
#include "button.h"
#include "UltraSonic.h"
#include "dot_matrix.h"
#include "Mainloop.h"
#include "stepmotor.h"

extern volatile unsigned short distance ; // �־��� �Ÿ���.
extern unsigned char global_duty;		  // PWM��ȣ�� ��Ƽ
extern unsigned char g_mode;			  
// ������ ���������� ��� 0 - ������ �̵�����, 1 - ��ġ����, 2 - ĳ�󵵾��, 3 - �����, 4 - ĳ�󵵾����
extern unsigned char print_flag;		  // �������͹̳� ����� �����ϴ� �÷���.
unsigned char people = 0;				  // ž�°� �� (��)

int main(void) 
{        
	unsigned char key, btn;  // Ű, ��ư �Է°�
	char string[20];  		// LCD ��� ���ڿ�

	LcdInit();     	   // Lcd �ʱ�ȭ
	KeyInit();		   // ����ġ �Է� �ʱ�ȭ
	DMInit(); 		   // ��Ʈ ��Ʈ���� �ʱ�ȭ
	Usart0Init(57600); // USART0�ʱ�ȭ (���� 2����̹Ƿ� ������Ʈ 115200 �̴�)
	Sonic_Init();	   // �����ļ��� �ʱ�ȭ
	MainTimerInit();   // ���� ���� Ÿ�̸��ʱ�ȭ (�����÷� ���ͷ�Ʈ ����)
	BtnInit();		   // ��ư �ʱ�ȭ
	StepmInit();       // ���Ǹ��� �ʱ�ȭ
	DDRB = 0xFF; 	   // LED Array Init
	
	sei();			   // ���� ���ͷ�Ʈ ���
	DCMInit();		   // DC���� �ʱ�ȭ

	while(1) 		   // main �Լ� ����
	{	
		key = KeyInput(); // Ű�е� �Է��� ����
		btn = BtnInput(); // ��ư �Է��� ����
		switch(key) 
		{
		   case SW0: AddData(1, DIR_NONE);break; // 1�������� �ö󰥼��� �����Ƿ� ���⼺ ����
		   case SW1: AddData(2, DIR_UP);break;
		   case SW2: AddData(2, DIR_DOWN);break;
		   case SW3: AddData(3, DIR_UP);break;
		   case SW4: AddData(3, DIR_DOWN);break;
		   case SW5: AddData(4, DIR_UP);break;
		   case SW6: AddData(4, DIR_DOWN);break;
		   case SW7: AddData(5, DIR_UP);break;
		   case SW8: AddData(5, DIR_DOWN);break;
		   case SW9: AddData(6, DIR_NONE);break; // 6�������� ���������� �����Ƿ� ���⼺ ����.
		   case SW10: AddData(1, DIR_NONE);break; // ĳ�󵵾� ���� ��ư��. ��� ���⼺�� ����.
		   case SW11: AddData(2, DIR_NONE);break;
		   case SW12: AddData(3, DIR_NONE);break;
		   case SW13: AddData(4, DIR_NONE);break;
		   case SW14: AddData(5, DIR_NONE);break;
		   case SW15: AddData(6, DIR_NONE);break;
		   default:		//�Է��� ���ų�,
			break;		//���� ����ġ�� ������ ��� ����
		}
		switch(btn)
		{
		case BTN_SW0:				 // SW0�� ������� ��Ƽ ����
			if(global_duty <= 250) global_duty += 2; break;
		case BTN_SW1:				 // SW1�� ������� ��Ƽ����
			if(global_duty >= 10)  global_duty -= 2; break;
		case BTN_SW3:				
			if(people >= 9)	people--;	 // ž���ο� �ʰ� ���¿��� SW3�� ������ ž�°��� �Ѹ� ����
			else if(g_mode == 3) people++;	// ĳ�� ��� �����ִ� �����ϋ� SW3�� ������ ž�°��� ����
			break;	
		default:break;
		}
		if(people < 9) // ž�°� �ʰ����°� �ƴ� ��
		{
			// ������, ��ǥ��, ��Ƽ�� ���
			LcdMove(0,0);
			sprintf(string, "F:%d N:%d Du:%d", GetCurrentFloor(),GetTargetFloor(),global_duty);
			LcdPuts(string);
			
			// ž�°���, ���ޱ��� ���� �޽��� ���
			LcdMove(1,0);
			sprintf(string, "Pe:%d Pulse:%3d  ", people, distance);
			LcdPuts(string);
		}
		else // ž�°��� �ʰ������϶�
		{
			// ž�°��� �ʰ��������� �˸�
			LcdMove(0,0);
			LcdPuts("Exceeding       ");
			LcdMove(1,0);
			LcdPuts("capacity         ");
		}
		if(print_flag) // �������͹̳� ��� �÷��װ� ���� �� (�� 200ms����)
		{
			Print();   // ��� ����
			print_flag = 0; // �÷��� ��
		}
	}
}  6
