#include "dynamixel.h"
#include "wheel_control.h"
#include <util/delay.h>
#define SPEED		35 // ȸ�������� ȸ�����ǵ�
#define ANGLE 		50	// 45�� ȸ���� �ش��ϴ� offset��.
#define GLOBAL_DELAY 12
int speed[6];
byte move_type = 0; // 0 ����, 1 ����, 2 ����, 3 ��ȸ��, 4 ��ȸ��, 5 ��ȸ������, 6 ��ȸ������, 7 ��ȸ������, 8 ��ȸ������
byte move_count = 0;
extern int scanning_count;
void angle_reset()
{
	if(move_type >= 3 || scanning_count)
	{
		_delay_us(100);
		move_type = 1;
		Control_Pos(9, (unsigned int)(INIT_MOTOR9)<<2, SPEED);
		Control_Pos(10, (unsigned int)(INIT_MOTOR10)<<2, SPEED);
		Control_Pos(7, (unsigned int)(INIT_MOTOR7)<<2, SPEED);
		Control_Pos(8, (unsigned int)(INIT_MOTOR8)<<2, SPEED);
		
		
		
		move_count = GLOBAL_DELAY;
	}
	stop();
}

#define FORWARD_SPEED	75
#define BACK_OFFSET		20
#define TURN_SPEED		80
#define OFFSET			30
#define MIDDLE_OFFSET	20
void forward()
{
	if(move_type != 1 && move_type != 2) // ��ݱ��� ���� �� �������� �ƴϾ�����
	{

		angle_reset(); // �ڼ� ����
		move_type = 1; // ������ȣ
		move_count = GLOBAL_DELAY; // ī���� ���
		return;
	} 
	if(move_count == 0)
	{
		speed[0] = FORWARD_SPEED;
		speed[1] = FORWARD_SPEED;
		speed[2] = FORWARD_SPEED;
		speed[3] = FORWARD_SPEED;
		speed[4] = FORWARD_SPEED;
		speed[5] = FORWARD_SPEED;
		for(int i=0; i<6; i++)
			Motor_Speed(21+i, speed[i]); // ID = count, �ӵ����� receive

	}
}

void backward()
{
	if(move_type != 2 && move_type != 1 || scanning_count) // ��ݱ��� �������� �ƴϾ�����
	{
		angle_reset(); // �ڼ� ����
		move_type = 2; // ������ȣ
		move_count = GLOBAL_DELAY; // ī���� ���

		return;

	} 
	if(move_count == 0)
	{
		speed[0] = -(FORWARD_SPEED - BACK_OFFSET);
		speed[1] = -(FORWARD_SPEED - BACK_OFFSET);
		speed[2] = -(FORWARD_SPEED - BACK_OFFSET);
		speed[3] = -(FORWARD_SPEED - BACK_OFFSET);
		speed[4] = -(FORWARD_SPEED - BACK_OFFSET);
		speed[5] = -(FORWARD_SPEED - BACK_OFFSET);
		for(int i=0; i<6; i++)
			Motor_Speed(21+i, speed[i]); // ID = count, �ӵ����� receive
	}
}

void stop()
{	
	for(int i=0; i<6; i++)
		Motor_Speed(21+i, 0); // ID = count, �ӵ����� receive
}

void all_torque_on()
{
	Torque_on(11);
	Torque_on(12);
	Torque_on(13);
	Torque_on(14);
	Torque_on(15);
	Torque_on(16);
}

void all_torque_off()
{
	Torque_off(11);
	Torque_off(12);
	Torque_off(13);
	Torque_off(14);
	Torque_off(15);
	Torque_off(16);
}

void rotate_left()
{
	if(move_type != 4 && move_type != 3) // ��ݱ��� �������� �ƴϾ�����
	{
		_delay_us(100);
		Control_Pos(10, (unsigned int)(INIT_MOTOR10+ANGLE)<<2, SPEED);
		Control_Pos(9, (unsigned int)(INIT_MOTOR9-ANGLE)<<2, SPEED);		
		Control_Pos(7, (unsigned int)(INIT_MOTOR7+ANGLE)<<2, SPEED);
		Control_Pos(8, (unsigned int)(INIT_MOTOR8-ANGLE)<<2, SPEED);

		move_type = 3; // ������ȣ
		move_count = GLOBAL_DELAY; // ī���� ���
		return;
	} 
	if(move_count == 0)
	{
		speed[0] = -(TURN_SPEED - OFFSET);
		speed[1] = -(TURN_SPEED - OFFSET - MIDDLE_OFFSET);
		speed[2] = -(TURN_SPEED - OFFSET);
		speed[3] = TURN_SPEED;
		speed[4] = TURN_SPEED - MIDDLE_OFFSET;
		speed[5] = TURN_SPEED;
		for(int i=0; i<6; i++)
			Motor_Speed(21+i, speed[i]); // ID = count, �ӵ����� receive
	}
}


void rotate_right()
{
	if(move_type != 3 && move_type != 4) // ��ݱ��� �������� �ƴϾ�����
	{
		_delay_us(100);
		Control_Pos(10, (unsigned int)(INIT_MOTOR10+ANGLE)<<2, SPEED);
		Control_Pos(9, (unsigned int)(INIT_MOTOR9-ANGLE)<<2, SPEED);	
		Control_Pos(7, (unsigned int)(INIT_MOTOR7+ANGLE)<<2, SPEED);
		Control_Pos(8, (unsigned int)(INIT_MOTOR8-ANGLE)<<2, SPEED);

		move_type = 4; // ������ȣ
		move_count = GLOBAL_DELAY; // ī���� ���
		return;
	} 
	if(move_count == 0)
	{
		speed[0] = TURN_SPEED;
		speed[1] = TURN_SPEED - MIDDLE_OFFSET;
		speed[2] = TURN_SPEED;
		speed[3] = -(TURN_SPEED - OFFSET);
		speed[4] = -(TURN_SPEED - OFFSET - MIDDLE_OFFSET);
		speed[5] = -(TURN_SPEED - OFFSET);
		for(int i=0; i<6; i++)
			Motor_Speed(21+i, speed[i]); // ID = count, �ӵ����� receive
	}
}

void scan()
{
	stop();
	if(move_type != 5)
	{
		Control_Pos(7, (unsigned int)(INIT_MOTOR7-76)<<2, 10);
		Control_Pos(8, (unsigned int)(INIT_MOTOR8+76)<<2, 10);
		move_type = 5;
	}
}

#define SCANNING_COUNT 80



void scan_forward()
{
	if(move_type != 1 && move_type != 2) // ��ݱ��� ���� �� �������� �ƴϾ�����
	{

		angle_reset(); // �ڼ� ����
		move_type = 1; // ������ȣ
		move_count = GLOBAL_DELAY; // ī���� ���
		scanning_count = SCANNING_COUNT;
		return;
	} 
	if(move_count == 0)
	{
		speed[0] = FORWARD_SPEED;
		speed[1] = FORWARD_SPEED;
		speed[2] = FORWARD_SPEED;
		speed[3] = FORWARD_SPEED;
		speed[4] = FORWARD_SPEED;
		speed[5] = FORWARD_SPEED;
		for(int i=0; i<6; i++)
			Motor_Speed(21+i, speed[i]); // ID = count, �ӵ����� receive
		if(scanning_count == 0)
		{
			Control_Pos(7, (unsigned int)(INIT_MOTOR7-30)<<2, 5);
			Control_Pos(8, (unsigned int)(INIT_MOTOR8+30)<<2, 5);
			scanning_count = SCANNING_COUNT;
		}
		if(scanning_count <= SCANNING_COUNT / 3)
		{
			Control_Pos(7, (unsigned int)(INIT_MOTOR7)<<2, 10);
			Control_Pos(8, (unsigned int)(INIT_MOTOR8)<<2, 10);
		}
	}
}
