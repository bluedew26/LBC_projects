#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "UltraSonic.h"

static unsigned char sonic_value = 0;	// ���� ������
volatile unsigned int pulse_count = 0;  // ���� �޽� ���̿� ���� ī��Ʈ �Ǵ� ��.

ISR(TIMER2_COMP_vect) // 58us����
{
	pulse_count++;
}

ISR(INT0_vect)
{
	static unsigned char toggle = 0;
    if(toggle == 0) // ī���� �����ϰ� �ܺ� ���ͷ�Ʈ(INT4)�� �������� Falling Edge���� �ɸ����� ��.
    {
        pulse_count = 0; // ������ ���� �Ÿ��� �ʱ�ȭ
        TIMSK |= (1 << OCIE2); // Ÿ�̸�ī���� 2 ����ġ���ͷ�Ʈ ���
        TCNT2 = 0;    // ī��Ʈ �ʱ�ȭ
        EICRA = 0x02; // �ܺ����ͷ�Ʈ0 �ϰ��������� �۵�.
        toggle = 1;
    }
    else // ī���͸� ������Ű�� �ܺ� ���ͷ�Ʈ(INT4)�� �������� Riging Edge���� �ɸ����� ��.
    {
        TIMSK &= ~(1 << OCIE2); // Ÿ�̸� ī����2 ����ġ ���ͷ�Ʈ ����
		sonic_value = pulse_count; // *0.5cm
        EICRA = 0x03; //  �ܺ����ͷ�Ʈ0 ��¿������� �۵�.
		toggle = 0;
    }                          
             
}

void Sonic_Init()
{
	TCCR2 = (1 << WGM21) | (1 << CS21);		 // Ÿ�̸�2 CTC���, OC2�� �̻��, ���ֺ� 8
	OCR2 = 57;       						 // 29us���� Ÿ�̸�0 ����ġ ���ͷ�Ʈ �߻�. (�޽� 58us�� 1cm, 29us�� 0.5cm)
	DDRD |= (1<<PD7);// PD7 ���.

	// ���ͷ�Ʈ
	EIMSK |= (1<<INT0); //�ܺ����ͷ�Ʈ 0 ���
    EICRA |= 0x03; //�ܺ����ͷ�Ʈ 0 ��¿������� �۵�.
}

void pulse()
{
	PORTD |= (1<<PD7); // PD7�� 15us�� �޽��� ��.
	_delay_us(15);
	PORTD &= ~((1<<PD7));
}

unsigned char GetDistance()
{
	return sonic_value;
}
