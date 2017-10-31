#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Elevator.h"
#include "Algorithm.h"
#include "lcd.h"
#include "dot_matrix.h"
#include "UltraSonic.h"
#include "usart.h"

#define DEFAULT_FLOOR	10 // ������� �ʴ� ���� �⺻��.
#define DISTANCE_PER_FLOOR	100  // ������ �̵��ϴµ� �߻��ϴ� �������ͷ����� �޽� �� 
#define STANDARD_DUTY	145 // �ʱ� ��Ƽ

struct request req[6] = {{10,0}, {10,0}, {10,0}, {10,0}, {10,0}, {10,0}}; // ��û���� ���� ���⿡ ���� ������ ���ĵǾ� ����Ǵ� ����ü �迭
unsigned char index = 0; // ����� �迭�� �ִ� �ε����� ��Ÿ��
int target_floor = 0;  // ������ (��)
int current_floor = 1; // ���� ��
int g_direction = 0;   // ���� �̵�����. -1 = ������, 1 = �ö�, 0 = ����
unsigned char g_mode = 0; 
// �۵���� (0�̸� ���� �� �̵�, 1�̸� ���� ������, 2�� ĳ�󵵾� ����, 3�� ����ä�� ��ٸ��»���, 4�� ĳ�󵵾� ����
unsigned char target_dis; // �����ļ����� ������ ��ǥ�Ÿ���.
unsigned char dis = 0; 	  // �ʱ� �� ������ �Ÿ���
unsigned char e_stop = 0; // ������� ����

unsigned short global_duty = STANDARD_DUTY; // ��Ƽ��.
unsigned short distance = 0; // �־��� �Ÿ���.
unsigned short dis_per_floor = 0; // �� �޽����� ���� �ٲ�°�
static volatile unsigned short n_clock = 0;//��¸𼭸� �� Ŭ�� ��;
unsigned char timer = 0; // �ʱ⿡ ���� �ӵ��� �̵��ϴ� �ð�
unsigned char floor_dis[6] = {12, 29, 47, 65, 82, 93}; // �� ���� �ش��ϴ� ������.

ISR(TIMER1_CAPT_vect) // Ÿ�̸�1 �Է�ĸ�� ISR
{
	if(e_stop) // ����������϶���
		return; // �ƹ��ϵ� �������� ����.
	if(distance)
	{
		if(distance % dis_per_floor == 5) // ���� dis_per_floor �޽��� ���� ������
		{
			if(g_direction == 1) // ������ �ö󰡴� �����̸�
			{
				current_floor++; // �������� +1
			}
			else if(g_direction == -1) // �������� �����̸�
			{
				current_floor--; // �������� -1
			}
//			if(distance == 80)
//				SetDuty(70);
		}
		distance--;				// �Ÿ��� ����
	}
	if(timer) // ��񵿾� ���� ����� ���� Ÿ�̸� ����
	{
		timer--; //  �޽��� ���ö����� �� ����
		if(timer == 1) // ���� 0�� ���������
			SetDuty(global_duty);  // ����(����)�ӵ��� ���ư�
	}
}

ISR(INT3_vect)
{
	if(e_stop == 0)
	{
		e_stop = 1;;
		Stop();
	}
	else
	{
		e_stop = 0;
		if(g_direction == 1)
		{
			E_Up(240);
			timer = 5;
		}
		else if(g_direction == -1)
		{
			E_Down(240);
			timer = 5;
		}	
	}
	_delay_ms(2);
	while(~PIND & 0x08);
	_delay_ms(2);
	EIFR = (1<<INTF3); // �÷��� ����
}

void DCMInit() // DC���� �� ���ڴ� �ʱ�ȭ, ts�� RPM �����ֱ�
{
// PWM��ȣ �� ���͹��� ��� ���� ////////
	DDRB |= (1<<DDB4);	//PB4(OC0)�� ������� ����
	DDRB |= (1<<DDB5);  //PB5(���͹���)�� ��¼��� (�⺻ ��ȸ��)
////////////////////////////////////////

////// Ÿ�̸�/ī���� 1 ����, �Է�ĸ�Ŀ� ////
	TCCR1A = 0x00;	      //ǥ�ظ��, ��� �� �̻��
	TCCR1B = (1<<ICES1);  //��¸𼭸� ĸ��, Ÿ�̸� ����
	TIMSK |= (1<<TICIE1);//Ÿ�̸�1 �Է�ĸ�� ���ͷ�Ʈ ���
	TCCR1B |= 0x02;	 //���ֺ� 8�� Ÿ�̸� ���� (T1,T2,T3�� ���ֺ� ������)
//////////////////////////////////////////////

////// Ÿ�̸� ī���� 0 ���� (PWM�� ���η�����) ////////
	TCCR0 = (1<<WGM01)|(1<<WGM00)|(1<<COM01); // ��Ƽ�� Ŭ���� �޽����� Ŀ���� ����. FAST PWM���.
	TCCR0 |= 0x05;							  // ���ֺ� 128 -> �� 2ms�ֱ�� �����÷ο�
	OCR0 = STANDARD_DUTY; 					  // �⺻ ��Ƽ�� ����.
///////////////////////////////////////////////////////


// ������� ��ư ���� (PD3) //////////////////////
	EIMSK |= (1<<INT3); //�ܺ����ͷ�Ʈ 3 ���
    EICRA |= (2<<ISC30); //�ܺ����ͷ�Ʈ 3 �ϰ��������� �۵�.
/////////////////////////////////////////////////


	Initial_setting(); // ������ ���� �ʱ� ���������� ��ġ�� ��� �˰����� ����.
}


void Initial_setting() // �ʱ���°� �������� �����ϴ� �Լ�
{
	char string[20];

	// ������ �������� ���� //
	pulse(); 
	_delay_ms(100);
	dis = GetDistance();
	/////////////////////////

// ������ �������� ������ ���� ���� ���������� ���� ////
	if(dis > 85)
		current_floor = 6;
	else if (dis > 75)
		current_floor = 5;
	else if (dis > 55)
		current_floor = 4;
	else if (dis > 38)
		current_floor = 3;
	else if (dis > 18)
		current_floor = 2;
	else
		current_floor = 1;
	target_floor = current_floor;

// target_dis�� �ش� ���� �߾Ӻο� �ش��ϴ� �������� ���� //
	switch(current_floor)
	{
	case 1:
		target_dis = floor_dis[0];
		break;
	case 2:
		target_dis = floor_dis[1];
		break;
	case 3:
		target_dis = floor_dis[2];
		break;
	case 4:
		target_dis = floor_dis[3];
		break;
	case 5:
		target_dis = floor_dis[4];
		break;
	case 6:
		target_dis = floor_dis[5];
		break;
	default:
		break;
	}

// �ش� ���� �߾Ӻη� �ڵ� �̵�. //
	while(1)
	{
		pulse();
		_delay_ms(30);
		dis = GetDistance();
		if(dis < target_dis-1)
			E_Up(S_DEF);
		else if (dis > target_dis+1)
			E_Down(S_DEF);
		else
		{
			Stop();
			LcdCommand(ALLCLR);
			return;
		}
		LcdMove(0,0);
		sprintf(string, "targetdis : %dcm ", target_dis);
		LcdPuts(string);
		LcdMove(1,0);
		sprintf(string, "Distance : %dcm", GetDistance());
		LcdPuts(string);
	}
//////////////////////////////
}


void E_Down(unsigned char speed) // ���������� �ϰ�
{	
	PORTB |= (1 << PB5);
	OCR0 = speed;
}
void E_Up(unsigned char speed) // ���������� ���
{
	PORTB &= ~(1 << PB5);
	OCR0 = speed;
}

int GetCurrentFloor() // ���� ��ġ ��ȯ
{
	return current_floor;
}
int GetDirection() // ���� ���� ��ȯ, -1�� �ϰ�, 1�� ���, 0�� ����
{
	return g_direction;
}
int GetTargetFloor()
{
	return target_floor;
}

void SetDuty(unsigned char speed)
{
	OCR0 = speed;
}


void Stop() // ���������� ����.
{
	OCR0 = 0;
}




void AddData(int _floor, int _direction) // �迭�� ������ ���Ⱚ�� �ִ´�.
{
	struct request *temp = BinarySearch(req, index, _floor); // ��û�� ���� �̹� �ٸ������� ��û�Ǿ� �ִ��� Ȯ��

	if(temp) // �̹� �� ���� ���� ��û�� �����ϸ� (���ϰ��� ������)
	{
		if(_direction == 0) // _direction ���� 0, �� ���������� ���ο��� ���� ��쿡
			temp->direction = DIR_NONE; // direction�� 0���� �����.
		else if(temp->direction + _direction == 0) // ���� ����� ���� ��û�� ������ �ٸ����. �� �ö󰡱� �������Ⱑ ���ÿ� �������
			temp->direction = DIR_NONE; // ���⼺�� ���� ������ 0���� �����. (�ݵ�� ����� �ϹǷ�)  
		return;
	}
	else // �� ���� ���� ��û�� �������� ������
	{
		if(current_floor == _floor && g_direction == 0) // ��û�� ���� ���� ���� ���� ���������ϰ��
			return; // ��û ����
		req[index].floor = _floor;					   // ���� �ε����� ��û���� ���� direction������ �־��ش�.
		req[index].direction = _direction;
		index++;		// �ε��� ����
		QuickSort(req, index);					       // �����Ѵ�.
		if(g_mode != 1) // ���1 (����������)�� �ƴ϶��
		{
			if(g_mode == 0) // ���0 �̶��
			{
			//  ���⼭�� ���� ���� �ִ� ������ �� �ָ������� �켱������ ���� ���� ��û���� ������ �� ó�� //
				if(BinarySearch(req, index, target_floor)->direction * g_direction == -1) // 1->5(-1), 6, 1�� ��å.
				{
					if(target_floor > _floor)
						distance += (target_floor - _floor) * dis_per_floor; // �̵��� �޽����� ���� �������ش�
					else
						distance += (_floor - target_floor) * dis_per_floor; // �̵��� �޽����� ���� �������ش�
				}
			//////////////////////////////////////////////////////////////////////////////////////////////////
				if(index == 1)		// �ƹ��� ��û�� ���� ���¿��� ó�� ���� ��û�̶��
				{
					target_floor = _floor;		// ������ �� ���� �̹��� �Է¹��� ������ ����
					GoNextFloor();				// �̵��Ѵ�.
				}
			}
			SetNextFloor();						// ������� ��� �Է��� ������ �������� �̵��� ���� ������.
		}
	}
}


void DeleteData(int key) // �迭���� Ư�� �����͸� �����.
{
	if(index)
	{
		BinarySearch(req, index, key)->floor = 10; // ���� �������� ���� 10���� �����. (�̻������ ����)
		QuickSort(req, index);		 // ����
		index--;					// �ε��� ����
	}
	return;						// �ε����� 0�� ��� �ƹ��͵� ����.
}
// target = 4
void SetNextFloor() // ���� ��ǥ�� ����
{
	int i=0;
	int do_return = 0; // ������ �Ұ����� �������� �Լ�
	struct request *temp;
	if(req[0].floor == DEFAULT_FLOOR) // ��û ������ �ϳ��� ������
	{
		g_direction = 0; // ���⼺�� ����.
		DM_OFF();
		target_floor = 0; // Ÿ���� �⺻��.
	}
	else // ��û������ ������
	{
		if(g_direction == DIR_UP) // �ö󰡴� ���̸�
		{
			for(i=current_floor+1; i<=NUM_FLOOR; i++)
			{
				temp = BinarySearch(req,index,i);// ��û�� ���߿� i���� �����ϴ��� �˻�
				if(temp) 
				{
					if(temp->direction == DIR_UP || temp->direction == DIR_NONE) // ���⼺�� ���ų� �ö󰡴� �����̸�
					{
						target_floor = i; // ���� ��ǥ ���� �װ����� �����Ѵ�. 
						return;
					}
					else if(temp->direction == DIR_DOWN) // �������� ������ ��� ���常 �صд�. (�ֻ����ϼ��� �켱)
					{
						target_floor = i;
						do_return = 1;
					}	
				}
			}
			if(do_return) // �� ���� �÷��װ� ���� �� ���� ������ �������� �ʱ��.
				return;
			g_direction = DIR_DOWN; // �Ʒ��� �������� �ٲ� (�Ʒ��� �������� �����Ƿ� ���� �����Ѵٴ��ǹ�)
			DM_ON_DOWN();
			SetNextFloor(); // ���
			return;
		}
		else if (g_direction == DIR_DOWN) // �������� ���̸�
		{
			for(i=current_floor-1; i>=1; i--)
			{
				temp = BinarySearch(req,index,i);// �Ʒ����� i���� �����ϴ��� �˻�
				if(temp) 
				{
					if(temp->direction == DIR_DOWN || temp->direction == DIR_NONE) // ���⼺�� ���ų� �������� �����̸�
					{
						target_floor = i; // ���� ��ǥ ���� �װ����� �����Ѵ�. 
						return; // �극��ũ��� �Լ�����
					}
					else if(temp->direction == DIR_UP) // �ö󰡴� ������ ��� ���常 �صд�. (�������ϼ��� �켱)
					{
						target_floor = i;
						do_return = 1;
					}	
				}
			}
			// for���� ���Ͼ��� ����������
			if(do_return) // �� ���� �÷��װ� ���� �� ���� ������ �������� �ʱ��.
				return;
			g_direction = DIR_UP; // ���� �������� �ٲ� (�Ʒ��� �������� �����Ƿ� ���� �����Ѵٴ��ǹ�)
			DM_ON_UP();
			SetNextFloor(); // ���
			return;
		}
		else if (g_direction == DIR_NONE) // ���⼺�� ���� ���¿��� request�� �����ҋ�
		{
			int k1 = 1, k2 = req[index-1].floor; // k1�ʱ� ������, k2�ʱ� �ֻ��� req[index-1].floor
			for(i=current_floor-1; i>=1; i--)
			{
				temp = BinarySearch(req,index,i);// �Ʒ����� i���� �����ϴ��� �˻�
				if(temp) 
				{
					if(temp->direction == DIR_UP) // ���� �Ʒ������ִ°��߿� ���ʷ� �ö󰡴� �����ΰ�
					{
						k1 = i;
						break;
					}
				}
			}
			for(i=current_floor+1; i<=NUM_FLOOR; i++)
			{
				temp = BinarySearch(req,index,i);// ��û�� ���߿� i���� �����ϴ��� �˻�
				if(temp)
				{
					if(temp->direction == DIR_DOWN) // ���� �������ִ°��߿� ���ʷ� �������� �����ΰ�
					{
						k2 = i;
						break;
					}
				}
			}
			int d = current_floor + 2*req[index-1].floor - 2*req[0].floor - k2;
			int u = -current_floor + 2*req[index-1].floor - 2*req[0].floor + k1;
			if(d > u) // ���κ��� ���°� �� �ִܰŸ��ϰ��
			{
				g_direction = DIR_UP; // ���� ���� ����.
				DM_ON_UP();
				SetNextFloor(); // �Լ� �ٽ� ȣ��
				return;
			}
			else // �ƴҰ��
			{
				g_direction = DIR_DOWN;
				DM_ON_DOWN();
				SetNextFloor(); // �Լ� �ٽ� ȣ��
				return;
			}
		} 
	}
}

void is_arrived() // ��ǥ���� �����ߴ��� Ȯ���ϴ� �Լ�.
{
	if(index == 0)
	{
		return;
	}
	if(current_floor == target_floor) // ��ǥ���� �������� ��ġ�ϸ�
	{
		DeleteData(target_floor);
		target_floor = 0; // �ӽ÷� 0���� ����.
		SetNextFloor();
		adjust();
	}
}

void GoNextFloor()
{
	if(target_floor == 0)
		return;
	if(current_floor > target_floor)
	{
		E_Down(240);
		cli();
		int i;
		for(i=target_floor + 1; i<=current_floor; i++)
		{
			if(current_floor >= 5 && target_floor >= 4)
			{
				distance += DISTANCE_PER_FLOOR * 11 / 14;
				continue;
			}
			if(i >= 4)
				distance += DISTANCE_PER_FLOOR * 13 / 14; // 92%�� ��. 4�����ʹ� 2�����̱� ����
			else
				distance += DISTANCE_PER_FLOOR;
		}
		distance -= (current_floor - target_floor - 1) * DISTANCE_PER_FLOOR / 7;
		dis_per_floor = distance / (current_floor - target_floor);
		timer = 20;
		sei();
	}
	else if(current_floor == target_floor)
		return;
	else
	{
		E_Up(240);
		cli();
		distance = 0;
		int i;
		for(i=current_floor + 1; i<=target_floor; i++)
		{
			if(target_floor == 5)
			{
				distance += DISTANCE_PER_FLOOR * 11 / 14;
				continue;
			}
			if(i >= 4)
				distance += DISTANCE_PER_FLOOR * 12 / 14; // 92%�� ��. 4�����ʹ� 2�����̱� ����
			else
				distance += DISTANCE_PER_FLOOR;
			
		}
		timer = 20;
		dis_per_floor = distance / (target_floor - current_floor);
		sei();
	}
}


void adjust() // �����ļ����� ������ ��û�ϴ� �Լ�.
{
	dis = GetDistance();
	switch(current_floor)
	{
	case 1:
		target_dis = floor_dis[0];
		break;
	case 2:
		target_dis = floor_dis[1];
		break;
	case 3:
		target_dis = floor_dis[2];
		break;
	case 4:
		target_dis = floor_dis[3];
		break;
	case 5:
		target_dis = floor_dis[4];
		break;
	case 6:
		target_dis = floor_dis[5];
		break;
	default:
		break;
	}
	g_mode = 1;
}
extern unsigned char people;
void Print()
{
	printf("<req����ü �迭 ��>\n");
	printf("        req[0]  req[1]  req[2]  req[3]  req[4]  req[5]\n");
	printf("���� : %6d  %6d  %6d  %6d  %6d  %6d\n", req[0].direction, req[1].direction, req[2].direction,req[3].direction,req[4].direction, req[5].direction);
	printf("��   : %6d  %6d  %6d  %6d  %6d  %6d\n\n\n", req[0].floor, req[1].floor, req[2].floor,req[3].floor,req[4].floor, req[5].floor);
	printf("index : %d\n\n������� = %d\n\n������ : %d\n\n������ : %d\n\n", index, g_direction, current_floor, target_floor);
	printf("ž���� ��� �� : %d\n\n�������� ���� �޽� : %d\n\n�����ļ����� : %d\n\n��Ƽ�� : %d\n\n\n", people, distance, GetDistance(), global_duty);
}
