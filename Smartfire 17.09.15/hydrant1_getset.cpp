#include "stdafx.h"
#include "hydrant.h"
#include "functions.h"


bool output::ishydrant(IplImage *img, int x, int y, int type)
{
	COLOR ref_color = getcolor(img, x, y);
	unsigned char R, G, B;	R = ref_color.R;	G = ref_color.G;	B = ref_color.B;
	if (R == C_HYDRANT[type].R && G == C_HYDRANT[type].G && B == C_HYDRANT[type].B)
	{
		return true;
	}
	return false;
}
bool output::iswhite(IplImage *img, int x, int y)
{
	COLOR ref_color = getcolor(img, x, y);
	unsigned char R, G, B;	R = ref_color.R;	G = ref_color.G;	B = ref_color.B;
	if (R >= 250 && G >= 250 && B >= 250)
	{
		return true;
	}
	return false;
}

bool output::iswall(IplImage *img, int x, int y) // 연두색
{
	COLOR ref_color = getcolor(img, x, y);
	unsigned char R, G, B;	R = ref_color.R;	G = ref_color.G;	B = ref_color.B;
	if (R == C_WALL.R && G == C_WALL.G && B == C_WALL.B)
	{
		return true;
	}
	return false;
}
bool output::isobstacle(IplImage *img, int x, int y, int layernum) // 빨강색
{
	COLOR ref_color = getcolor(img, x, y);
	unsigned char R, G, B;	R = ref_color.R;	G = ref_color.G;	B = ref_color.B;
	if (R == C_OBSTACLE[layernum].R && G == C_OBSTACLE[layernum].G && B == C_OBSTACLE[layernum].B)
	{
		return true;
	}
	return false;
}
bool output::isstair(IplImage *img, int x, int y) // 계단
{
	COLOR ref_color = getcolor(img, x, y);
	unsigned char R, G, B;	R = ref_color.R;	G = ref_color.G;	B = ref_color.B;
	if (R == C_STAIR.R && G == C_STAIR.G && B == C_STAIR.B)
	{
		return true;
	}
	return false;
}
bool output::isscale(IplImage *img, int x, int y) // 계단
{
	COLOR ref_color = getcolor(img, x, y);
	unsigned char R, G, B;	R = ref_color.R;	G = ref_color.G;	B = ref_color.B;
	if (R == C_SCALE.R && G == C_SCALE.G && B == C_SCALE.B)
	{
		return true;
	}
	return false;
}

bool output::isequipment(IplImage *img, int x, int y) // 사다리
{
	COLOR ref_color = getcolor(img, x, y);
	unsigned char R, G, B;	R = ref_color.R;	G = ref_color.G;	B = ref_color.B;
	if (R == C_EQUIPMENT.R && G == C_EQUIPMENT.G && B == C_EQUIPMENT.B)
	{
		return true;
	}
	return false;
}
bool output::ispath(IplImage *img, int x, int y) // 사다리
{
	COLOR ref_color = getcolor(img, x, y);
	unsigned char R, G, B;	R = ref_color.R;	G = ref_color.G;	B = ref_color.B;
	if (R == C_PATH.R && G == C_PATH.G && B == C_PATH.B)
	{
		return true;
	}
	return false;
}

/// 특정 타입의 hydrant가 몇번째인지 index를 얻는 함수
int output::Get_Hyd_index( int hydnum)
{
	int type = 0;
	int count = 0;
	for (int i = 1; i <= hydnum; i++)
	{
		if (type == m_hydpos[i].type)
			count++;
		else
		{
			type = m_hydpos[i].type;
			count = 0;
		}
	}
	return count;
}


void output::Set_Hyd_Parameters(int hydnum, int xpos, int ypos, double length, double streamlength, int sourcenum, int colorindex, int type, int activation, int highlighted, bool calculation)
{
	HYDRANT &hyd = m_hydpos[hydnum];
	HYDRANT prehyd = m_hydpos[hydnum];
	if (xpos != -1)
	{
		int xgap = xpos - hyd.x;
		hyd.x = xpos;
		hyd.needtoupdate = 1;
		for (int i = 0; i < 3; i++)
			hyd.xx[i] += xgap;
	}
	if (ypos != -1)
	{
		int ygap = ypos - hyd.y;
		hyd.y = ypos;
		hyd.needtoupdate = 1;
		for (int i = 0; i < 3; i++)
			hyd.yy[i] += ygap;
	}
	if (type != -1)
	{
		hyd.type = type;
	}

	if (length != -1)
	{
		hyd.length = length;
		hyd.needtoupdate = 1;
	}
	if (streamlength != -1)
	{
		hyd.streamlength = streamlength;
		hyd.needtoupdate = 2;
	}
	if (colorindex != -1)
	{
		hyd.colorindex = colorindex % NUM_OF_COLOR;
	}
	if (activation != -1)
	{
		hyd.activate = activation;
	}
	if (sourcenum != -1)
		hyd.sourcenum = sourcenum % NUM_OF_SOURCE;
	if (highlighted != -1)
	{
		hyd.highlighted = highlighted;  // 하이라이트는 바로 리턴만
		return;
	}

	if (calculation && hyd.needtoupdate)
		calculate_hydrant_area(hydnum, true);
	sorthyds();
	m_ActStack.push_back(ActionStack(MODIFY_HYDRANT, prehyd));
}

void output::Set_ScaleLine(int x1, int x2, int y, double length) 
{
	if (x1 != x2)
	{
		m_scaleline = ScaleLine(x1, x2, y, length);
		DISTANCE_PER_PIXEL = length / abs(x2 - x1);
		DISTANCE_PER_GRID = DISTANCE_PER_PIXEL * GRIDSIZE;
		SCALELENGTH = length;
	}
	else
	{
		printf("error");
	}
}
