#include "stdafx.h"
#include <iostream>
#include <set>
#include <algorithm>
#include <cmath>
#include <map>


#include "config.h"
#include "outputconfig.h"
#include "pathfinder.h"
#include "structs.h"
#include "func.h"


#ifdef TEST
using namespace cv;
#endif
using namespace std;

vector<path_data> three_ptr_smoothing(vector<path_data> &vec, AADS_Client *AADS, vector<vector<node>> &map);


int _minx, _miny, _maxx, _maxy;
double min_cost = pow(2, 32);

//Book *book60, *book70, *book80, *book90, *book100;
//Sheet *sheet60, *sheet70, *sheet80, *sheet90, *sheet100;
#ifdef TEST

IplImage *sample = NULL;

#endif

bool issurrounded(int x, int y, double time, AADS_Client *AADS, vector<vector<node>> &map)
{
	float ice_t[5];
	double depth[5];
	
	ice_t[0] = KaistRequestForGRID(ICH, x, y, time, map, AADS);
	ice_t[1] = KaistRequestForGRID(ICH, x + 1, y, time, map, AADS);
	ice_t[2] = KaistRequestForGRID(ICH, x - 1, y, time, map, AADS);
	ice_t[3] = KaistRequestForGRID(ICH, x, y + 1, time, map, AADS);
	ice_t[4] = KaistRequestForGRID(ICH, x, y - 1, time, map, AADS);
	depth[0] = map[x][y].depth;
	depth[1] = map[x+1][y].depth;
	depth[2] = map[x-1][y].depth;
	depth[3] = map[x][y+1].depth;
	depth[4] = map[x][y-1].depth;

	for (int i = 0; i < 5; i++)
	{
		printf("ice_t : %f\n", ice_t[i]);
		printf("depth : %lf\n", depth[i]);
		if (ice_t[i] < ICE_BREAKING_CAPA && depth[i] + DEPTH_THRESHOLD <= -HULLDEPTH)
		{


			printf("not surrounded #%d\n", i);
			return false;
		}
		else
		{
			printf("surrounded\n");
			return true;
		}
		/*
		else if (i == 0)
		{
			printf("surrounded\n");
			return true;
		}
		*/
	}
	printf("surrounded\n");
	return true;
}


output* KAIST_pathfinder(gamemap_point *init_info, AADS_Client *AADS)// 리턴타입은 경로를 담은 structure, 인자는 출발점, 도착점, 경유지에 대한 sturucture
{
	Init(init_info);
	
	AADS->Init(); /// sdb파일에서 정보를 읽음
	AADS->ExcelInit();
	vector<path_data> total_path; /// 전체 경로
	/// 변수 정의 ///



	vector<path_data> temp_path;

	/// map 정의(그리드만큼) ///
	vector<vector<node>> map;
	map.assign(ENTIRE_MAP_WIDTH / GRIDSIZE + 2, vector<node>());
	for (int i = 0; i < map.size(); i++)	{ map[i].assign(ENTIRE_MAP_HEIGHT / GRIDSIZE + 2, node()); }
	/// map 정의 ///



	/// 맵 사이즈 얻고 gamemap 얻기 
	int maxx, maxy, minx, miny;
	minx = ENTIRE_MAP_HEIGHT;
	miny = ENTIRE_MAP_WIDTH;
	maxx = 0;
	maxy = 0;
	for (int i = 0; i < init_info->num_of_node; i++)
	{
		maxx = max(maxx, init_info->point[i][0] / GRIDSIZE);
		maxy = max(maxy, init_info->point[i][1] / GRIDSIZE);
		minx = min(minx, init_info->point[i][0] / GRIDSIZE);
		miny = min(miny, init_info->point[i][1] / GRIDSIZE);
	}
	
	minx = minx - Offset;
	if (minx < 0)
		minx = 0;
	maxx = maxx + Offset;
	if (maxx >= ENTIRE_MAP_WIDTH / GRIDSIZE)
		maxx = ENTIRE_MAP_WIDTH / GRIDSIZE - 1;
	miny = miny - Offset;
	if (miny < 0)
		miny = 0;
	maxy = maxy + Offset;
	if (maxy >= ENTIRE_MAP_HEIGHT / GRIDSIZE)
		maxy = ENTIRE_MAP_HEIGHT / GRIDSIZE - 1;
	/// 이 시점에서 테두리 minx, miny, maxx, maxy 정의됨

	printf(" minx : %d, maxx : %d, miny : %d, maxy : %d\n", maxx, maxy, minx, miny);


	initmap(map, minx, miny, maxx, maxy);  /// 테두리 부분 블록시킴


	double current_t = 0;
	
	FILE *dump = fopen("datadump.txt", "w");

	

	/// 수심데이터 로드 ///

	/// 수심데이터 로드 ///


#ifdef TEST
	
	sample = cvCreateImage(CvSize(2048 / GRIDSIZE, 2048 / GRIDSIZE), IPL_DEPTH_8U, 3);
	cvSet(sample, cvScalarAll(0)); // 전체가 검은색. 육지로 취급

#endif
	readdata(minx, miny, maxx, maxy, map, current_t, AADS);

#ifdef TEST
	cvShowImage("test", sample);
	cvSaveImage("result.png", sample);
	cvWaitKey(20);
#endif

	printf("데이터 획득 완료\n");

	/// 유효성 체크
	output *out = NULL;
	for (int i = 0; i < init_info->num_of_node; i++)
	{
		if (issurrounded(init_info->point[i][0] / GRIDSIZE, init_info->point[i][1] / GRIDSIZE, current_t, AADS, map)) /// 유효성 검사에 실패하면
		{
			out = new output(total_path.size()); /// n에 -1을 달아 리턴.
			printf("유효성 검사 실패 \n");
			goto result;
		}
	}
	/// 유효성 체크

	AADS->KaistRequest_widely(ICH, 1, 1, 1, 1, 0);
	double current_speed = SPEED_FOR_OPEN_WATER;   // waypoint지날 때 마다 그때 속도로 덮어씌움


	double tweaked = 1;
	for (int i = 0; i < init_info->num_of_node - 1; i++)
	{
		int xpos_init, ypos_init, xpos_final, ypos_final;
		// 필요한 인자, initial cost(현재까지 누적된 current_t), 
		xpos_init = init_info->point[i][0] / GRIDSIZE;
		ypos_init = init_info->point[i][1] / GRIDSIZE;
		xpos_final = init_info->point[i + 1][0] / GRIDSIZE;
		ypos_final = init_info->point[i + 1][1] / GRIDSIZE;
		
		printf("startx : %d, starty : %d\n", xpos_init, ypos_init);
		printf("destx : %d, desty : %d\n", xpos_final, ypos_final);

		path_data initial_path_info = path_data(xpos_init, ypos_init, 0, current_t, current_speed);



		map[xpos_final][ypos_final].isflooded == false;

		/// ETA 관련 설정 ///
		double pre_ETA = init_info->final_time[i];
		double ETA = init_info->final_time[i + 1];
		if (ETA == 0)
			ETA = current_t + sqrt(pow(xpos_init - xpos_final, 2) + pow(ypos_init - ypos_final, 2)) * NODE_DISTANCE / SPEED_FOR_OPEN_WATER* 15;
			//ETA = pow(2, 32);
		/// ETA 관련 설정 ///


		int cur_x = xpos_init;
		int cur_y = ypos_init;

		/// H값 설정을 위한 밑작업 - Gamemap에 대한 초기 정보들 모두 얻기


		for (int i = minx; i <= maxx; i++)
		{
			for (int j = miny; j <= maxy; j++)
			{
				map[i][j].cost = 0;
				map[i][j].isflooded = false;
				map[i][j].time = 0;
				map[i][j].speed = 0;
				map[i][j].dir = NONE;
			}
		}

		vector<path_data> path_info; // path를 저장할 list를 선언

		vector<ptr> endpoints, finalpoints;
//		flooded.insert(pair<ptr, ptr_data>(ptr(xpos_init, ypos_init), ptr_data(NONE, current_t, current_speed)));
		endpoints.push_back(ptr(xpos_init, ypos_init, NONE, current_speed, 0, current_t));
		
		finalpoints.push_back(ptr(xpos_init, ypos_init, NONE, current_speed, 0, current_t));
		printf("costused : %lf, time : %lf\n", finalpoints.begin()->costused, finalpoints.begin()->time);
		map[xpos_init][ypos_init].isflooded = true;
		map[xpos_init][ypos_init].time = current_t;
		ptr dst = ptr(xpos_final, ypos_final, NONE, 0, 0, 0);

		double capacity = 10000;// ETA의 2%만큼 우선 플러딩 진행
		bool nopath = false;
		/// main loop

		

		while (1)
		{
		
			static int count = 0;
			count++;
			printf("iteration : %d\n", count);
			printf("ETA : %lf\n", ETA);
			/// 여기서부터 flooding 수행
			flood(endpoints, finalpoints, map, ETA, capacity, AADS, dst, nopath);
			printf("capacity : %lf, flooded points: %d\n", capacity, 0);
			/// flood 완료
			
			
			
			if (nopath == true)
				break;

			/// 도착점이 있는지 검사
			if (map[xpos_final][ypos_final].isflooded == true) // 도착점이 flood 되어 있으면
			{
				printf("도착점 도달 \n");
				int cur_x = xpos_final;
				int cur_y = ypos_final;
				current_speed = map[cur_x][cur_y].speed;
				current_t = map[cur_x][cur_y].time;
				double current_cost = map[cur_x][cur_y].cost;
				bool escape = false;
				while (1)
				{
					
					
					double cost = map[cur_x][cur_y].cost;
					float spd = map[cur_x][cur_y].speed;
					double time = map[cur_x][cur_y].time;
					
					path_info.push_back(path_data(cur_x, cur_y, cost, time, spd));
					int DIR = map[cur_x][cur_y].dir;
					if (escape)
					{
						break;
					}
					switch (DIR)
					{
					case LEFT: cur_x--; break;
					case RIGHT: cur_x++; break;
					case UP: cur_y++; break;
					case DOWN: cur_y--; break;
					case LEFT_UP: cur_x--; cur_y++; break;
					case LEFT_DOWN: cur_x--; cur_y--; break;
					case RIGHT_UP: cur_x++; cur_y++; break;
					case RIGHT_DOWN: cur_x++; cur_y--; break;
					default: escape = true;  break;
					}
					
					if (cur_x == xpos_init && cur_y == ypos_init)
						break;
				}
				path_info.push_back(initial_path_info);
				reverse(path_info.begin(), path_info.end());
				
				break;
			}
		}


		for (int i = 0; i < path_info.size(); i++)
		{
			printf(" i : %d, time : %.2lf, cost : %.2lf\n", i, path_info[i].time, path_info[i].cost);
		}

		temp_path = path_info;


#ifdef TEST

		/// minx, miny, maxx, maxy 기준으로 생성

		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 0.6, 0.6, 0, 1); // 폰트 정의
		///
		IplImage *res;
		int diffx = (maxx - minx);
		int diffy = (maxy - miny);
		res = cvCreateImage(CvSize(diffx * MAP_MAGITUDE, diffy * MAP_MAGITUDE), IPL_DEPTH_8U, 3);
		cvSet(res, cvScalarAll(255));
		for (int x = minx + 1; x <= maxx - 1; x++)
		{
			for (int y = miny + 1; y <= maxy - 1; y++)
			{
				int ax = x - minx;
				int ay = y - miny;
				char s[100];

				if (map[x][y].depth >= 0)
				{
					cvRectangle(res, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE), CvPoint((ax + 1)*MAP_MAGITUDE, (ay + 1)*MAP_MAGITUDE), CvScalar(0, 0, 255), CV_FILLED);
				}
				else
				{
					unsigned char colorval = 0;
					float ice_t = map[x][y].datas[0].ice_t;
					if (ice_t > MAX_ICE_T)
					{
						colorval = 0;
						colorval = 255 - colorval;
#ifdef TEST
						cvRectangle(res, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE), CvPoint((ax + 1)*MAP_MAGITUDE, (ay + 1)*MAP_MAGITUDE), CvScalar(255, 255 - colorval, 255 - colorval), CV_FILLED);
#endif
					}
					else
					{
						colorval = 255 - (255 * ice_t / MAX_ICE_T);
						colorval = 255 - colorval;
#ifdef TEST
						cvRectangle(res, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE), CvPoint((ax + 1)*MAP_MAGITUDE, (ay + 1)*MAP_MAGITUDE), CvScalar(255, 255 - colorval, 255 - colorval), CV_FILLED);
#endif
					}
				}
			}
		}
		
		
		for (int i = 0; i < temp_path.size(); i++)
		{
			int x = (temp_path[i].x - minx);// / GRIDSIZE;
			int y = (temp_path[i].y - miny);// / GRIDSIZE;
			cvRectangle(res, CvPoint(x*MAP_MAGITUDE, y*MAP_MAGITUDE), CvPoint((x + 1)*MAP_MAGITUDE, (y + 1)*MAP_MAGITUDE), CvScalar(0, 255, 0), CV_FILLED);

		}
		for (int x = minx + 1; x <= maxx - 1; x++)
		{
			for (int y = miny + 1; y <= maxy - 1; y++)
			{
				if (map[x][y].time > 0)
				{
					int ax = x - minx;
					int ay = y - miny;
					float v = map[x][y].speed;
					float t = map[x][y].time;
					float cost = map[x][y].cost;
					
					
				
					float wind_u = map[x][y].datas[0].wind_U;
					float wind_v = map[x][y].datas[0].wind_V;
					float wave_u = map[x][y].datas[0].cur_u;
					float wave_v = map[x][y].datas[0].cur_v;
					float ice_c = map[x][y].datas[0].ice_c;
					float waveheight = map[x][y].datas[0].waveHeight;
					float swellheight = map[x][y].datas[0].swellHeight;
					float ice_t = map[x][y].datas[0].ice_t;
					float wind_angle = atan2(wind_v, wind_u);
					float current_angle = atan2(wave_v, wave_u);
					
					if (wind_u == 0)
					{
						if (wind_v >= 0)
							wind_angle = 3.1415 / 2;
						else
							wind_angle = -3.1415 / 2;
					}
					if (wave_u == 0)
					{
						if (wave_v >= 0)
							current_angle = 3.1415 / 2;
						else
							current_angle = -3.1415 / 2;
					}

					/*
					if (wind_u*wind_v < 0)
						wind_angle = wind_angle + 3.1415;
					if (wave_u*wave_v < 0)
						current_angle = current_angle + 3.1415;
						*/
					CvPoint start = CvPoint(ax*MAP_MAGITUDE + MAP_MAGITUDE / 2, ay*MAP_MAGITUDE + MAP_MAGITUDE / 2);
					CvPoint dst = CvPoint(start.x + MAP_MAGITUDE * cos(wind_angle) / 2, start.y + MAP_MAGITUDE * sin(wind_angle) / 2);

					if (wind_u == 0 && wind_v == 0)
					{
					}
					else
						cvLine(res, start, dst, CvScalar(255, 0, 255), 2); /// 바람선 그리기

					start = CvPoint(ax*MAP_MAGITUDE + MAP_MAGITUDE / 2, ay*MAP_MAGITUDE + MAP_MAGITUDE / 2);
					dst = CvPoint(start.x + MAP_MAGITUDE * cos(current_angle) / 2, start.y + MAP_MAGITUDE * sin(current_angle) / 2);
					
					if (wave_u == 0 && wave_v == 0)
					{
					
					}
					else
						cvLine(res, start, dst, CvScalar(0, 0, 255), 2); /// 파도선 그리기


					/// 속도값 표시
					char s[100];
					sprintf(s, "%.3f", v); // m/s 속도
					cvPutText(res, s, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE + MAP_MAGITUDE / 3), &font, CvScalar(0, 0, 0));
					///

					/// 시간 및 코스트 표시
					/*
					sprintf(s, "%.1f", t / 60); // 시간 (분)
					cvPutText(res, s, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE + MAP_MAGITUDE * 2 / 3), &font, CvScalar(0, 0, 0));
					sprintf(s, "%.1f", cost / 60); // 코스트 (분)
					cvPutText(res, s, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE + MAP_MAGITUDE), &font, CvScalar(0, 0, 0));
					*/
					///

					/// 파도값 표시
					
					sprintf(s, "%.2f", wave_u); // 파도 x축
					cvPutText(res, s, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE + MAP_MAGITUDE * 2 / 3), &font, CvScalar(0, 0, 0));
					sprintf(s, "%.2f", wave_v); // 파도 y축
					cvPutText(res, s, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE + MAP_MAGITUDE), &font, CvScalar(0, 0, 0));
					
					///

					/// 바람값 표시
					/*
					sprintf(s, "%.2f", wind_u); // 바람 x축
					cvPutText(res, s, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE + MAP_MAGITUDE * 2 / 3), &font, CvScalar(0, 0, 0));
					sprintf(s, "%.2f", wind_v); // 바람 y축
					cvPutText(res, s, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE + MAP_MAGITUDE), &font, CvScalar(0, 0, 0));
					*/
					///

					/// 빙집적도 표시
					/*
					sprintf(s, "%.2f", ice_t); // 파도 y축
					cvPutText(res, s, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE + MAP_MAGITUDE * 2 / 3), &font, CvScalar(0, 0, 0));

					sprintf(s, "%.2f", ice_c); // 파도 y축
					cvPutText(res, s, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE + MAP_MAGITUDE), &font, CvScalar(0, 0, 0));
					*/
					///
					/*
					/// wave 높이와 swellheight표시
					sprintf(s, "%.2f", waveheight); // 시간 (분)
					cvPutText(res, s, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE + MAP_MAGITUDE * 2 / 3), &font, CvScalar(0, 0, 0));
					sprintf(s, "%.2f", swellheight); // 코스트 (분)
					cvPutText(res, s, CvPoint(ax*MAP_MAGITUDE, ay*MAP_MAGITUDE + MAP_MAGITUDE), &font, CvScalar(0, 0, 0));
					///
					*/
				}
			}
		}
		for (int i = 0; i < res->width; i += MAP_MAGITUDE)
		{
			for (int j = 0; j < res->height; j += MAP_MAGITUDE)
			{
				cvLine(res, CvPoint(i, 0), CvPoint(i, res->height - 1), CvScalar(0, 0, 0), 1); // 격자선긋기
				cvLine(res, CvPoint(0, j), CvPoint(res->width - 1, j), CvScalar(0, 0, 0), 1); // 격자선긋기
			}
		}
		cvSaveImage("result3.png", res);
#endif


		/// 스무딩
		if (USE_SMOOTHING)
		{
			three_ptr_smoothing(path_info, AADS, map);

			printf("<스무딩 후>\n");
			for (int i = 0; i < path_info.size(); i++)
			{
				printf(" i : %d, time : %.2lf, cost : %.2lf\n", i, path_info[i].time, path_info[i].cost);
			}
		}

		/// 스무딩
		if (!path_info.empty())
		{
			if (ETA != 0 && ETA != pow(2, 32) && path_info.back().time < ETA)
			{
				/// ETA 계산하는 함수
				tweaked = ETA_fitting(path_info, ETA, pre_ETA);
				printf("%.2lf%%로 에너지 소비 감소 \n", pow(tweaked,2)*100);
				current_t = ETA;
			}

		}
		if (total_path.size() > 0)
		{
			if (path_info.size() > 0)
				total_path.erase(total_path.end() - 1);
		}
		

		total_path.insert(total_path.end(), path_info.begin(), path_info.end());
	
	}
	/// main loop

result:

	//vector<path_data> total_path2 = total_path;

	vector<vector<node>> vclear;
	map.swap(vclear);
	vclear.clear();

	out = new output(total_path.size());
	out->n = total_path.size();
	int curx = 0, cury = 0, prex = 0, prey = 0;
	for (int i = 0; i < out->n; i++)
	{
		vector<path_data>::iterator it;
		it = total_path.begin();
		out->x[i] = it->x*GRIDSIZE;
		out->y[i] = it->y*GRIDSIZE;
		out->v[i] = it->speed / KNOTS_TO_M_PER_SEC * tweaked;   // m/s to knot
		out->t[i] = it->time;	
//		out->v[i] = 1;   // m/s to knot
//		out->t[i] = 1;		// sec
		total_path.erase(it);
		
		curx = out->x[i];
		cury = out->y[i];
#ifdef TEST
		if (i > 0)
		{
			
			cvLine(sample, CvPoint(curx / GRIDSIZE, cury / GRIDSIZE), CvPoint(prex / GRIDSIZE, prey / GRIDSIZE), CvScalar(255, 0, 0), 2);
			cvCircle(sample, CvPoint(curx / GRIDSIZE, cury / GRIDSIZE), 3, CvScalar(0, 100, 255), CV_FILLED);
		}
#endif
		prex = curx;
		prey = cury;
#ifdef TEST
		cvShowImage("test", sample);
#endif
	}
#ifdef TEST
	cvSaveImage("result2.png", sample);
//	cvWaitKey(0);
#endif


	/// 경로상의 상세데이터를 얻는 부분.

	AADS->ExcelRelease();



	return out;
}







pair<double, double> cal_time(AADS_Client *AADS, path_data p1, path_data p2, vector<vector<node>> &map)
{
	if (p2.x == 31 && p2.y == 32)
		int a = 3;

	
	double init_t = p1.cost; // 초기 소요 시간
	
	double distance = sqrt(pow(p2.y - p1.y, 2) + pow(p2.x - p1.x, 2));
	double deltax = (p2.x - p1.x) / distance;
	double deltay = (p2.y - p1.y) / distance;
	double step = 1 / max(abs(deltax), abs(deltay)) + 0.001;
	double tsum = 0, Gsum = 0;    // Gsum은 avoidance반영된 것
	
	/// 한칸씩 조사해야한다는 가정
	int cur_x = p1.x;
	int cur_y = p1.y;
	double cur_speed = p1.speed; // 이 시점에서 스피드는 knots
	int pre_x = p1.x;
	int pre_y = p1.y;
	double i = 0;
	double deltat = 0;
	double deltaG = 0;
	double cur_time;
	for (i = step; i <= distance + 0.1; i += step)
	{
		float ice_t = 0;
		double outputspeed = 0;
		if (i + step > distance + 0.1)
		{
			cur_x = p2.x;
			cur_y = p2.y;
		}
		else
		{
			cur_x = p1.x + (int)(deltax*i);
			cur_y = p1.y + (int)(deltay*i);
		}
		if (pre_x == cur_x && pre_y == cur_y)
			continue;

		if (cur_x == pre_x && cur_y == pre_y)
			continue;
		cur_time = init_t + tsum;

		int steped_time = cal_steped_time(init_t + tsum);
		node &n = map[cur_x][cur_y];
		int index = n.find(steped_time);
		ice_t = n.datas[index].ice_t;
		///


		if (ice_t >= ICE_BREAKING_CAPA || map[cur_x][cur_y].isblocked == true || map[cur_x][cur_y].depth > -HULLDEPTH)
			return pair<double, double>(-1, -1);

		/// 속도 계산 및 데이터 갱신
		outputspeed = GetSpeed(pre_x, pre_y, cur_x, cur_y, cur_speed, cur_time, map, AADS);
		cur_speed = outputspeed;

		int manhattan_dis = abs(pre_x - cur_x) + abs(pre_y - cur_y);
		if (manhattan_dis == 1)
			deltat = NODE_DISTANCE * GRIDSIZE / cur_speed;
		else
			deltat = sqrt(2) * NODE_DISTANCE * GRIDSIZE / cur_speed;

		deltaG = deltat * calculate_weight(ice_t);

		tsum += deltat;
		Gsum += deltaG;


		pre_x = cur_x;
		pre_y = cur_y;
	}
	if (cur_x == p2.x && cur_y == p2.y)
	{
		// 아무것도 안함
	}
	else
	{
		cur_time = init_t + tsum;

		int steped_time = cal_steped_time(init_t + tsum);
		node &n = map[cur_x][cur_y];
		int index = n.find(steped_time);
		float ice_t = n.datas[index].ice_t;
		///

		if (ice_t >= ICE_BREAKING_CAPA || map[cur_x][cur_y].isblocked == true || map[cur_x][cur_y].depth > -HULLDEPTH)
			return pair<double, double>(-1, -1);

		/// 속도 계산 및 데이터 갱신
		double outputspeed = GetSpeed(cur_x, cur_y, p2.x, p2.y, cur_speed, cur_time, map, AADS);
		cur_speed = outputspeed;

		int manhattan_dis = abs(p2.x - cur_x) + abs(p2.y - cur_y);
		if (manhattan_dis == 1)
			deltat = NODE_DISTANCE * GRIDSIZE / cur_speed;
		else
			deltat = sqrt(2) * NODE_DISTANCE * GRIDSIZE / cur_speed;

		deltaG = deltat * calculate_weight(ice_t);

		tsum += deltat;
		Gsum += deltaG;
	}


	return pair<double, double>(tsum, Gsum);
	///
}

vector<path_data> three_ptr_smoothing(vector<path_data> &vec, AADS_Client *AADS, vector<vector<node>> &map)
{
	if (vec.empty() == true)
		return vector<path_data>();
	vector<path_data>::iterator here = vec.begin();
	
	while (1)
	{
		if ((here + 1) == vec.end() || (here + 2) == vec.end())
			break;

		if (here->x == (here + 1)->x && (here + 1)->x == (here + 2)->x) // 일직선이면
		{
			vec.erase(here + 1);
			continue;
		}
		else if (here->y == (here + 1)->y && (here + 1)->y == (here + 2)->y) // 일직선이면
		{
			vec.erase(here + 1);
			continue;
		}
		else if (abs((here + 2)->x - here->x) == abs((here + 2)->y - here->y)) // 완전 대각선 방향인데
		{
			int mag = abs((here + 2)->x - here->x);
			int minx = min((here + 2)->x, here->x);
			int miny = min((here + 2)->y, here->y);
			int maxx = max((here + 2)->x, here->x);
			int maxy = max((here + 2)->y, here->y);
			bool smoothen = false;
			for (int i = 1; i < mag; i++)
			{
				int curx = minx + i;
				int cury = miny + i;
				if ((here + 1)->x == curx && (here + 1)->y == cury)
				{
					smoothen = true;
				}
			}
			if (smoothen)
			{
				vec.erase(here + 1);
				continue;
			}
		}

		/// 최근 3점을 얻음
		path_data p1 = *here;
		path_data p3 = *(here + 2);
		/// p1과 p3을 이어서 장애물이 없으면

		pair<double, double> result = cal_time(AADS, p1, p3, map);
		double direct_time = result.first;
		double direct_cost = result.second;


		if (direct_cost != -1)
		{
			double deltaC = p3.cost - p1.cost - direct_cost;
			double deltat = p3.time - p1.time - direct_time;
			if ((p1.cost + direct_cost <= p3.cost + 0.001))
			{
				vector<path_data>::iterator it2 = here + 2;
				while (it2 != vec.end())   // 스피드를 평균으로 바꿈?
				{
					it2->time -= deltat;			 // 나머지 모든 점들의 time을 차분만큼 감소시킴.
					it2->cost -= deltaC;
					it2++;
				}
				vec.erase(here + 1); // 중간부분을 지운다.
				continue;
			}
			else
				here++;
		}
		else
			here++;
	}
	return vec;
}
