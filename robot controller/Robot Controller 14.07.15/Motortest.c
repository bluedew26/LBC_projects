#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "dynamixel.h"
#include "avr_lib.h"
#include "sensor.h"
#include "wheel_control.h"


#define BIT_RS485_DIRECTION  0x04  // PORTE 2번 비트.


byte init_offset = 112; // 기울기센서 초기 기울임값 
int scanning_count = 0;
extern byte move_count;
unsigned char sensor_num = 0; // 어떤 센서가 활성화중인가.
unsigned char sync_mode = 0; // 연동중인가 아닌가.
unsigned char sensor_delay = 0; // 센서 카운트.
unsigned char sensor_delay2 = 0; // ADC 반복주기
unsigned int centi = 0;

unsigned char sonic_mean_value[6] = {0}; // 각 초음파센서의 평균값
unsigned char sonic_value1[SAMPLE_NUM] = {0}; // 초음파센서 1 샘플
unsigned char sonic_value2[SAMPLE_NUM] = {0}; // 초음파센서 2 샘플
unsigned char sonic_value3[SAMPLE_NUM] = {0}; // 초음파센서 3 샘플
unsigned char TILT_value1[SAMPLE_NUM] = {0}; // 기울기센서 1 샘플
unsigned char TILT_value2[SAMPLE_NUM] = {0}; // 기울기센서 2 샘플
unsigned char TILT_value3[SAMPLE_NUM] = {0}; // 기울기센서 3 샘플


unsigned char sonic_index = 0;	// 인덱스
unsigned char TILT_index = 0;

unsigned char TILT_sensor[3] = {0}; // 적외선센서 저장공간
unsigned char auto_mode = 0;

ISR(USART1_RX_vect) // 신호가 바뀌면 이 신호가 맞는지 확인시킴.
{

	unsigned char receive = 0;
	static unsigned char speed = 20; // 초기속도
	static byte rx_start = 0;
	static int motornum = 0;
	static char control = 0; // 1은 단일 모터제어, 2는 바퀴제어, 3은 모터 속도제어
	static int count = 0;
	receive = UDR1;	


DDRC=0xFF;
PORTC=0xFF;
	if(!control)
	{
		if (receive == 246)				// 정지
			angle_reset();		
		else if(receive >= 11 && receive <= 20)       // 개별 이동 (빠르게)
		{
			motornum = receive;
			if(receive == 19)
				motornum = 0;
			else if (receive == 20)
				motornum = 20;
			control = 1;	
		}			
		else if (receive == 0xFE) // 속도 설정
			control = 3;
		else if(receive >= 21 && receive <= 30) // 개별 토크온
		{
			motornum = receive - 10;
			Torque_on(motornum);
		}
		else if(receive >= 31 && receive <= 40) // 개별 토크오프
		{
			motornum = receive - 20;
			Torque_off(motornum);
		}
		else if (receive == 247)
			scan();
		else if(receive == 240) // 팔 연동
			sync_mode = 1;
		else if(receive == 241) // 팔 연동 해제
			sync_mode = 0;
		else if(receive == 238) // 기울기제어
			control = 2;
		else if(receive == 244) 				// 좌회전
			rotate_left();
		else if (receive == 245)				// 우회전
			rotate_right();
		else if (receive == 243)				// 후진
			backward();
		else if (receive == 242)				// 전진
			forward();
		else if (receive == 248)
			scan_forward();						// 탐색 전진
		else if (receive == 250)				// 전체토크온
			all_torque_on();
		else if (receive == 251)				// 전체 토크오프
			all_torque_off();

	}
	else
	{
		if(control == 1)   // 개별 모터제어
		{
			Control_Pos(motornum, (unsigned int)receive<<2, speed);
			control = 0;
		}
		else if(control == 2)  // 바퀴 전체 속도 제어
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
	PortInitialize();    // 485방향비트를 아웃풋으로 설정하는걸 잊음.
	RS485_RXD; //Set RS485 Direction to Input State.
	SerialInitialize(SERIAL_PORT0,1,RX_INTERRUPT);	//RS485 Initializing(RxInterrupt)
	SerialInitialize(SERIAL_PORT1,DEFAULT_BAUD_RATE,RX_INTERRUPT);	
  	gbRxBufferReadPointer = gbRxBufferWritePointer = 0;  //RS485 RxBuffer Clearing.

	sensor_init();

	_delay_ms(1000);
	sei();
	UDR1 = 0;
	Torque_on(254);
/*원본 
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

		if(TIFR & (1 << OCF0)) // 타이머0 비교일치 플래그 발생
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
	
		if(TIFR & (1 << OCF2)) // 타이머2 비교일치 플래그 발생.
		{
			if(PINE & 0b11100000) // 셋중 하나가 펄스가 들어와있을떄, 이후 수정가능한 부분.
				centi++; // 값을 증가시킴
			else if(sensor_num)           // 펄스가 끊겨있고 sensor_num이 정의되어있는경우
			{
				switch(sensor_num)
				{
				case 1:
					sonic_value1[sonic_index] = centi; // 각 sonic_value에 초음파센서의 값이 저장될거임.
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
			if(!sync_mode) // 팔 연동상태가 아닐때만 데이터를 보냄
			{
				cbi(UCSR1B,7);
				TxD81(255);  // 헤더
				
				sonic_mean_value[0] = mean(sonic_value1);
				sonic_mean_value[1] = mean(sonic_value2);
				sonic_mean_value[2] = mean(sonic_value3);
				TILT_sensor[0] = mean(TILT_value1);
				TILT_sensor[1] = mean(TILT_value2);
				TILT_sensor[2] = mean(TILT_value3);

				TxD81(sonic_mean_value[0]);
				TxD81(sonic_mean_value[1]);
				TxD81(sonic_mean_value[2]);
				TxD81(TILT_sensor[0]); // 틸트
				TxD81(TILT_sensor[1]);
				TxD81(TILT_sensor[2]);
				
				Read_Volt(21); // 볼트 읽음. 단위 0.1V
				if(auto_mode == 0)
				{
					Read_Pos(11); // 7번모터 값을 읽어 전송
					Read_Pos(12); 
					Read_Pos(13); 
					Read_Pos(14);
					Read_Pos(15);
					Read_Pos(16);
		
					Read_Pos(0); // 기울기조절모터값 
				}
				TxD81(254); // 헤더
				sbi(UCSR1B,7);
			}
		
			Tilt_Control(TILT_sensor[1]); // 기울기 측정한뒤 모터제어
		}
			
		switch(sensor_delay) // 초음파를 순차적으로 쏘기 위함.
		{
			case 0:
				if(++sonic_index == SAMPLE_NUM) sonic_index = 0; // 인덱스 초기화
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
			TILT_value1[TILT_index] = ADC_Convert_16(1) >> 2; // 수정 가능한 부분
			TILT_value2[TILT_index] = ADC_Convert_16(2) >> 2;
			TILT_value3[TILT_index] = ADC_Convert_16(3) >> 2;
		}
	}
	
}
