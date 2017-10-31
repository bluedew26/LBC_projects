#include "dynamixel.h"
#include <stdio.h>

/*
TxPacket() send data to RS485.
TxPacket() needs 3 parameter; ID of Dynamixel, Instruction byte, Length of parameters.
TxPacket() return length of Return packet from Dynamixel.
*/
byte TxPacket(byte bID, byte bInstruction, byte bParameterLength) 
{
    byte bCount,bCheckSum,bPacketLength;

    gbpTxBuffer[0] = 0xff;
    gbpTxBuffer[1] = 0xff;
    gbpTxBuffer[2] = bID;
    gbpTxBuffer[3] = bParameterLength+2; //Length(Paramter,Instruction,Checksum)
    gbpTxBuffer[4] = bInstruction;
    for(bCount = 0; bCount < bParameterLength; bCount++)
    {
        gbpTxBuffer[bCount+5] = gbpParameter[bCount];
    }
    bCheckSum = 0;
    bPacketLength = bParameterLength+4+2;
    for(bCount = 2; bCount < bPacketLength-1; bCount++) //except 0xff,checksum
    {
        bCheckSum += gbpTxBuffer[bCount];
    }
    gbpTxBuffer[bCount] = ~bCheckSum; //Writing Checksum with Bit Inversion

    RS485_TXD;
    for(bCount = 0; bCount < bPacketLength; bCount++)
    {
        sbi(UCSR0A,6);//SET_TXD0_FINISH;
        TxD80(gbpTxBuffer[bCount]);
    }
    while(!CHECK_TXD0_FINISH); //Wait until TXD Shift register empty
    RS485_RXD;
    return(bPacketLength);
}

/*
RxPacket() read data from buffer.
RxPacket() need a Parameter; Total length of Return Packet.
RxPacket() return Length of Return Packet.
*/

byte RxPacket(byte bRxPacketLength)
{
#define RX_TIMEOUT_COUNT2   3000L  
#define RX_TIMEOUT_COUNT1  (RX_TIMEOUT_COUNT2*10L)  
  unsigned long ulCounter;
  byte bCount, bLength, bChecksum;
  byte bTimeout;

  bTimeout = 0;
  for(bCount = 0; bCount < bRxPacketLength; bCount++)
  {
    ulCounter = 0;
    while(gbRxBufferReadPointer == gbRxBufferWritePointer)
    {
      if(ulCounter++ > RX_TIMEOUT_COUNT1)
      {
        bTimeout = 1;
        break;
      }
    }
    if(bTimeout) break;
    gbpRxBuffer[bCount] = gbpRxInterruptBuffer[gbRxBufferReadPointer++];
  }
  bLength = bCount;
  bChecksum = 0;

  if(gbpTxBuffer[2] != BROADCASTING_ID)
  {
    if(bTimeout && bRxPacketLength != 255) 
    {
      CLEAR_BUFFER;
    }
    
    if(bLength > 3) //checking is available.
    {
      if(gbpRxBuffer[0] != 0xff || gbpRxBuffer[1] != 0xff ) 
      {	
        CLEAR_BUFFER;
        return 0;
      }
      if(gbpRxBuffer[2] != gbpTxBuffer[2] )
      {
        CLEAR_BUFFER;
        return 0;
      }  
      if(gbpRxBuffer[3] != bLength-4) 
      {
        CLEAR_BUFFER;
        return 0;
      }  
      for(bCount = 2; bCount < bLength; bCount++) bChecksum += gbpRxBuffer[bCount];
      if(bChecksum != 0xff) 
      {
        CLEAR_BUFFER;
        return 0;
      }
    }
  }
  return bLength;
}




/*Hardware Dependent Item*/
#define TXD1_READY			bit_is_set(UCSR1A,5) //(UCSR1A_Bit5)
#define TXD1_DATA			(UDR1)
#define RXD1_READY			bit_is_set(UCSR1A,7)
#define RXD1_DATA			(UDR1)

#define TXD0_READY			bit_is_set(UCSR0A,5)
#define TXD0_DATA			(UDR0)
#define RXD0_READY			bit_is_set(UCSR0A,7)
#define RXD0_DATA			(UDR0)

/*
SerialInitialize() set Serial Port to initial state.
Vide Mega128 Data sheet about Setting bit of register.
SerialInitialize() needs port, Baud rate, Interrupt value.

*/
void SerialInitialize(byte bPort, byte bBaudrate, byte bInterrupt)
{
  if(bPort == SERIAL_PORT0)
  {
    UBRR0H = 0; UBRR0L = bBaudrate; 
    UCSR0A = 0x02;  UCSR0B = 0x18;
    if(bInterrupt&RX_INTERRUPT) sbi(UCSR0B,7); // RxD interrupt enable
    UCSR0C = 0x06; UDR0 = 0xFF;
    sbi(UCSR0A,6);//SET_TXD0_FINISH; // Note. set 1, then 0 is read
  }
  else if(bPort == SERIAL_PORT1)
  {
    UBRR1H = 0; UBRR1L = bBaudrate; 
    UCSR1A = 0x02;  UCSR1B = 0x18;
    if(bInterrupt&RX_INTERRUPT) sbi(UCSR1B,7); // RxD interrupt enable
    UCSR1C = 0x06; UDR1 = 0xFF;
    sbi(UCSR1A,6);//SET_TXD1_FINISH; // Note. set 1, then 0 is read
  }
}


void SerialInit1(unsigned long baud)
{
	unsigned short ubrr;
	


	ubrr = (unsigned short) (F_CPU / (16*baud) - 1);
	UBRR1H = (unsigned char) (ubrr >> 8); // ubrr�� ������Ʈ�� ������Ʈ�� ���� ����
	UBRR1L = (unsigned char) ubrr;
	UCSR1A = 0x02;
	UCSR1B = (1<<RXEN1) | (1<<TXEN1); // RX�� TX�� ���
	UCSR1C = (3<<UCSZ10); // �񵿱�, 1���� ��Ʈ, 8������ ��Ʈ  <<- �������� ����
	sbi(UCSR1B,7);
}

/*
TxD80() send data to USART 0.
*/
void TxD80(byte bTxdData)
{
  while(!TXD0_READY);
  TXD0_DATA = bTxdData;
}

/*
TXD81() send data to USART 1.
*/
void TxD81(byte bTxdData)
{
  while(!TXD1_READY);
  TXD1_DATA = bTxdData;
}

/* 
TxDString() prints data in ACSII code.
*/
void TxDString(byte *bData)
{
  while(*bData)
  {
    TxD8(*bData++);
  }
}

/*
RxD81() read data from UART1.
RxD81() return Read data.
*/
byte RxD81(void)
{
  while(!RXD1_READY);
  return(RXD1_DATA);
}

/*
SIGNAL() UART0 Rx Interrupt - write data to buffer
*/
SIGNAL (SIG_UART0_RECV)
{
  gbpRxInterruptBuffer[(gbRxBufferWritePointer++)] = RXD0_DATA;
}


/////////////////////////////////////

void Motor_control(byte ID, byte direction, unsigned int speed_rate)
{
	byte speed_lb = 0;  // ���� ����Ʈ
	byte speed_hb = 0;  // ���� ����Ʈ

	if(speed_rate < 0 | speed_rate > 100)
		return;  						// invalid value!

	unsigned long speed = (unsigned long)speed_rate*1023 / 100;
	
	if(speed >= 512) { speed_hb += 2; speed-= 512; }
	if(speed >= 256) { speed_hb += 1; speed-= 256; }

	speed_lb = speed;

	if(direction) // ������ CCW��
		speed_hb |= 0x04;		

	gbpParameter[0] = P_GOAL_SPEED_L; //Address of Firmware Version    // ����� ����� �Է� (�Ķ���Ϳ�), ���� ����ϴ°��� TxBuffer��.
	gbpParameter[1] = speed_lb; //Writing Data P_GOAL_SPEED_L
	gbpParameter[2] = speed_hb; //Writing Data P_GOAL_SPEED_H
	bTxPacketLength = TxPacket(ID,INST_WRITE,3);		 //  254 ��ε�ĳ��Ʈ ���� (�ƹ� ���̵� ���� ����)					  // 5���� ��Ŷ�� �����鼭 ���� ����.
}

void Motor_Speed(byte ID, unsigned char speed)
{
	byte speed_lb = 0;  // ���� ����Ʈ
	byte speed_hb = 0;  // ���� ����Ʈ
	unsigned long speed_sum = 0;

	if(speed <= 100) // ������ CCW��
	{
		if(ID > 23)
			speed_hb |= 0x04;
		speed_sum = (unsigned long) speed * 1023 / 100;
	}
	else
	{
		if(ID <= 23)
			speed_hb |= 0x04;
		speed_sum = (unsigned long) (speed - 100) * 1023 / 100;
	}
	if(speed_sum >= 512) { speed_hb += 2; speed_sum-= 512; }
	if(speed_sum >= 256) { speed_hb += 1; speed_sum-= 256; }

	speed_lb = speed_sum;

	gbpParameter[0] = P_GOAL_SPEED_L; //Address of Firmware Version    // ����� ����� �Է� (�Ķ���Ϳ�), ���� ����ϴ°��� TxBuffer��.
	gbpParameter[1] = speed_lb; //Writing Data P_GOAL_SPEED_L
	gbpParameter[2] = speed_hb; //Writing Data P_GOAL_SPEED_H
	bTxPacketLength = TxPacket(ID,INST_WRITE,3);		 //  254 ��ε�ĳ��Ʈ ���� (�ƹ� ���̵� ���� ����)					  // 5���� ��Ŷ�� �����鼭 ���� ����.
}

void Torque_off(byte bID)
{
	gbpParameter[0] = P_TORQUE_ENABLE;
	gbpParameter[1] = 0; //Writing Data
	bTxPacketLength = TxPacket(bID,INST_WRITE,2);
}

void Torque_on(byte bID)
{
	gbpParameter[0] = P_TORQUE_ENABLE;
	gbpParameter[1] = 1; //Writing Data
	bTxPacketLength = TxPacket(bID,INST_WRITE,2);
}

void Read_Pos(byte bID)
{
	byte pos_lb, pos_hb;
	byte pos;

	gbpParameter[0] = P_PRESENT_POSITION_L;
	gbpParameter[1] = 1; //���� ũ��.
	bTxPacketLength = TxPacket(bID,INST_READ,2);
	bRxPacketLength = RxPacket(DEFAULT_RETURN_PACKET_SIZE + 1); // üũ������ + 1��
	pos_lb = gbpRxBuffer[5];

	gbpParameter[0] = P_PRESENT_POSITION_H;
	gbpParameter[1] = 1; //���� ũ��.
	bTxPacketLength = TxPacket(bID,INST_READ,2);
	bRxPacketLength = RxPacket(DEFAULT_RETURN_PACKET_SIZE + 1);
	pos_hb = gbpRxBuffer[5];
	
	pos = ((pos_hb << 8) + pos_lb) / 4;
	if(pos >= 254) pos == 253;	 // 255�� ����� �������� -1��.

	TxD81(pos);
}

byte Read_Pos2(byte bID)
{
	byte pos_lb, pos_hb;
	byte pos;

	gbpParameter[0] = P_PRESENT_POSITION_L;
	gbpParameter[1] = 1; //���� ũ��.
	bTxPacketLength = TxPacket(bID,INST_READ,2);
	bRxPacketLength = RxPacket(DEFAULT_RETURN_PACKET_SIZE + 1); // üũ������ + 1��
	pos_lb = gbpRxBuffer[5];

	gbpParameter[0] = P_PRESENT_POSITION_H;
	gbpParameter[1] = 1; //���� ũ��.
	bTxPacketLength = TxPacket(bID,INST_READ,2);
	bRxPacketLength = RxPacket(DEFAULT_RETURN_PACKET_SIZE + 1);
	pos_hb = gbpRxBuffer[5];
	
	pos = ((pos_hb << 8) + pos_lb) / 4;

	return pos;
}


void Read_Volt(byte bID)
{
	byte volt;

	gbpParameter[0] = P_PRESENT_VOLTAGE;
	gbpParameter[1] = 1; //���� ũ��.
	bTxPacketLength = TxPacket(bID,INST_READ,2);
	bRxPacketLength = RxPacket(DEFAULT_RETURN_PACKET_SIZE + 1); // üũ������ + 1��
	volt = gbpRxBuffer[5];
	if(volt >= 254) volt = 253;
	TxD81(volt);
}

void Control_Pos(byte ID, unsigned int pos, unsigned int speed_rate)  // ���� pos����
{
	byte pos_lb = 0;  // ���� ����Ʈ
	byte pos_hb = 0;  // ���� ����Ʈ
	////////////// �ӵ� ��� ///////////////
	byte speed_lb = 0;  // ���� ����Ʈ
	byte speed_hb = 0;  // ���� ����Ʈ

	unsigned long speed = (unsigned long)speed_rate*1023 / 100;
	
	if(speed >= 512) { speed_hb += 2; speed-= 512; }
	if(speed >= 256) { speed_hb += 1; speed-= 256; }
	speed_lb = speed;

	if(pos >= 512) { pos_hb += 2; pos-= 512; }
	if(pos >= 256) { pos_hb += 1; pos-= 256; }
	pos_lb = pos;

	////////////////////////////////////////

	gbpParameter[0] = P_GOAL_POSITION_L; //Address of Firmware Version 
	gbpParameter[1] = pos_lb;
	gbpParameter[2] = pos_hb;
	gbpParameter[3] = speed_lb;
	gbpParameter[4] = speed_hb;
	bTxPacketLength = TxPacket(ID, INST_WRITE, 5);		 //  254 ��ε�ĳ��Ʈ ���� (�ƹ� ���̵� ���� ����)					  // 5���� ��Ŷ�� �����鼭 ���� ����.
}

extern byte init_offset;
#define TILT_MIN	(225 - init_offset - 5)
#define TILT_MAX	(225 - init_offset + 5)

void Tilt_Control(unsigned int value) // ���� ���� �Լ�.
{
	if(value < TILT_MIN || value > TILT_MAX)
	{
		int pos_lb, pos_hb; // 0������ ���� pos��.
		int pos_lb2, pos_hb2; // 20������ ���� pos��.

/////////////////// 0������ ���� ��ġ �ޱ� ////////////////
		gbpParameter[0] = P_PRESENT_POSITION_L;
		gbpParameter[1] = 1; //���� ũ��.
		bTxPacketLength = TxPacket(0,INST_READ,2);
		bRxPacketLength = RxPacket(DEFAULT_RETURN_PACKET_SIZE + 1); // üũ������ + 1��
		pos_lb = gbpRxBuffer[5];

		gbpParameter[0] = P_PRESENT_POSITION_H;
		gbpParameter[1] = 1; //���� ũ��.
		bTxPacketLength = TxPacket(0,INST_READ,2);
		bRxPacketLength = RxPacket(DEFAULT_RETURN_PACKET_SIZE + 1);
		pos_hb = gbpRxBuffer[5];

///////////////// 20������ ���� ��ġ �ޱ� ///////////////////
		gbpParameter[0] = P_PRESENT_POSITION_L;
		gbpParameter[1] = 1; //���� ũ��.
		bTxPacketLength = TxPacket(20,INST_READ,2);
		bRxPacketLength = RxPacket(DEFAULT_RETURN_PACKET_SIZE + 1); // üũ������ + 1��
		pos_lb2 = gbpRxBuffer[5];

		gbpParameter[0] = P_PRESENT_POSITION_H;
		gbpParameter[1] = 1; //���� ũ��.
		bTxPacketLength = TxPacket(20,INST_READ,2);
		bRxPacketLength = RxPacket(DEFAULT_RETURN_PACKET_SIZE + 1);
		pos_hb2 = gbpRxBuffer[5];
/////////////////////////////////////////////////////////////////
	
		if(value > TILT_MAX)
		{
			pos_lb2 -= (value - TILT_MAX) * 2; // 0�����ʹ� ���ҽ�Ŵ
			pos_lb += (value - TILT_MAX) * 2; // 20�����ʹ� ������Ŵ.
		}
		else if (value < TILT_MIN)
		{
			pos_lb2 += (TILT_MIN - value) * 2;
			pos_lb -= (TILT_MIN - value) * 2;
		}
		// 0������ ���� //
		if(pos_lb < 0)
		{
			pos_lb += 256;
			pos_hb--;
		}
		else if (pos_lb >= 256)
		{
			pos_lb -= 256;
			pos_hb++;
		}

		// 20������ ���� //
		if(pos_lb2 < 0)
		{
			pos_lb2 += 256;
			pos_hb2--;
		}
		else if (pos_lb2 >= 256)
		{
			pos_lb2 -= 256;
			pos_hb2++;
		}

			// 0������ �� 20������ �̵���Ŵ //
		gbpParameter[0] = P_GOAL_POSITION_L; //Address of Firmware Version    // ����� ����� �Է� (�Ķ���Ϳ�), ���� ����ϴ°��� TxBuffer��.
		gbpParameter[1] = pos_lb; //Writing Data P_GOAL_POSITION_L			   // �� �ڷ� ����ؼ� ���� ��ɿ� ���� ���� �Է�
		gbpParameter[2] = pos_hb; //Writing Data P_GOAL_POSITION_H
		gbpParameter[3] = 0x50; //Writing Data P_GOAL_SPEED_L
		gbpParameter[4] = 0x00; //Writing Data P_GOAL_SPEED_H
		bTxPacketLength = TxPacket(0,INST_WRITE,5);

		gbpParameter[1] = pos_lb2; //Writing Data P_GOAL_POSITION_L			   // �� �ڷ� ����ؼ� ���� ��ɿ� ���� ���� �Է�
		gbpParameter[2] = pos_hb2; //Writing Data P_GOAL_POSITION_H
		bTxPacketLength = TxPacket(20,INST_WRITE,5);
	}
//	else
//	{
//		Control_Pos(20, (unsigned int)(128 - (init_offset * 2 / 3))<<2, 25);
//		Control_Pos(0, (unsigned int)(128 + (init_offset * 2 / 3))<<2, 25);
//	}
}
