#ifndef __SENSOR_H__
#define __SENSOR_H__

#define SAMPLE_NUM 			 4     // 센서 샘플 갯수
void sensor_init(void);
void SONIC_ON(unsigned char num);

unsigned char maxval(unsigned char* arr);

unsigned char minval(unsigned char* arr);

unsigned char mean(unsigned char* arr); // 최대 / 최소값을 제외한 평균을 구함

#endif
