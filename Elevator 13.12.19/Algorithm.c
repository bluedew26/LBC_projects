#include "Algorithm.h"

#define SWAP(a,b) { int t;t=a;a=b;b=t; } // SWAP 매크로 함수.

///////////////////////////////////////////
//////// QuickSort 정렬 알고리즘 //////////
///////////////////////////////////////////
void QuickSort(struct request *ar, int num) // 퀵소트, 배열, 배열길이
{
     int left,right;
     char key;

     if (num <= 1) return;
     key=ar[num-1].floor; // 기준값 = 배열의 마지막 값
     for (left=0,right=num-2;;left++,right--) 
	 {
        while (ar[left].floor < key) { left++; }
        while (ar[right].floor > key) { right--; }
        if (left >= right) break;            // 좌우가 만날 경우 끝냄
		SWAP(ar[left].floor,ar[right].floor);
        SWAP(ar[left].direction,ar[right].direction);
     }
     SWAP(ar[left].floor,ar[num-1].floor);            // 기준값과 left위치값 교환
     SWAP(ar[left].direction,ar[num-1].direction);    // direction도 함께 교환 (정렬기준은 floor임)
 
     QuickSort(ar,left);                     // 왼쪽 구간 정렬
     QuickSort(ar+left+1,num-left-1);        // 오른쪽 구간 정렬
}

///////////////////////////////////////////
// BinarySearch 탐색 알고리즘 /////////////
///////////////////////////////////////////
struct request* BinarySearch(struct request* arr,int size, int target)
{
	int low=0,high=size-1,mid;
	while(low<=high)
	{
		mid = (low+high)/2;
		if(arr[mid].floor > target) high = mid-1;
		else if (arr[mid].floor < target) low = mid +1;
		else return arr+mid;
	}
	return 0;
}
