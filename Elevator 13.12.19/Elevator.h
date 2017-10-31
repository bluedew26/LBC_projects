#ifndef __ELEVATOR_H__
#define __ELEVATOR_H__

struct request // ������� ��û�� ��Ÿ���� ����ü
{
	int floor;
	int direction; // -1�� ������. 1�� �ö�. 0�� ���⼺ ����.
};

#define DIR_UP		1  // �ö󰡴� ����
#define DIR_DOWN	-1 // �������¹���
#define DIR_NONE	0  // ���⼺�� ���� (���������� ���ο��� ����)
#define NUM_FLOOR	6 // �� ����

#define S_DEF		150 // ���� ��Ƽ ����Ʈ��

void AddData(int _floor, int _direction); // ��û���� ��/���ⵥ���͸� �迭�� �����ϴ� �Լ�.
void DeleteData(int key);		// ��û���� �����͸� ����� �Լ�
void SetNextFloor();			 // ���� ��ǥ�� ����
void GoNextFloor();
void is_arrived(); 				// ��ǥ���� �����ߴ��� Ȯ���ϴ� �Լ�.
void Initial_setting();			// �ʱ� ���������� ��ġ�� �����ϴ� �Լ�
void DCMInit();			   		// DC���� �ʱ�ȭ
void E_Up(unsigned char speed);  // �ش� ��Ƽ������ ���������͸� ��½�Ű���Լ�
void E_Down(unsigned char speed); // �ش� ��Ƽ������ ���������͸� �ϰ���Ű�� �Լ�
void SetDuty(unsigned char speed); // ��Ƽ������ ������Ű�� �Լ�
void Stop();					 // ���������� ����
void adjust();					 // ��ǥ���� ���������� �ٽ� ��ġ ������ �ϴ� �Լ�
int GetCurrentFloor();			 // ���� ���� ����.
int GetDirection();				 // ���� �̵������� ����.
int GetTargetFloor();			 // ���� ��ǥ���� ����.

void Print();					// �������͹̳� �����Լ�.
#endif
