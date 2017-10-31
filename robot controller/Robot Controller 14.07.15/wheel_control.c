#include "dynamixel.h"
#include "wheel_control.h"
#include <util/delay.h>
#define SPEED		35 // 회전모터의 회전스피드
#define ANGLE 		50	// 45도 회전에 해당하는 offset값.
#define GLOBAL_DELAY 12
int speed[6];
byte move_type = 0; // 0 정지, 1 전진, 2 후진, 3 좌회전, 4 우회전, 5 좌회전전진, 6 우회전전진, 7 좌회전후진, 8 우회전후진
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
	if(move_type != 1 && move_type != 2) // 방금까지 전진 및 후진중이 아니었으면
	{

		angle_reset(); // 자세 리셋
		move_type = 1; // 전진신호
		move_count = GLOBAL_DELAY; // 카운팅 대기
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
			Motor_Speed(21+i, speed[i]); // ID = count, 속도값은 receive

	}
}

void backward()
{
	if(move_type != 2 && move_type != 1 || scanning_count) // 방금까지 전진중이 아니었으면
	{
		angle_reset(); // 자세 리셋
		move_type = 2; // 전진신호
		move_count = GLOBAL_DELAY; // 카운팅 대기

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
			Motor_Speed(21+i, speed[i]); // ID = count, 속도값은 receive
	}
}

void stop()
{	
	for(int i=0; i<6; i++)
		Motor_Speed(21+i, 0); // ID = count, 속도값은 receive
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
	if(move_type != 4 && move_type != 3) // 방금까지 전진중이 아니었으면
	{
		_delay_us(100);
		Control_Pos(10, (unsigned int)(INIT_MOTOR10+ANGLE)<<2, SPEED);
		Control_Pos(9, (unsigned int)(INIT_MOTOR9-ANGLE)<<2, SPEED);		
		Control_Pos(7, (unsigned int)(INIT_MOTOR7+ANGLE)<<2, SPEED);
		Control_Pos(8, (unsigned int)(INIT_MOTOR8-ANGLE)<<2, SPEED);

		move_type = 3; // 전진신호
		move_count = GLOBAL_DELAY; // 카운팅 대기
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
			Motor_Speed(21+i, speed[i]); // ID = count, 속도값은 receive
	}
}


void rotate_right()
{
	if(move_type != 3 && move_type != 4) // 방금까지 전진중이 아니었으면
	{
		_delay_us(100);
		Control_Pos(10, (unsigned int)(INIT_MOTOR10+ANGLE)<<2, SPEED);
		Control_Pos(9, (unsigned int)(INIT_MOTOR9-ANGLE)<<2, SPEED);	
		Control_Pos(7, (unsigned int)(INIT_MOTOR7+ANGLE)<<2, SPEED);
		Control_Pos(8, (unsigned int)(INIT_MOTOR8-ANGLE)<<2, SPEED);

		move_type = 4; // 전진신호
		move_count = GLOBAL_DELAY; // 카운팅 대기
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
			Motor_Speed(21+i, speed[i]); // ID = count, 속도값은 receive
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
	if(move_type != 1 && move_type != 2) // 방금까지 전진 및 후진중이 아니었으면
	{

		angle_reset(); // 자세 리셋
		move_type = 1; // 전진신호
		move_count = GLOBAL_DELAY; // 카운팅 대기
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
			Motor_Speed(21+i, speed[i]); // ID = count, 속도값은 receive
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
