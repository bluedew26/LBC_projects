#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Elevator.h"
#include "Algorithm.h"
#include "lcd.h"
#include "dot_matrix.h"
#include "UltraSonic.h"
#include "usart.h"

#define DEFAULT_FLOOR	10 // 사용하지 않는 층의 기본값.
#define DISTANCE_PER_FLOOR	100  // 한층을 이동하는데 발생하는 포토인터럽터의 펄스 수 
#define STANDARD_DUTY	145 // 초기 듀티

struct request req[6] = {{10,0}, {10,0}, {10,0}, {10,0}, {10,0}, {10,0}}; // 요청받은 층과 방향에 대한 정보가 정렬되어 저장되는 구조체 배열
unsigned char index = 0; // 저장된 배열의 최대 인덱스를 나타냄
int target_floor = 0;  // 목적지 (층)
int current_floor = 1; // 현재 층
int g_direction = 0;   // 현재 이동방향. -1 = 내려감, 1 = 올라감, 0 = 정지
unsigned char g_mode = 0; 
// 작동모드 (0이면 정지 및 이동, 1이면 층간 보정중, 2는 캐빈도어 열림, 3은 열린채로 기다리는상태, 4는 캐빈도어 닫힘
unsigned char target_dis; // 초음파센서로 조정시 목표거리값.
unsigned char dis = 0; 	  // 초기 및 조정중 거리값
unsigned char e_stop = 0; // 비상정지 여부

unsigned short global_duty = STANDARD_DUTY; // 듀티값.
unsigned short distance = 0; // 주어진 거리값.
unsigned short dis_per_floor = 0; // 몇 펄스마다 층이 바뀌는가
static volatile unsigned short n_clock = 0;//상승모서리 간 클록 수;
unsigned char timer = 0; // 초기에 빠른 속도로 이동하는 시간
unsigned char floor_dis[6] = {12, 29, 47, 65, 82, 93}; // 각 층에 해당하는 센서값.

ISR(TIMER1_CAPT_vect) // 타이머1 입력캡쳐 ISR
{
	if(e_stop) // 비상정지중일때는
		return; // 아무일도 수행하지 않음.
	if(distance)
	{
		if(distance % dis_per_floor == 5) // 계산된 dis_per_floor 펄스가 지날 때마다
		{
			if(g_direction == 1) // 방향이 올라가는 방향이면
			{
				current_floor++; // 현재층을 +1
			}
			else if(g_direction == -1) // 내려가는 방향이면
			{
				current_floor--; // 현재층을 -1
			}
//			if(distance == 80)
//				SetDuty(70);
		}
		distance--;				// 거리값 감소
	}
	if(timer) // 잠깐동안 빠른 출력을 내는 타이머 관련
	{
		timer--; //  펄스가 들어올때마다 값 감소
		if(timer == 1) // 값이 0에 가까워지면
			SetDuty(global_duty);  // 정상(설정)속도로 돌아감
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
	EIFR = (1<<INTF3); // 플래그 지움
}

void DCMInit() // DC모터 및 엔코더 초기화, ts는 RPM 측정주기
{
// PWM신호 및 모터방향 출력 설정 ////////
	DDRB |= (1<<DDB4);	//PB4(OC0)를 출력으로 설정
	DDRB |= (1<<DDB5);  //PB5(모터방향)을 출력설정 (기본 정회전)
////////////////////////////////////////

////// 타이머/카운터 1 설정, 입력캡쳐용 ////
	TCCR1A = 0x00;	      //표준모드, 출력 핀 미사용
	TCCR1B = (1<<ICES1);  //상승모서리 캡쳐, 타이머 정지
	TIMSK |= (1<<TICIE1);//타이머1 입력캡쳐 인터럽트 허용
	TCCR1B |= 0x02;	 //분주비 8로 타이머 시작 (T1,T2,T3이 분주비 공유함)
//////////////////////////////////////////////

////// 타이머 카운터 0 설정 (PWM및 메인루프용) ////////
	TCCR0 = (1<<WGM01)|(1<<WGM00)|(1<<COM01); // 듀티가 클수록 펄스폭이 커지는 형태. FAST PWM모드.
	TCCR0 |= 0x05;							  // 분주비 128 -> 약 2ms주기로 오버플로우
	OCR0 = STANDARD_DUTY; 					  // 기본 듀티로 설정.
///////////////////////////////////////////////////////


// 비상정지 버튼 정의 (PD3) //////////////////////
	EIMSK |= (1<<INT3); //외부인터럽트 3 허용
    EICRA |= (2<<ISC30); //외부인터럽트 3 하강엣지에서 작동.
/////////////////////////////////////////////////


	Initial_setting(); // 센서를 통해 초기 엘레베이터 위치를 잡는 알고리즘을 수행.
}


void Initial_setting() // 초기상태가 몇층인지 결정하는 함수
{
	char string[20];

	// 초음파 센서값을 받음 //
	pulse(); 
	_delay_ms(100);
	dis = GetDistance();
	/////////////////////////

// 측정된 센서값의 범위에 따라 현재 몇층쯤인지 짐작 ////
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

// target_dis로 해당 층의 중앙부에 해당하는 센서값을 설정 //
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

// 해당 층의 중앙부로 자동 이동. //
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


void E_Down(unsigned char speed) // 엘레베이터 하강
{	
	PORTB |= (1 << PB5);
	OCR0 = speed;
}
void E_Up(unsigned char speed) // 엘레베이터 상승
{
	PORTB &= ~(1 << PB5);
	OCR0 = speed;
}

int GetCurrentFloor() // 현재 위치 반환
{
	return current_floor;
}
int GetDirection() // 현재 방향 반환, -1은 하강, 1은 상승, 0은 정지
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


void Stop() // 엘레베이터 정지.
{
	OCR0 = 0;
}




void AddData(int _floor, int _direction) // 배열에 층값과 방향값을 넣는다.
{
	struct request *temp = BinarySearch(req, index, _floor); // 요청한 층이 이미 다른곳에서 요청되어 있는지 확인

	if(temp) // 이미 그 층에 대한 요청이 존재하면 (리턴값이 있으면)
	{
		if(_direction == 0) // _direction 값이 0, 즉 엘레베이터 내부에서 누른 경우에
			temp->direction = DIR_NONE; // direction을 0으로 만든다.
		else if(temp->direction + _direction == 0) // 기존 방향과 새로 요청된 방향이 다른경우. 즉 올라가기 내려가기가 동시에 눌린경우
			temp->direction = DIR_NONE; // 방향성이 없는 상태인 0으로 만든다. (반드시 멈춰야 하므로)  
		return;
	}
	else // 그 층에 대한 요청이 존재하지 않으면
	{
		if(current_floor == _floor && g_direction == 0) // 요청한 값과 현재 층이 같고 정지상태일경우
			return; // 요청 무시
		req[index].floor = _floor;					   // 다음 인덱스에 요청받은 층과 direction정보를 넣어준다.
		req[index].direction = _direction;
		index++;		// 인덱스 증가
		QuickSort(req, index);					       // 정렬한다.
		if(g_mode != 1) // 모드1 (층간조정중)이 아니라면
		{
			if(g_mode == 0) // 모드0 이라면
			{
			//  여기서는 현재 가고 있는 층보다 더 멀리있지만 우선순위가 높은 층이 요청으로 들어왔을 때 처리 //
				if(BinarySearch(req, index, target_floor)->direction * g_direction == -1) // 1->5(-1), 6, 1층 대책.
				{
					if(target_floor > _floor)
						distance += (target_floor - _floor) * dis_per_floor; // 이동할 펄스값을 좀더 보정해준다
					else
						distance += (_floor - target_floor) * dis_per_floor; // 이동할 펄스값을 좀더 보정해준다
				}
			//////////////////////////////////////////////////////////////////////////////////////////////////
				if(index == 1)		// 아무런 요청도 없는 상태에서 처음 들어온 요청이라면
				{
					target_floor = _floor;		// 다음에 갈 층을 이번에 입력받은 층으로 설정
					GoNextFloor();				// 이동한다.
				}
			}
			SetNextFloor();						// 통상적인 경우 입력을 받으면 다음으로 이동할 층을 갱신함.
		}
	}
}


void DeleteData(int key) // 배열에서 특정 데이터를 지운다.
{
	if(index)
	{
		BinarySearch(req, index, key)->floor = 10; // 지울 데이터의 층을 10으로 만든다. (미사용으로 만듬)
		QuickSort(req, index);		 // 정렬
		index--;					// 인덱스 감소
	}
	return;						// 인덱스가 0일 경우 아무것도 안함.
}
// target = 4
void SetNextFloor() // 다음 목표층 설정
{
	int i=0;
	int do_return = 0; // 리턴을 할것인지 말것인지 함수
	struct request *temp;
	if(req[0].floor == DEFAULT_FLOOR) // 요청 정보가 하나도 없으면
	{
		g_direction = 0; // 방향성을 없앰.
		DM_OFF();
		target_floor = 0; // 타겟층 기본값.
	}
	else // 요청정보가 있으면
	{
		if(g_direction == DIR_UP) // 올라가는 중이면
		{
			for(i=current_floor+1; i<=NUM_FLOOR; i++)
			{
				temp = BinarySearch(req,index,i);// 요청된 층중에 i층이 존재하는지 검색
				if(temp) 
				{
					if(temp->direction == DIR_UP || temp->direction == DIR_NONE) // 방향성이 없거나 올라가는 방향이면
					{
						target_floor = i; // 다음 목표 층을 그곳으로 설정한다. 
						return;
					}
					else if(temp->direction == DIR_DOWN) // 내려가는 방향의 경우 저장만 해둔다. (최상층일수록 우선)
					{
						target_floor = i;
						do_return = 1;
					}	
				}
			}
			if(do_return) // 두 리턴 플래그가 서면 이 밑의 문장을 수행하지 않기로.
				return;
			g_direction = DIR_DOWN; // 아래쪽 방향으로 바꿈 (아래에 존재하지 않으므로 위에 존재한다는의미)
			DM_ON_DOWN();
			SetNextFloor(); // 재귀
			return;
		}
		else if (g_direction == DIR_DOWN) // 내려가는 중이면
		{
			for(i=current_floor-1; i>=1; i--)
			{
				temp = BinarySearch(req,index,i);// 아래층에 i층이 존재하는지 검색
				if(temp) 
				{
					if(temp->direction == DIR_DOWN || temp->direction == DIR_NONE) // 방향성이 없거나 내려가는 방향이면
					{
						target_floor = i; // 다음 목표 층을 그곳으로 설정한다. 
						return; // 브레이크대신 함수종료
					}
					else if(temp->direction == DIR_UP) // 올라가는 방향의 경우 저장만 해둔다. (최하층일수록 우선)
					{
						target_floor = i;
						do_return = 1;
					}	
				}
			}
			// for문을 리턴없이 빠져나오면
			if(do_return) // 두 리턴 플래그가 서면 이 밑의 문장을 수행하지 않기로.
				return;
			g_direction = DIR_UP; // 위쪽 방향으로 바꿈 (아래에 존재하지 않으므로 위에 존재한다는의미)
			DM_ON_UP();
			SetNextFloor(); // 재귀
			return;
		}
		else if (g_direction == DIR_NONE) // 방향성이 없는 상태에서 request가 존재할떄
		{
			int k1 = 1, k2 = req[index-1].floor; // k1초기 최하층, k2초기 최상층 req[index-1].floor
			for(i=current_floor-1; i>=1; i--)
			{
				temp = BinarySearch(req,index,i);// 아래층에 i층이 존재하는지 검색
				if(temp) 
				{
					if(temp->direction == DIR_UP) // 보다 아래층에있는것중에 최초로 올라가는 방향인거
					{
						k1 = i;
						break;
					}
				}
			}
			for(i=current_floor+1; i<=NUM_FLOOR; i++)
			{
				temp = BinarySearch(req,index,i);// 요청된 층중에 i층이 존재하는지 검색
				if(temp)
				{
					if(temp->direction == DIR_DOWN) // 보다 윗층에있는것중에 최초로 내려가는 방향인거
					{
						k2 = i;
						break;
					}
				}
			}
			int d = current_floor + 2*req[index-1].floor - 2*req[0].floor - k2;
			int u = -current_floor + 2*req[index-1].floor - 2*req[0].floor + k1;
			if(d > u) // 위로부터 가는게 더 최단거리일경우
			{
				g_direction = DIR_UP; // 방향 위로 설정.
				DM_ON_UP();
				SetNextFloor(); // 함수 다시 호출
				return;
			}
			else // 아닐경우
			{
				g_direction = DIR_DOWN;
				DM_ON_DOWN();
				SetNextFloor(); // 함수 다시 호출
				return;
			}
		} 
	}
}

void is_arrived() // 목표층에 도달했는지 확인하는 함수.
{
	if(index == 0)
	{
		return;
	}
	if(current_floor == target_floor) // 목표층과 현재층이 일치하면
	{
		DeleteData(target_floor);
		target_floor = 0; // 임시로 0으로 만듬.
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
				distance += DISTANCE_PER_FLOOR * 13 / 14; // 92%만 들어감. 4층부터는 2차선이기 때문
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
				distance += DISTANCE_PER_FLOOR * 12 / 14; // 92%만 들어감. 4층부터는 2차선이기 때문
			else
				distance += DISTANCE_PER_FLOOR;
			
		}
		timer = 20;
		dis_per_floor = distance / (target_floor - current_floor);
		sei();
	}
}


void adjust() // 초음파센서로 조정을 요청하는 함수.
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
	printf("<req구조체 배열 값>\n");
	printf("        req[0]  req[1]  req[2]  req[3]  req[4]  req[5]\n");
	printf("방향 : %6d  %6d  %6d  %6d  %6d  %6d\n", req[0].direction, req[1].direction, req[2].direction,req[3].direction,req[4].direction, req[5].direction);
	printf("층   : %6d  %6d  %6d  %6d  %6d  %6d\n\n\n", req[0].floor, req[1].floor, req[2].floor,req[3].floor,req[4].floor, req[5].floor);
	printf("index : %d\n\n현재방향 = %d\n\n현재층 : %d\n\n다음층 : %d\n\n", index, g_direction, current_floor, target_floor);
	printf("탑승한 사람 수 : %d\n\n도착까지 남은 펄스 : %d\n\n초음파센서값 : %d\n\n듀티값 : %d\n\n\n", people, distance, GetDistance(), global_duty);
}
