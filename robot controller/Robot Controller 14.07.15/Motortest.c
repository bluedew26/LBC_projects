#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "dynamixel.h"
#include "avr_lib.h"
#include "sensor.h"
#include "wheel_control.h"


#define BIT_RS485_DIRECTION  0x04  // PORTE 2�� ��Ʈ.


byte init_offset = 112; // ���⼾�� �ʱ� ����Ӱ� 
int scanning_count = 0;
extern byte move_count;
unsigned char sensor_num = 0; // � ������ Ȱ��ȭ���ΰ�.
unsigned char sync_mode = 0; // �������ΰ� �ƴѰ�.
unsigned char sensor_delay = 0; // ���� ī��Ʈ.
unsigned char sensor_delay2 = 0; // ADC �ݺ��ֱ�
unsigned int centi = 0;

unsigned char sonic_mean_value[6] = {0}; // �� �����ļ����� ��հ�
unsigned char sonic_value1[SAMPLE_NUM] = {0}; // �����ļ��� 1 ����
unsigned char sonic_value2[SAMPLE_NUM] = {0}; // �����ļ��� 2 ����
unsigned char sonic_value3[SAMPLE_NUM] = {0}; // �����ļ��� 3 ����
unsigned char TILT_value1[SAMPLE_NUM] = {0}; // ���⼾�� 1 ����
unsigned char TILT_value2[SAMPLE_NUM] = {0}; // ���⼾�� 2 ����
unsigned char TILT_value3[SAMPLE_NUM] = {0}; // ���⼾�� 3 ����


unsigned char sonic_index = 0;	// �ε���
unsigned char TILT_index = 0;

unsigned char TILT_sensor[3] = {0}; // ���ܼ����� �������
unsigned char auto_mode = 0;

ISR(USART1_RX_vect) // ��ȣ�� �ٲ�� �� ��ȣ�� �´��� Ȯ�ν�Ŵ.
{

	unsigned char receive = 0;
	static unsigned char speed = 20; // �ʱ�ӵ�
	static byte rx_start = 0;
	static int motornum = 0;
	static char control = 0; // 1�� ���� ��������, 2�� ��������, 3�� ���� �ӵ�����
	static int count = 0;
	receive = UDR1;	


DDRC=0xFF;
PORTC=0xFF;
	if(!control)
	{
		if (receive == 246)				// ����
			angle_reset();		
		else if(receive >= 11 && receive <= 20)       // ���� �̵� (������)
		{
			motornum = receive;
			if(receive == 19)
				motornum = 0;
			else if (receive == 20)
				motornum = 20;
			control = 1;	
		}			
		else if (receive == 0xFE) // �ӵ� ����
			control = 3;
		else if(receive >= 21 && receive <= 30) // ���� ��ũ��
		{
			motornum = receive - 10;
			Torque_on(motornum);
		}
		else if(receive >= 31 && receive <= 40) // ���� ��ũ����
		{
			motornum = receive - 20;
			Torque_off(motornum);
		}
		else if (receive == 247)
			scan();
		else if(receive == 240) // �� ����
			sync_mode = 1;
		else if(receive == 241) // �� ���� ����
			sync_mode = 0;
		else if(receive == 238) // ��������
			control = 2;
		else if(receive == 244) 				// ��ȸ��
			rotate_left();
		else if (receive == 245)				// ��ȸ��
			rotate_right();
		else if (receive == 243)				// ����
			backward();
		else if (receive == 242)				// ����
			forward();
		else if (receive == 248)
			scan_forward();						// Ž�� ����
		else if (receive == 250)				// ��ü��ũ��
			all_torque_on();
		else if (receive == 251)				// ��ü ��ũ����
			all_torque_off();

	}
	else
	{
		if(control == 1)   // ���� ��������
		{
			Control_Pos(motornum, (unsigned int)receive<<2, speed);
			control = 0;
		}
		else if(control == 2)  // ���� ��ü �ӵ� ����
		{
			control = 0;
			init_offset = receive;
		}
		else if (control == 3)
		{
			speed = receive;
			control = 0;
		}
	}
}


void PortInitialize(void)
{
  cbi(SFIOR,2); //All Port Pull Up ready
  DDRE |= (BIT_RS485_DIRECTION); //set output the bit RS485direction

  TCCR0 = (1 << WGM01) | (7 << CS00);
  OCR0 = 155;

}



int main()
{
	byte tx_delay = 0;

	byte bCount,bID, bTxPacketLength,bRxPacketLength;
	PortInitialize();    // 485�����Ʈ�� �ƿ�ǲ���� �����ϴ°� ����.
	RS485_RXD; //Set RS485 Direction to Input State.
	SerialInitialize(SERIAL_PORT0,1,RX_INTERRUPT);	//RS485 Initializing(RxInterrupt)
	SerialInitialize(SERIAL_PORT1,DEFAULT_BAUD_RATE,RX_INTERRUPT);	
  	gbRxBufferReadPointer = gbRxBufferWritePointer = 0;  //RS485 RxBuffer Clearing.

	sensor_init();

	_delay_ms(1000);
	sei();
	UDR1 = 0;
	Torque_on(254);
/*���� 
	Control_Pos(20, (unsigned int)(128 - ((init_offset - 100) * 2 / 3))<<2, 25);
	Control_Pos(0, (unsigned int)(128 + ((init_offset - 100) * 2 / 3))<<2, 25);
	Control_Pos(11, (unsigned int)128<<2, 25);
	*/

	
//	_delay_ms(1000);
	Control_Pos(9, (unsigned int)512<<2, 50);
//	Control_Pos(10, (unsigned int)INIT_MOTOR10<<2, 50);
//	Control_Pos(7, (unsigned int)INIT_MOTOR7<<2, 50);
//	Control_Pos(8, (unsigned int)INIT_MOTOR8<<2, 50);
	_delay_ms(200);	
//	Control_Pos(16, (unsigned int)30<<2, 20);
//	Control_Pos(12, (unsigned int)3<<2, 20);
//	Control_Pos(13, (unsigned int)40<<2, 20);
//	Control_Pos(14, (unsigned int)189<<2, 20);
	Control_Pos(15, (unsigned int)185<<2, 20);




TxDString("set");
	while(1)
	{

		if(TIFR & (1 << OCF0)) // Ÿ�̸�0 ����ġ �÷��� �߻�
		{
			if(move_count) move_count--;
			if(scanning_count) scanning_count--;
			if(auto_mode)
			{
				if(++tx_delay == 5) tx_delay = 0;
			}
			else
				if(++tx_delay == 9) tx_delay = 0;
			if(++sensor_delay == 9) sensor_delay = 0;
			if(++sensor_delay2 == 3) sensor_delay2 = 0;
			TIFR = (1 << OCF0);
		}
	
		if(TIFR & (1 << OCF2)) // Ÿ�̸�2 ����ġ �÷��� �߻�.
		{
			if(PINE & 0b11100000) // ���� �ϳ��� �޽��� ����������, ���� ���������� �κ�.
				centi++; // ���� ������Ŵ
			else if(sensor_num)           // �޽��� �����ְ� sensor_num�� ���ǵǾ��ִ°��
			{
				switch(sensor_num)
				{
				case 1:
					sonic_value1[sonic_index] = centi; // �� sonic_value�� �����ļ����� ���� ����ɰ���.
					break;
				case 2:
					sonic_value2[sonic_index] = centi;
					break;
				case 3:
					sonic_value3[sonic_index] = centi;
					break;
				default:
					break;
				}
				centi = 0;
				sensor_num = 0;
			}

			TIFR = (1 << OCF2);
		}
		if(tx_delay == 0)
		{
			tx_delay++;
			if(!sync_mode) // �� �������°� �ƴҶ��� �����͸� ����
			{
				cbi(UCSR1B,7);
				TxD81(255);  // ���
				
				sonic_mean_value[0] = mean(sonic_value1);
				sonic_mean_value[1] = mean(sonic_value2);
				sonic_mean_value[2] = mean(sonic_value3);
				TILT_sensor[0] = mean(TILT_value1);
				TILT_sensor[1] = mean(TILT_value2);
				TILT_sensor[2] = mean(TILT_value3);

				TxD81(sonic_mean_value[0]);
				TxD81(sonic_mean_value[1]);
				TxD81(sonic_mean_value[2]);
				TxD81(TILT_sensor[0]); // ƿƮ
				TxD81(TILT_sensor[1]);
				TxD81(TILT_sensor[2]);
				
				Read_Volt(21); // ��Ʈ ����. ���� 0.1V
				if(auto_mode == 0)
				{
					Read_Pos(11); // 7������ ���� �о� ����
					Read_Pos(12); 
					Read_Pos(13); 
					Read_Pos(14);
					Read_Pos(15);
					Read_Pos(16);
		
					Read_Pos(0); // �����������Ͱ� 
				}
				TxD81(254); // ���
				sbi(UCSR1B,7);
			}
		
			Tilt_Control(TILT_sensor[1]); // ���� �����ѵ� ��������
		}
			
		switch(sensor_delay) // �����ĸ� ���������� ��� ����.
		{
			case 0:
				if(++sonic_index == SAMPLE_NUM) sonic_index = 0; // �ε��� �ʱ�ȭ
				sensor_delay++;
				SONIC_ON(1);
				break;
			case 3:
				sensor_delay++;
				SONIC_ON(2);
				break;
			case 6:
				sensor_delay++;
				SONIC_ON(3);
				break; 
			default:
				break;
		}
		if(sensor_delay2 == 0)
		{
			++sensor_delay2;
			if(++TILT_index == SAMPLE_NUM) TILT_index = 0;
			TILT_value1[TILT_index] = ADC_Convert_16(1) >> 2; // ���� ������ �κ�
			TILT_value2[TILT_index] = ADC_Convert_16(2) >> 2;
			TILT_value3[TILT_index] = ADC_Convert_16(3) >> 2;
		}
	}
	
}
