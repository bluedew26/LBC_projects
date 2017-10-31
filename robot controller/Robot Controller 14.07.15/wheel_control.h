#ifndef __WHEEL_H__
#define __WHEEL_H__

/// 바퀴 회전모터 초기값 ///
#define INIT_MOTOR7		107 // 앞쪽1 좌회전 시계
#define INIT_MOTOR8		131 // 앞쪽2 좌회전 시계
#define INIT_MOTOR9		106 // 뒤쪽1 반시계 
#define INIT_MOTOR10	110 // 뒤쪽2 반시계
// 반시계가 플러스임 ///
// 45도는 38.4에 해당 //

void forward();
void backward();
void stop();
void all_torque_on();
void all_torque_off();
void angle_reset();
void rotate_left();
void rotate_right();
void scan();
void scan_forward();
#endif
