#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__
#include "Elevator.h"


struct request* BinarySearch(struct request* arr,int size, int target); 
// ���̳ʸ� ��ġ, ����ü �迭, �迭����, ã�� ��

void QuickSort(struct request *ar, int num); 
// ����Ʈ, �迭, �迭����

#endif
