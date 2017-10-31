#ifndef __DCMOTOR_H__
#define __DCMOTOR_H__

void DCMInit();
unsigned long GetRPM();
void E_Up(unsigned char speed);
void E_Down(unsigned char speed);
void SetDuty(unsigned char speed);
void Stop();
#endif
