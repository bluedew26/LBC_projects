/////////////////////////////////////////////////////////////////////////////////
// ���ϸ� : dot_matrix.c													   //
// ���� : ��Ʈ��Ʈ������ �����ϴ� �Լ��� ��Ƴ��� ����						   //
/////////////////////////////////////////////////////////////////////////////////

#include "dot_matrix.h"
#include <avr/io.h>

#define DM_COLUMN_PORT	PORTA
#define DM_ROW_PORT		PORTE
#define DM_COLUMN_IO	DDRA
#define DM_ROW_IO		DDRE

volatile static int row = 0;  // ���� �����

static char UP_pattern[6] = {0x18, 0x3c, 0x7E, 0xFF, 0x18, 0x18}; // ���� ȭ��ǥ
static char DOWN_pattern[6] = {0x18, 0x18, 0xFF, 0x7E, 0x3c, 0x18}; // �Ʒ��� ȭ��ǥ
static char NO_pattern[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // �Ʒ��� ȭ��ǥ

static char *DM_pattern = NO_pattern; // ��Ʈ��Ʈ���� ������ �޴� ������. �ʱⰪ�� R ����.

void DM_ON_UP() { DM_pattern = UP_pattern;  row = 0;} // ��Ʈ��Ʈ������ ���� R�� ��� (+ ������� �ʱ�ȭ)
void DM_ON_DOWN() { DM_pattern = DOWN_pattern;  row = 0;} // ��Ʈ��Ʈ������ ���� Y�� ���
void DM_OFF() {DM_pattern = NO_pattern; row = 0;}
/////////////////////////////////////////////////////////////////////////////////////
// �Լ� DM_NextRow()															   //
// ���� : ��������� ���������� �ٲٴ� �� �������� ���������� 2ms�ֱ⸶�� 		   //
//		  �ݺ�ȣ��Ǿ� ��Ʈ��Ʈ���� ������ ǥ���ϴ� ������ �ϴ� �Լ��̴�.		   //
/////////////////////////////////////////////////////////////////////////////////////

void DMInit()
{
	DM_COLUMN_IO = 0xFF;
	DM_ROW_IO = 0xFC; // ���� ��Ʈ 2���� ��ſ� ����.
}


void DM_NextRow() 
{
	if(++row == 6) 		// ���� ������� ������Ŵ (0~7������ ���� ����)
	{
		row = 0;		
	}
	DM_ROW_PORT &= 0x03; // ���� 2��Ʈ�� �����ϰ� �ʱ�ȭ.
	DM_ROW_PORT |= (0x01<<(row + 2));       // row+1��°�� ���
	DM_COLUMN_PORT = ~(DM_pattern[row]); // ���� ������� �������¸� ���� (��0�϶� LED�� ������ ����)
}


	
