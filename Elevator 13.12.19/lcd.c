//lcd.c : LCD �����Լ��� ����
// ���ϸ� : lcd.c
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

//LCD ��Ʈ �ּ�
#define LCD_PORT  PORTC
#define LCD_DDR   DDRC

//���� �Լ�
static void checkbusy(void);
static void write_command(char command);
static void write_data(char ch);

//��� : LCD Display�� �ʱ�ȭ�Ѵ�.
void LcdInit(void)  {
        LCD_DDR = 0xFF;       //LCD��Ʈ�� ������� ����
	_delay_ms(15);
	write_command(0x30);
	_delay_ms(5);
	write_command(0x30);
	_delay_ms(1);
	write_command(0x32);

	LcdCommand(FUNSET);
	LcdCommand(DISP_ON);
	LcdCommand(ALLCLR);
	LcdCommand(ENTMOD);

	LcdCommand(DISP_ON);   //ȭ���� �Ҵ�.
}

//LCD�� ����� ����ϴ� �Լ�
//�Է� : command - LCD�� ������ ���
//                 lcd.h�� ���ǵ� ����� ����� ��
void LcdCommand(char command)  {

    checkbusy();
    write_command(command);
}

//������ġ�� ���� �ϳ��� ����Ѵ�.
//�Է� : ch - ȭ�鿡 �� ���� �ڵ�
void LcdPutchar(char ch)  {  
	checkbusy();
	write_data(ch);
}

//���� ��ġ�� ���ڿ��� ����Ѵ�.
//�Է� : str - ����� ���ڿ�
void LcdPuts(char* str)  {
    while(*str)  {                 //*str�� NULL���ڰ� �ƴϸ� ������ ����.
	    LcdPutchar(*str);      //���� *str�� ȭ�鿡 ���
	    str++;                 //str�� ���� ���ڸ� ����Ŵ
	}
}

//���ڸ� �� ��ġ�� ������ ��ġ(line, pos)�� �̵���Ų��.
//�Է� : line - ȭ���� ��(0����� ����)
//       pos - ȭ���� ��(0������ ����)
void LcdMove(char line, char pos)  {   
        char addr;

	addr = (line << 6) + pos;
	addr |= 0x80;                //��Ʈ 7�� ��Ʈ�Ѵ�.
	LcdCommand(addr);
}

//���ο� �۲��� �����Ѵ�.
//�Է� : ch - ����Ҽ� �ִ� �ڵ尪
//       font - �۲� �迭
int LcdNewchar(char ch, char font[])  {
    int i;
    if(ch > 0x07) { return -1; }  //�۰��� ����� �� ����            	                              
    ch <<= 3;                     //ch = ch << 3;�� ����
    ch |= 0x40;                   //��Ʈ 6�� ��Ʈ => CGRAM �ּҼ��� ���

    LcdCommand(ch);               //CGRAM �ּҼ��� => LcdPutchar()�� 
	                          //���� ���ڴ� CGRAM�� ����
    for(i = 0 ; i < 8 ; i++){     //�۲��� CGRAM�� ����
        LcdPutchar(font[i]);
    }    
    LcdMove(0, 0);                //������ LcdPutchar()��
	                          //���� ���ڴ� ȭ�鿡 ǥ��
    return 0;                     //�۲� ��� ����
}

//��� �������Ϳ� ����� ����.
//�Է� : command - LCD�� ������ ��� �ڵ�
static void write_command(char command)  {
    char temp;
    //���� �Ϻ� ���
    temp = (command & 0xF0) | 0x04;  //0x04 : RS = 0 (���)
	                             //RW = 0 (����), E = 1
    LCD_PORT = temp;
    LCD_PORT = temp & ~0x04;         //E = 0
    //���� �Ϻ� ���
    temp = (command << 4) | 0x04;    //0x04 : RS = 0 (���)
	                             //RW = 0 (����), E = 1
    LCD_PORT = temp;
    LCD_PORT = temp & ~0x04;         //E = 0
}

//������ �������Ϳ� ����� ����.
//�Է� : ch - LCD�� �� ������
static void write_data(char ch)  {

    unsigned char temp;
    
    //���� �Ϻ� ���
    temp = (ch & 0xF0) | 0x05;       //0x05 : RS = 1 (������)
	                             //RW = 0 (����), E = 1
    LCD_PORT = temp;
    LCD_PORT = temp & ~0x04;         //E = 0
    
    //���� �Ϻ� ���
    temp = (ch << 4) | 0x05;        //0x05 : RS = 1 (������)
	                            //RW = 0 (����), E = 1
    LCD_PORT = temp;
    LCD_PORT = temp & ~0x04;        //E = 0
}

//1msec�� �ð��������� BF �÷��� �˻縦 ��ü
static void checkbusy()  {
    _delay_ms(1);
    return;
}
