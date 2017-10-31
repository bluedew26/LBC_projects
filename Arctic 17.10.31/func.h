#pragma once
#include "config.h"
#include "structs.h"
#include "AADS_Client.h"
#include <vector>
#include <map>
void initmap(vector<vector<node>> &_map, int minx, int miny, int maxx, int maxy);
int cal_steped_time(double time); // s������ �ð��� 6�ð� ������ hour�� �ٲ�
float GetSpeed(int pre_x, int pre_y, int cur_x, int cur_y, float v, double time, vector<vector<node>> &map, AADS_Client *AADS);
float KaistRequestForGRID(int Datas, int x, int y, double time, vector<vector<node>> &map, AADS_Client *AADS);

void flood(vector<ptr> &endpoints, vector<ptr> &finalpoints, vector<vector<node>> &map, double &ETA, double &capacity, AADS_Client *AADS, ptr dst, bool &nopath);
double ETA_fitting(vector<path_data> &path_info, double ETA, double pre_ETA); // ����ȭ ���� ����

#ifdef TEST

void setcolor(IplImage *img, int x, int y, unsigned char R, unsigned char G, unsigned char B);
{
	int index = y * img->widthStep + (x*img->nChannels);
	img->imageData[index + 2] = R;	// R����
	img->imageData[index + 1] = G;	// G����
	img->imageData[index + 0] = B;	// B����
}
#endif

void Init(gamemap_point *init_info)
{
	/// �α� ���� ///
	FILE *fp = fopen("kaist_log.txt", "w");
	fclose(fp);
	/// �α� ���� ///

	/// input�� ������ ���ġ�ϴ� �κ�
	int n = init_info->num_of_node;
	int temp_x = init_info->point[1][0];
	int temp_y = init_info->point[1][1]; // �������� x,y��ǥ�� �ӽ÷� �޾Ƶ�
	int temp_ftime = init_info->final_time[1];

	for (int i = 1; i < n; i++)  // �������� �������� ������ ����� ������ŭ �ݺ�.
	{
		init_info->point[i][0] = init_info->point[i + 1][0];
		init_info->point[i][1] = init_info->point[i + 1][1]; // ��ĭ�� ������ �о
		init_info->final_time[i] = init_info->final_time[i + 1];
		if (i == n - 1)
		{
			init_info->point[n - 1][0] = temp_x;
			init_info->point[n - 1][1] = temp_y;
			init_info->final_time[n - 1] = temp_ftime;
		}
	}
}

void readdata(int minx, int miny, int maxx, int maxy, vector<vector<node>> &map, double current_t, AADS_Client *AADS)
{

	
	if (pre_load) // �׽�Ʈ ����� ��� ���� �б�
	{
		FILE *dtfile[8];
		dtfile[0] = fopen("TPZ-ARC-ICH-2015-09-20-12-P000.txt", "rt");
		dtfile[1] = fopen("TPZ-ARC-ICF-2015-09-20-12-P000.txt", "rt");
		dtfile[2] = fopen("TPZ-ARC-SCU-2015-09-20-12-P000.txt", "rt");
		dtfile[3] = fopen("TPZ-ARC-SCV-2015-09-20-12-P000.txt", "rt");
		dtfile[4] = fopen("GFS-ARC-AWU-2015-09-20-12-P000.txt", "rt");
		dtfile[5] = fopen("GFS-ARC-AWV-2015-09-20-12-P000.txt", "rt");
		dtfile[6] = fopen("WW3-ARC-TWH-2015-09-20-12-P000.txt", "rt");
		dtfile[7] = fopen("WW3-ARC-SWH-2015-09-20-12-P000.txt", "rt");
		char value[10];
		int row = 0;
		while (1)
		{
			float dta[8][2047] = { { 0 } };
			bool endpoint = false;
			if (row > maxy) /// ���� Ż�� ����
				break;

			if (row < miny)
			{
				for (int k = 0; k < GRIDSIZE; k++)
				{
					for (int i = 0; i < 8; i++)
					{
						char buffer[20000];
						fgets(buffer, sizeof(buffer), dtfile[i]);
					}
				}
				row++;
				continue;
			}
			for (int k = 0; k < GRIDSIZE / 2; k++)  // ��ó��
			{
				for (int i = 0; i < 8; i++)
				{
					char buffer[20000];
					fgets(buffer, sizeof(buffer), dtfile[i]);
				}
			}

			/// 8���� ������ ���������� �б� ///
			for (int i = 0; i < 8; i++)
			{
				int col = 0;
				int index = 1;
				char buffer[20000];

				if (fgets(buffer, sizeof(buffer), dtfile[i]) == NULL) // ���� ���̸� NULL������ ����
				{
					endpoint = true;
					break;
				}

				/// ���� ���������� �̵�
				while (col < minx)
				{
					for (int k = 0; k < GRIDSIZE; k++)
					{
						if (buffer[index] == 'N') // NaN�̸�
						{
							index += 8;
						}
						else
						{
							while (buffer[index] != '\t')
							{
								index += 2;
							}
							index += 2;
						}
					}
					col++;
				}



				while (1)
				{
					for (int k = 0; k < GRIDSIZE / 2; k++) // ��ó��
					{
						if (buffer[index] == 'N')
						{
							index += 8;
						}
						else
						{
							while (buffer[index] != '\t')
							{
								index += 2;
							}
							index += 2;
						}
					}
					if (col > maxx || buffer[index - 1] == '\r')
						break;

					else if (buffer[index] == 'N') // NaN�̸� ���� �ε�����
					{
						index += 8;
						int tempcol = col - 1;
						float tempval = 0;
						while (1)
						{
							if (tempcol <= minx)
							{
								dta[i][col] = INVALID_VALUE;
								break;
							}

							if (dta[i][tempcol] == INVALID_VALUE)
							{
								tempcol--;
								continue;
							}
							else
							{
								dta[i][col] = dta[i][tempcol];
								break;
							}
						}
						col++;
					}
					else
					{
						string str;
						while (buffer[index] != '\t')
						{
							str += buffer[index];
							index += 2;
						}
						index += 2;

						float val = ::atof(str.c_str());
						dta[i][col] = val;

						col++;
					}
					for (int k = 0; k < GRIDSIZE / 2; k++) // ��ó��
					{
						if (buffer[index] == 'N')
						{
							index += 8;
						}
						else
						{
							while (buffer[index] != '\t')
							{
								index += 2;
							}
							index += 2;
						}
					}

				}
			}

			for (int k = 0; k < GRIDSIZE / 2; k++)  // ��ó��
			{
				for (int i = 0; i < 8; i++)
				{
					char buffer[20000];
					fgets(buffer, sizeof(buffer), dtfile[i]);
				}
			}

			if (endpoint)
				break;


			/// �̰��� ������ ��� �߰� ///
			for (int k = minx; k <= maxx; k++)
			{
				map[k][row].add_data(Datas(0, dta[0][k], dta[1][k], dta[2][k], dta[3][k], dta[4][k], dta[5][k], dta[6][k], dta[7][k]));

				/// ���� �׸��� �κ� ///
				unsigned char colorval;
				if (dta[0][k] == INVALID_VALUE)
					colorval = 255;
				else
				{
					if (dta[0][k] > MAX_ICE_T)
					{
						colorval = 0;
#ifdef TEST
						setcolor(sample, k, row, 0, 255, 0);
#endif
					}
					else
					{
						colorval = 255 - (255 * dta[0][k] / MAX_ICE_T);
#ifdef TEST
						setcolor(sample, k, row, colorval, colorval, colorval);
#endif
					}
				}
			}
			row++;
		}
		for (int i = 0; i < 8; i++)
			fclose(dtfile[i]);
	}

	if (!pre_load)
	{
		printf("������ ����\n");
		bool depthdataconfirmed = false;
		for (int k = 0; k < MAX_DAYS_STORED; k++)
		{
			
			printf("%d���� ������ ����\n", k);
			int steped_time = cal_steped_time(current_t);
			int w = (maxx - minx + 1);
			int h = (maxy - miny + 1);
			float *result[8];
			double time = k * 3600 * 24; // 1��
			result[0] = AADS->KaistRequest_widely(ICH, minx * GRIDSIZE, miny * GRIDSIZE, w * GRIDSIZE, h * GRIDSIZE, k * 24);
			result[1] = AADS->KaistRequest_widely(ICF, minx * GRIDSIZE, miny * GRIDSIZE, w * GRIDSIZE, h * GRIDSIZE, k * 24);
			result[2] = AADS->KaistRequest_widely(SCU, minx * GRIDSIZE, miny * GRIDSIZE, w * GRIDSIZE, h * GRIDSIZE, k * 24);
			result[3] = AADS->KaistRequest_widely(SCV, minx * GRIDSIZE, miny * GRIDSIZE, w * GRIDSIZE, h * GRIDSIZE, k * 24);
			result[4] = AADS->KaistRequest_widely(AWU, minx * GRIDSIZE, miny * GRIDSIZE, w * GRIDSIZE, h * GRIDSIZE, k * 24);
			result[5] = AADS->KaistRequest_widely(AWV, minx * GRIDSIZE, miny * GRIDSIZE, w * GRIDSIZE, h * GRIDSIZE, k * 24);
			result[6] = AADS->KaistRequest_widely(TWH, minx * GRIDSIZE, miny * GRIDSIZE, w * GRIDSIZE, h * GRIDSIZE, k * 24);
			result[7] = AADS->KaistRequest_widely(SWH, minx * GRIDSIZE, miny * GRIDSIZE, w * GRIDSIZE, h * GRIDSIZE, k * 24);



			float ice_t = 0, ice_c = 0, cur_u = 0, cur_v = 0, wnd_u = 0, wnd_v = 0, WH = 0, SH = 0;
			for (int y = miny; y <= maxy; y++)
			{
				for (int x = minx; x <= maxx; x++)
				{
					int windex = x - minx;
					int hindex = y - miny;

					for (int i = 0; i < 8; i++)
					{
						float value = result[i][(int)((windex*GRIDSIZE + GRIDSIZE / 2) + (w*GRIDSIZE + GRIDSIZE / 2)*(hindex*GRIDSIZE + GRIDSIZE / 2))];
						if (value < -1500)
							value = -1500;
						switch (i)
						{
						case 0: ice_t = value; break; //fprintf(dump, "%f%\t", value);  break;
						case 1: ice_c = value; break;
						case 2: cur_u = value; break;
						case 3: cur_v = value; break;
						case 4: wnd_u = value; break;
						case 5: wnd_v = value; break;
						case 6: WH = value; break;
						case 7: SH = value; break;
						default: break;
						}
					}
					int stt = k * TIME_GAP; // 7��ġ ����, steped time
					map[x][y].add_data(Datas(stt, ice_t, ice_c, cur_u, cur_v, wnd_u, wnd_v, WH, SH, false));

				}
				//	fprintf(dump, "\n");
			}
			//	fclose(dump);
			for (int i = 0; i < 8; i++)
			{
				delete result[i];
			}

		}
	}

	FILE *depthdata;
	depthdata = fopen("depthdata.txt", "rt");
	printf("���ɵ����� ����\n");

	char buffer[120000];
	for (int y = 0; y < 2047; y++)
	{
		char dummystr[30];
		double dumf = 0;
		fscanf(depthdata, "%s\t", dummystr);
		for (int x = 1; x < 2047; x++)
		{
			char str2[30];

			double value = 0;
			fscanf(depthdata, "%s\t", str2);
			value = atof(str2);

			/// rotated (������ 45��)
			int Halfmap = ((ENTIRE_MAP_WIDTH / 2 + 1));
			float x2 = x - Halfmap;
			float y2 = y - Halfmap;

			float sqt = sqrt(2);
			//	x2 = x2 * sqt;
			//	y2 = y2 * sqt;


			float xr = (1 / sqt) * (x2 - y2);
			float yr = (1 / sqt) * (x2 + y2);

			int xr2 = xr + (Halfmap);
			int yr2 = yr + (Halfmap);

			/// ���� x,y (����)
			int actx = (ENTIRE_MAP_WIDTH - 1) - xr2;
			int acty = yr2;

			if (actx < 0 || actx >= ENTIRE_MAP_WIDTH || acty < 0 || acty >= ENTIRE_MAP_HEIGHT)
				continue;

			//map[actx][acty].depth = value;
			
			int xx[4] = { actx + 1, actx - 1, actx, actx };
			int yy[4] = { acty, acty, acty + 1, acty - 1 };
			map[actx / GRIDSIZE][acty / GRIDSIZE].depth = value; 
			for (int i = 0; i < 4; i++)
			{
				if (xx[i] < 0) xx[i] = 0;
				if (yy[i] < 0) yy[i] = 0;
				if (xx[i] >= ENTIRE_MAP_WIDTH) xx[i] = ENTIRE_MAP_WIDTH - 1;
				if (yy[i] >= ENTIRE_MAP_HEIGHT) yy[i] = ENTIRE_MAP_HEIGHT - 1;
				map[xx[i] / GRIDSIZE][yy[i] / GRIDSIZE].depth = value;
			}

			



		}
		fgets(buffer, sizeof(buffer), depthdata);
	}
	fclose(depthdata);


	/// �����κ� OFFSET�ִ� ��ó��
	for (int i = 0; i < map.size() - 1; i++)
	{
		for (int j = 0; j < map[i].size() - 1; j++)
		{
			if (map[i][j].depth > 0) // >= 0���� �ϸ� ��� �κп� Ȯ���.
			{
				for (int x = -LANDOFFSET; x <= LANDOFFSET; x++)
				{
					for (int y = -LANDOFFSET; y <= LANDOFFSET; y++)
					{
						if (i+x < 0 || i+x >= ENTIRE_MAP_WIDTH || j+y < 0 || j+y >= ENTIRE_MAP_HEIGHT)
							continue;
						if (abs(x) + abs(y) <= LANDOFFSET && map[i + x][j + y].depth < 0) // ���������� �ƴϾ���������
						{
							map[i + x][j + y].depth = 0; // ���� 0(����)���� ���
						}
					}
				}
			}
		}
	}

	/// �׸��� �κ�
#ifdef TEST
	for (int i = 0; i < map.size(); i++)
	{
		for (int j = 0; j < map[i].size(); j++)
		{
			if (map[i][j].depth >= 0)
			{
				setcolor(sample, i, j, 255, 0, 0);
			}
		}
	}
#endif
	AADS->KaistRequest_widely(ICH, 1, 1, 1, 1, 0);
	/// ���ɵ����� ���� �Ϸ�

}



void initmap(vector<vector<node>> &_map, int minx, int miny, int maxx, int maxy)
{
	for (int i = minx; i <= maxx; i++)
	{
		for (int j = miny; j <= maxy; j++)
		{
			if (i == minx  || j == miny || i == maxx || j == maxy)
			{
				_map[i][j].isblocked = true;
			}
		}
	}
}


int cal_steped_time(double time); // s������ �ð��� 24�ð� ������ hour�� �ٲ�

float GetSpeed(int pre_x, int pre_y, int cur_x, int cur_y, float v, double time, vector<vector<node>> &map, AADS_Client *AADS);


float KaistRequestForGRID(int Datas, int x, int y, double time, vector<vector<node>> &map, AADS_Client *AADS);

/// �� ����ġ ���
float calculate_weight(float ice_t);
void flood(vector<ptr> &endpoints, vector<ptr> &finalpoints, vector<vector<node>> &map, double &ETA, double &capacity, AADS_Client *AADS, ptr dst, bool &nopath);




double ETA_fitting(vector<path_data> &path_info, double ETA, double pre_ETA) // ����ȭ ���� ����;