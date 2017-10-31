#ifndef __WHEEL_H__
#define __WHEEL_H__

/// ���� ȸ������ �ʱⰪ ///
#define INIT_MOTOR7		107 // ����1 ��ȸ�� �ð�
#define INIT_MOTOR8		131 // ����2 ��ȸ�� �ð�
#define INIT_MOTOR9		106 // ����1 �ݽð� 
#define INIT_MOTOR10	110 // ����2 �ݽð�
// �ݽð谡 �÷����� ///
// 45���� 38.4�� �ش� //

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
