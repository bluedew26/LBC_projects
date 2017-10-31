#pragma once
#include "stdafx.h"
#include "config.h"
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <opencv2\core.hpp>
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
#include <set>
#include <time.h>
using namespace std;

COLOR getcolor(IplImage *img, int x, int y);

void setcolor(IplImage *img, int x, int y, unsigned char R, unsigned char G, unsigned char B);