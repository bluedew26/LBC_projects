#include "stdafx.h"
#include "functions.h"

COLOR getcolor(IplImage *img, int x, int y)
{
	if (x < 0 || y < 0 || x >= img->width || y >= img->height)
		return COLOR(2, 2, 2);

	int index = y * img->widthStep + (x*img->nChannels);

	COLOR col;
	col.R = img->imageData[index + 2];	// R康开
	col.G = img->imageData[index + 1];	// G康开
	col.B = img->imageData[index + 0];	// B康开

	return col;
}
void setcolor(IplImage *img, int x, int y, unsigned char R, unsigned char G, unsigned char B)
{
	int index = y * img->widthStep + (x*img->nChannels);
	img->imageData[index + 2] = R;	// R康开
	img->imageData[index + 1] = G;	// G康开
	img->imageData[index + 0] = B;	// B康开
}




