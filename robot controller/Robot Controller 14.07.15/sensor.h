#ifndef __SENSOR_H__
#define __SENSOR_H__

#define SAMPLE_NUM 			 4     // ���� ���� ����
void sensor_init(void);
void SONIC_ON(unsigned char num);

unsigned char maxval(unsigned char* arr);

unsigned char minval(unsigned char* arr);

unsigned char mean(unsigned char* arr); // �ִ� / �ּҰ��� ������ ����� ����

#endif
