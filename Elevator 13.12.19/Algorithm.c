#include "Algorithm.h"

#define SWAP(a,b) { int t;t=a;a=b;b=t; } // SWAP ��ũ�� �Լ�.

///////////////////////////////////////////
//////// QuickSort ���� �˰��� //////////
///////////////////////////////////////////
void QuickSort(struct request *ar, int num) // ����Ʈ, �迭, �迭����
{
     int left,right;
     char key;

     if (num <= 1) return;
     key=ar[num-1].floor; // ���ذ� = �迭�� ������ ��
     for (left=0,right=num-2;;left++,right--) 
	 {
        while (ar[left].floor < key) { left++; }
        while (ar[right].floor > key) { right--; }
        if (left >= right) break;            // �¿찡 ���� ��� ����
		SWAP(ar[left].floor,ar[right].floor);
        SWAP(ar[left].direction,ar[right].direction);
     }
     SWAP(ar[left].floor,ar[num-1].floor);            // ���ذ��� left��ġ�� ��ȯ
     SWAP(ar[left].direction,ar[num-1].direction);    // direction�� �Բ� ��ȯ (���ı����� floor��)
 
     QuickSort(ar,left);                     // ���� ���� ����
     QuickSort(ar+left+1,num-left-1);        // ������ ���� ����
}

///////////////////////////////////////////
// BinarySearch Ž�� �˰��� /////////////
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
