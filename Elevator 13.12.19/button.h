//=================================================
// ��ư �Է� ������� button.h
//=================================================
#ifndef __BUTTON_H__
#define __BUTTON_H__

// �Է� ��ư ����
#define BTN_SW0 	0x02		// ����ġ 0�� ����
#define BTN_SW1		0x04		// ����ġ 1�� ����
#define BTN_SW2		0x08		// ����ġ 2�� ����
#define BTN_SW3		0x20		// ����ġ 3�� ����

#define NO_BTN		0x00		// �Է��� ����

#define BTN_MASK	(BTN_SW0 | BTN_SW1 | BTN_SW2 | BTN_SW3)

//�Լ� ����
void BtnInit(void);
unsigned char BtnInput(void);

#endif
