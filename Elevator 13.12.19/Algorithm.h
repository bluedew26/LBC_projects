#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__
#include "Elevator.h"


struct request* BinarySearch(struct request* arr,int size, int target); 
// 바이너리 서치, 구조체 배열, 배열길이, 찾을 값

void QuickSort(struct request *ar, int num); 
// 퀵소트, 배열, 배열길이

#endif
