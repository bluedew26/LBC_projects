#include "Mainloop.h"
#include "dot_matrix.h"
#include "Elevator.h"
#include "stepmotor.h"
#include "UltraSonic.h"
#include <avr/io.h>
#include <avr/interrupt.h>

int pcurrent_floor;			// �������� �����ߴ��� Ȯ���ϱ� ���� ���� ��
unsigned short pdistance;   // 400ms������ ���� �޽����� �����ϴ� ����.
unsigned char print_flag = 0; // �������͹̳η��� ����� ��û�ϴ� �÷���, ����ð��� �ټ� ��⋚���� ���ͷ�Ʈ ���κ��ٴ� main���� ó���Ѵ�.
extern int current_floor;		// ���� ��
extern unsigned char e_stop;    // ��������� ���ȴ��� ����
extern unsigned char g_mode;    // ���� ���������� ��� (0~4)
extern unsigned char target_dis;// ������ ������ ������ �� ��ǥ �Ÿ���
extern unsigned char dis;		// ������ ������ ������ �� ���� �Ÿ���
extern unsigned char people;    // ž�°� ��
extern unsigned short distance; // ���ޱ��� ���� pulse��


void MainTimerInit()
{
	TIMSK |= (1<<TOIE0); // Ÿ�̸�1 �����÷� ���ͷ�Ʈ�� ����Ѵ� (���� Ÿ�̸�1�� PWM��ȣ���̾�����, ��Ȱ���� ��)
}



ISR(TIMER0_OVF_vect)
{
	static unsigned char count1 = 0; 
	static unsigned int  count2 = 0; // 2�ʰ�
	static unsigned int  count3 = 0; // 200ms���� �������͹̳� ��� ������ ���� ī��Ʈ
	static unsigned int  count4 = 0; // 400ms������ ���ޱ��� ���� pulse���� �����Ͽ� ���ϱ� ���� ī��Ʈ
	///////////////////////
	// 200ms���� �ݺ��ϴ� ����
	if(++count3 == 100)
	{
		print_flag = 1; // �������͹̳� ����� ��û�ϴ��÷��׸� ���� (ó���� main����)
		count3 = 0;
	}
	/////////////////////

	// ������� ���� //
	if(e_stop) // ����������϶���
		return; // �������͹̳� ����� �����ϰ� �ƹ��ϵ� ����
	///////////////////

	// 20ms���� �ݺ��ϴ� ���� //
	if(++count1 == 10)
	{
		pulse(); // ������ �߻�
		count1 = 0;
	}
	////////////////////////////

	/// g_mode�� ���� �������� ���� ī��Ʈ ���� //
	if(count2)
		count2--;
	//////////////////////////////////////////////
	
	////////// ��ǥ���� �����ߴ��� ���÷� Ȯ�� /////
	is_arrived();
	////////////////////////////////////////////////

	//// ��Ʈ ��Ʈ���� �� ���� ///////////
	DM_NextRow(); // 2ms�ֱ�� �� ����
	/////////////////////////////////////

///// ���� ���Ҷ� LED ǥ�ÿ� ��ȭ�� �ִ� �κ� /////////////
	if(pcurrent_floor != current_floor)
	{
		pcurrent_floor = current_floor;
		PORTB &= ~(0xCF); // LED�� array �ش��ϴ� ��Ʈ�� �켱 ����.
		switch(pcurrent_floor)
		{
			case 1: PORTB |= 0x01; break;
			case 2: PORTB |= 0x02; break;
			case 3: PORTB |= 0x04; break;
			case 4: PORTB |= 0x08; break;
			case 5: PORTB |= 0x40; break;
			case 6: PORTB |= 0x80; break;
			default: break;
		}	// ���� ���� �ش��ϴ� LED�� ��.
	}
////////////////////////////////////////////


//////////// g_mode ������ ���� �� ////////////////
	if(g_mode == 1) // ������ ��������ϋ�
	{
		dis = GetDistance();  // �����ļ������� ��� �����鼭
		if(dis < target_dis-1) // �Ÿ����� ��ǥ�Ÿ��� (�����ؾ��� ���� �߾���ġ) �αٿ� ��ġ�Ҷ����� ���� �Ʒ��� �̵��Ѵ�.
			E_Up(S_DEF);
		else if (dis > target_dis+1)
			E_Down(S_DEF);
		else                 // ��ǥ�Ÿ����� ��ġ�ϸ�
		{
			Stop(); 		 // �ϴ� ���������͸� ������Ű��
			g_mode = 2; 	 // �����Ϸ�, ĳ�󵵾 ���¸��� ��.
			count2 = 500;	 // 1�ʵ��� ����
		}
	}
	else if(g_mode == 2)
	{
		if(count2 % 4 == 1) // �� 8ms����
			StepmUp(); 		// ĳ�󵵾 ��. (���ܸ��� ��ȸ��)
		if(count2 == 0)
		{
			g_mode = 3; 
			count2 = 1500; // ĳ�󵵾� �����ִ� ���·� 3�� ����
			if(people) 	   // ����� �־�����
				people--;  // ��� �Ѹ� ����
		}
	}
	else if(g_mode == 3)
	{
		if(count2 == 0 && people < 9)  // ������� 9�� �̸��̾�� ���� ����. (9����� �����ʰ�)
		{
			g_mode = 4;				   // ĳ�󵵾 ������ ���
			count2 = 500;			   // 1�ʰ� ����
			
		}
	}
	else if(g_mode == 4)
	{
		if(count2 % 4 == 1) // �� 8ms����
			StepmDn();		// ĳ�󵵾 ���� (���Ǹ��� ��ȸ��)
		if(count2 == 0)	    // ĳ�󵵾 �� ������
		{
			g_mode = 0; 	// ���0���� ����
			SetNextFloor(); // ���� ��ǥ���� ������� �ľ��Ѵ�.
			GoNextFloor();  // ���� ��ǥ������ �̵��Ѵ�.
		}
	}
//////////////////////////////////////////////////////////////
	
	/// 400ms���� �ݺ��ϴ� ���� -> (�ٴ��̳� õ�忡 �����°��� �����ϱ� ���� ��å)
	if(++count4 == 200)
	{
		if(pdistance == distance && GetDirection() != 0 && g_mode == 0) // ���0�� �������°� �ƴԿ��� 400ms���� ��ȭ�� �����ٸ�
		{
			distance = 0;
			current_floor = GetTargetFloor(); // �������� ��ǥ������ ����� ������ is_arrive�� ȣ����� �����ļ����� ������ �����Ѵ�.
		}
		count4 = 0;
		pdistance = distance;
	}
	//////////////////////////

}
