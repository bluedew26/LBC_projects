#ifndef __ELEVATOR_H__
#define __ELEVATOR_H__

struct request // 사용자의 요청을 나타내는 구조체
{
	int floor;
	int direction; // -1은 내려감. 1은 올라감. 0은 방향성 없음.
};

#define DIR_UP		1  // 올라가는 방향
#define DIR_DOWN	-1 // 내려가는방향
#define DIR_NONE	0  // 방향성이 없음 (엘레베이터 내부에서 누름)
#define NUM_FLOOR	6 // 층 갯수

#define S_DEF		150 // 모터 듀티 디폴트값

void AddData(int _floor, int _direction); // 요청받은 층/방향데이터를 배열에 삽입하는 함수.
void DeleteData(int key);		// 요청받은 데이터를 지우는 함수
void SetNextFloor();			 // 다음 목표층 설정
void GoNextFloor();
void is_arrived(); 				// 목표층에 도달했는지 확인하는 함수.
void Initial_setting();			// 초기 엘레베이터 위치를 조정하는 함수
void DCMInit();			   		// DC모터 초기화
void E_Up(unsigned char speed);  // 해당 듀티값으로 엘레베이터를 상승시키는함수
void E_Down(unsigned char speed); // 해당 듀티값으로 엘레베이터를 하강시키는 함수
void SetDuty(unsigned char speed); // 듀티값만을 변동시키는 함수
void Stop();					 // 엘레베이터 정지
void adjust();					 // 목표층에 도달했을때 다시 위치 조정을 하는 함수
int GetCurrentFloor();			 // 현재 층을 얻음.
int GetDirection();				 // 현재 이동방향을 얻음.
int GetTargetFloor();			 // 다음 목표층을 얻음.

void Print();					// 하이퍼터미널 갱신함수.
#endif
