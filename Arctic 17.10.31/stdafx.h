// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.



#define TEST
//#define PENALTY_TEST

#ifdef TEST
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <opencv2\core.hpp>

#pragma comment(lib, "libxl.lib")
#pragma comment(lib, "opencv_world300.lib")
//#pragma comment(lib, "opencv_world300d.lib")
#pragma comment(lib, "opencv_ts300.lib")
//#pragma comment(lib, "opencv_ts300d.lib")

#endif
// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
