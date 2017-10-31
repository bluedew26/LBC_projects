#include "func.h"

#ifdef TEST
void setcolor(IplImage *img, int x, int y, unsigned char R, unsigned char G, unsigned char B)
{
	int index = y * img->widthStep + (x*img->nChannels);
	img->imageData[index + 2] = R;	// R����
	img->imageData[index + 1] = G;	// G����
	img->imageData[index + 0] = B;	// B����
}
#endif