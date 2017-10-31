#include "stdafx.h"
#include "hydrant.h"

void output::flood(HYDRANT &pos, int numofiter) // m_grid는 배열 형태. 첫번째 Index는 도면 번호, 두번째부터 좌표
{
//	static bool first = true;
	//if (first == true)
	//{
	//	numofiter = 1;
	//	first = false;
	//}


	/// 시간 체크
	//	printf("now flooding... ( : %d, hydnum : %d)\n". hydnum);

	clock_t iteration_time = clock();
	///
	if (pos.type == TYPE_MONITOR) // 모니터일경우 다른거 수행
		return;

	vector<vector<LocMap>> &Lmap = pos.LMap;
	vector<vector<Node>> &grid = m_grid;

	// local map 할당 //
	pos.reset();
	int GRIDLENGTH = pos.length / DISTANCE_PER_GRID;
	Lmap.assign(GRIDLENGTH * 2 + 1, vector<LocMap>());
	for (int i = 0; i < Lmap.size(); i++)
	{
		Lmap[i].assign(GRIDLENGTH * 2 + 1, LocMap());
	}
	// local map 할당 //

	// 상대 좌표 획득 //

	int rx = GRIDLENGTH;
	int ry = GRIDLENGTH;
	// 상대 좌표 획득 //

	int ax = pos.x; // 절대좌표
	int ay = pos.y;

	int xgap = ax - rx; // gap을 더하면 상대좌표를 절대좌표로 만들 수 있음
	int ygap = ay - ry; // gap을 더하면 상대좌표를 절대좌표로 만들 수 있음

	pos.rel_x = xgap;
	pos.rel_y = ygap;
	/// 초기화 
	double avail_dis = pos.length;
	double remain_dis = avail_dis;
	Lmap[rx][ry].isflooded = true;
	Lmap[rx][ry].dir = NONE;
	Lmap[rx][ry].rdis = remain_dis;
	vector<ptr> endpoints;
	vector<ptr> &flooded = pos.range;
	vector<ptr> &outlines = pos.outline;
	vector<ptr> finalpoints;
	vector<ptr> nextpoints;
	endpoints.push_back(ptr(rx, ry)); // 상대좌표로 저장
	flooded.push_back(ptr(ax, ay));
	/// 변수 설정
	double cur_rdistance = 0.0; // 현재 남은 길이
	///

	/// 1차 flooding ///


#define NUMOFDIR 8
	while (1)
	{
		bool noescape = false;  // 루프 탈출과 관련된 플래그

		for (int i = 0; i < endpoints.size(); i++)
		{


			double cost = 0;
			rx = endpoints[i].x;
			ry = endpoints[i].y;
			ax = rx + xgap; // endpoint의 현재 인덱스
			ay = ry + ygap;

			cur_rdistance = Lmap[rx][ry].rdis;

			pair<ptr, DIRECTION> target[8]; // 8방향 이동시의 포인트를 정의 (절대좌표로)
			target[0] = pair<ptr, DIRECTION>(ptr(ax + 1, ay), DIRECTION(LEFT));
			target[1] = pair<ptr, DIRECTION>(ptr(ax - 1, ay), DIRECTION(RIGHT));
			target[2] = pair<ptr, DIRECTION>(ptr(ax, ay + 1), DIRECTION(DOWN));
			target[3] = pair<ptr, DIRECTION>(ptr(ax, ay - 1), DIRECTION(UP));
			target[4] = pair<ptr, DIRECTION>(ptr(ax + 1, ay + 1), DIRECTION(LEFT_DOWN));
			target[5] = pair<ptr, DIRECTION>(ptr(ax + 1, ay - 1), DIRECTION(LEFT_UP));
			target[6] = pair<ptr, DIRECTION>(ptr(ax - 1, ay + 1), DIRECTION(RIGHT_DOWN));
			target[7] = pair<ptr, DIRECTION>(ptr(ax - 1, ay - 1), DIRECTION(RIGHT_UP));

			bool blocked[8] = { 0 };


			/// 8방향 검색 ///
			for (int k = 0; k < NUMOFDIR; k++)
			{
				int x = target[k].first.x; // 절대좌표 (상대좌표로 바꾸려면 gap을 뺌)
				int y = target[k].first.y;
				int rxt = x - xgap; // 타겟의 rx, ry
				int ryt = y - ygap;


				if (rxt < 0 || ryt < 0 || rxt > GRIDLENGTH * 2 || ryt > GRIDLENGTH * 2)
				{
					continue;
				}


				/// 계단 사이를 가로질러 가는것 방지 ///
				if (grid[x][y].isstair == true && grid[ax][ay].isstair == true)
					continue;
				/// 계단 사이를 가로질러 가는것 방지 ///

				/// 이동 지점이 장애물이면 안감 ///

				if (k >= 4) // 대각선이동시
				{
					int x1 = ax, x2 = x;
					int y1 = y, y2 = ay;
					if (grid[x1][y1].isobstacle[m_currentlayer] == true || grid[x2][y2].isobstacle[m_currentlayer] == true || grid[x][y].isobstacle[m_currentlayer] ||
						grid[x1][y1].isequipment == true || grid[x2][y2].isequipment == true || grid[x][y].isequipment == true)
					{
						pos.wallblocked.insert(ptr(ax, ay));
					//	Lmap[rxt][ryt].isoutline = true;
					//	outlines.push_back(ptr(ax, ay));
						continue;
					}
				}
				else
				{
					if (grid[x][y].isobstacle[m_currentlayer] == true || grid[x][y].isequipment == true)
					{
						pos.wallblocked.insert(ptr(ax, ay));
					//	Lmap[rxt][ryt].isoutline = true;
					//	outlines.push_back(ptr(ax, ay));
						continue;
					}
				}

				/// 이동 코스트 산정 (계단, 사다리, 길에 따라 다름) ///
				if (k < 4)
				{
					cost = DISTANCE_PER_GRID;
				}
				else
					cost = sqrt(2) * DISTANCE_PER_GRID;
				if (grid[x][y].isstair == true)
				{
					cost += STAIR_PENALTY[grid[x][y].stairindex];
				}
				/// 이동 코스트 산정 (계단, 사다리, 길에 따라 다름) ///

				double rdis = cur_rdistance - cost;

				/// 코스트 부족 판정 ///
				if (rdis < 0) // 코스트 부족하면 이동불가. 다음루프로
				{
					blocked[k] = true;
					continue;
				}
				/// 코스트 부족 판정 ///

				if (Lmap[rxt][ryt].isflooded == false) // 목록에 없으면
				{
					Lmap[rxt][ryt].isflooded = true;
					Lmap[rxt][ryt].rdis = rdis;
					Lmap[rxt][ryt].dir = target[k].second;
					nextpoints.push_back(ptr(rxt, ryt));					  // 추가
					flooded.push_back(ptr(x, y));					      // 추가
				//	Lmap[rx][ry].isoutline = true;
					noescape = true;
				}
				else // 이미 목록에 있으면
				{

					if (Lmap[rxt][ryt].rdis < rdis) // 더 높은 remain_dis를 가질 수 있는 경우
					{
						Lmap[rxt][ryt].rdis = rdis;
						Lmap[rxt][ryt].dir = target[k].second; // 방향 역시 갱신
						noescape = true;
						nextpoints.push_back(ptr(rxt, ryt));
					}


				}
			}
			
			bool block = false;
			for (int p = 0; p < NUMOFDIR; p++)
			{
				block = block | blocked[p];
			}
			if (block == true) // 하나라도 코스트부족으로 못가면
			{
				finalpoints.push_back(ptr(rx, ry)); // finalpoint만 상대좌표로 넣음
				Lmap[rx][ry].isfinalpoint = true;
			//	Lmap[rx][ry].isoutline = true;
			}
			
		}
		if (noescape == false) // 더이상 변화가 없으면
			break;
		endpoints.clear();
		endpoints = nextpoints;
		nextpoints.clear();
	}
	/// 초기 flooding 부분 ///, 이 시점에서 종점들이 finalpoints들에 존재.
	nextpoints.clear();

	int iteration = 1;

	/// 후속 flooding 부분 ///
	while (1) // iteration이 불가능할때까지 반복
	{
		vector<ptr> fpts = finalpoints;
		finalpoints.clear();

		if (iteration >= numofiter && numofiter != 0) // 원하는 iteration 횟수를 초과할 경우 종료;
			break;



		iteration++;


		bool flag = 0;  // 변화가 존재하면 flag on, 변화가 없으면 루프에서 빠져나옴


		/// 길이를 구하는 부분 ///
		for (int t = 0; t < fpts.size(); t++)
		{
			// 4면이 다 flood되어있으면 생략
			rx = fpts[t].x;
			ry = fpts[t].y; // 상대좌표 얻음

			int rx2[4] = { rx + 1, rx - 1, rx, rx };
			int ry2[4] = { ry, ry, ry + 1, ry - 1 };
			bool surrounded = true; // 주위 4칸이 모두 flooding?
			for (int m = 0; m < 4; m++)
			{
				if (rx2[m] < 0 || ry2[m] < 0 || rx2[m] > 2 * GRIDLENGTH || ry2[m] > 2 * GRIDLENGTH)
				{
					surrounded = false;
					break;
				}
				surrounded = surrounded & Lmap[rx2[m]][ry2[m]].isflooded; // 모두 true면 surrounded유지
			}
			if (surrounded == true)
			{
				Lmap[rx][ry].isoutline = false;
				continue;
			}

			ax = rx + xgap;
			ay = ry + ygap;


			vector<vector<ptr>> temppaths;   /// 이중 벡터
			vector<ptr> temppath;



			bool firstpoint = true;


			temppath.push_back(ptr(ax, ay)); /// 현재 층에서 도착점을 넣음.
			double total_stairpenalty = 0;
			/// 끝점으로 가는 임의의 경로를 추출하는 부분. ///
			while (1)
			{
				/// fptr의 rx와 ry는 이미 얻었다.
				if (grid[ax][ay].isstair == true) // 계단만나면
				{
					total_stairpenalty += STAIR_PENALTY[grid[ax][ay].stairindex];
				}
				DIRECTION targetdir = Lmap[rx][ry].dir;
				switch (targetdir)
				{
				case UP:
					ay++; ry++;	break;
				case DOWN:
					ay--; ry--;	break;
				case LEFT:
					ax--; rx--;	break;
				case RIGHT:
					ax++; rx++; break;
				case LEFT_UP:
					ax--; ay++;  rx--; ry++; break;
				case LEFT_DOWN:
					ax--; ay--; rx--; ry--; break;
				case RIGHT_UP:
					ax++; ay++; rx++; ry++; break;
				case RIGHT_DOWN:
					ax++; ay--; rx++; ry--; break;
				default:
					break;
				}
				temppath.push_back(ptr(ax, ay)); // 그곳을 경로에 넣음.
				if (ax == pos.x && ay == pos.y) // 시작점에 돌아오면
					break;
			}
			vector<ptr> smoothen4 = four_ptr_smoothing(temppath, false);
			vector<ptr> smoothen3 = three_ptr_smoothing(smoothen4, false);
			temppath = smoothen3;
			int iterated = 0;
			
			while (1)
			{
				vector<ptr> padded = padding(smoothen3);
				vector<ptr> smoothen_pos, smoothen_neg;
				smoothen_pos = three_ptr_smoothing(padded, false);
				smoothen_neg = three_ptr_smoothing(padded, true);

				if (totaldistance(smoothen_pos, total_stairpenalty) > totaldistance(smoothen_neg, total_stairpenalty))
					padded = smoothen_neg;
				else
					padded = smoothen_pos;

				if (abs(totaldistance(padded, total_stairpenalty) - totaldistance(smoothen3, total_stairpenalty)) / totaldistance(smoothen3, total_stairpenalty) < 0.1)  // 오차 10% 이내
				{
					temppath = padded;
					break;
				}
				else
				{
					smoothen3 = padded;
				}
				iterated++;

				if (iterated >= 3) // 최대 반복수 제한
				{
					temppath = padded;
					break;
				}
			}
			/// 여기까지 남은 길이 갱신
			





			rx = fpts[t].x;
			ry = fpts[t].y;
			ax = rx + xgap;
			ay = rx + ygap;
			remain_dis = avail_dis - totaldistance(temppath, total_stairpenalty);
			endpoints.clear();
			nextpoints.clear();
			Lmap[rx][ry].rdis = remain_dis;
			endpoints.push_back(ptr(rx, ry)); // 상대좌표

			/// 국소적인 flooding 루프시작 (한개의 final point에 대해?)
			while (1)
			{

				// 루프 탈출 방법 생각하기 //
				bool noescape = false;

				for (int i = 0; i < endpoints.size(); i++)
				{
					double cost = 0;
					rx = endpoints[i].x;
					ry = endpoints[i].y;
					ax = rx + xgap; // endpoint의 현재 인덱스
					ay = ry + ygap;
					cur_rdistance = Lmap[rx][ry].rdis;

					pair<ptr, DIRECTION> target[8]; // 8방향 이동시의 포인트를 정의 (절대좌표로)
					target[0] = pair<ptr, DIRECTION>(ptr(ax + 1, ay), DIRECTION(LEFT));
					target[1] = pair<ptr, DIRECTION>(ptr(ax - 1, ay), DIRECTION(RIGHT));
					target[2] = pair<ptr, DIRECTION>(ptr(ax, ay + 1), DIRECTION(DOWN));
					target[3] = pair<ptr, DIRECTION>(ptr(ax, ay - 1), DIRECTION(UP));
					target[4] = pair<ptr, DIRECTION>(ptr(ax + 1, ay + 1), DIRECTION(LEFT_DOWN));
					target[5] = pair<ptr, DIRECTION>(ptr(ax + 1, ay - 1), DIRECTION(LEFT_UP));
					target[6] = pair<ptr, DIRECTION>(ptr(ax - 1, ay + 1), DIRECTION(RIGHT_DOWN));
					target[7] = pair<ptr, DIRECTION>(ptr(ax - 1, ay - 1), DIRECTION(RIGHT_UP));

					bool blocked[8] = { 0 };
					bool thereispath = false;
					/// 8방향 검색 ///
					for (int k = 0; k < NUMOFDIR; k++)
					{
						int x = target[k].first.x; // 절대좌표 (상대좌표로 바꾸려면 gap을 뺌)
						int y = target[k].first.y;
						int rxt = x - xgap; // 타겟의 rx, ry
						int ryt = y - ygap;
						if (rxt < 0 || ryt < 0 || rxt > GRIDLENGTH * 2 || ryt > GRIDLENGTH * 2)
						{
							continue;
						}

						/// 계단 사이를 가로질러 가는것 방지 ///
						if (grid[x][y].isstair == true && grid[ax][ay].isstair == true)
							continue;
						/// 계단 사이를 가로질러 가는것 방지 ///

						/// 이동 지점이 장애물이면 안감 ///

						if (k >= 4) // 대각선이동시
						{
							int x1 = ax, x2 = x;
							int y1 = y, y2 = ay;
							if (grid[x1][y1].isobstacle[m_currentlayer] == true || grid[x2][y2].isobstacle[m_currentlayer] == true || grid[x][y].isobstacle[m_currentlayer]
								|| grid[x1][y1].isequipment == true || grid[x2][y2].isequipment == true || grid[x][y].isequipment == true)
							{
								pos.wallblocked.insert(ptr(ax, ay));
									//Lmap[rxt][ryt].isoutline = true;
									//outlines.push_back(ptr(ax, ay));
								continue;
							}
						}
						else
						{
							if (grid[x][y].isobstacle[m_currentlayer] == true || grid[x][y].isequipment == true)
							{
								pos.wallblocked.insert(ptr(ax, ay));
									//Lmap[rxt][ryt].isoutline = true;
									//outlines.push_back(ptr(ax, ay));
								continue;
							}
						}

						/// 이동 코스트 산정 (계단, 사다리, 길에 따라 다름) ///
						if (k < 4)
						{
							cost = DISTANCE_PER_GRID;
						}
						else
							cost = sqrt(2) * DISTANCE_PER_GRID;
						if (grid[x][y].isstair == true)
						{
							cost += STAIR_PENALTY[grid[x][y].stairindex];
						}
						/// 이동 코스트 산정 (계단, 사다리, 길에 따라 다름) ///

						double rdis = cur_rdistance - cost;

						/// 코스트 부족 판정 ///
						if (rdis < 0) // 코스트 부족하면 이동불가. 다음루프로
						{
							blocked[k] = 1;
							continue;
						}
						/// 코스트 부족 판정 ///

						if (Lmap[rxt][ryt].isflooded == false) // 목록에 없으면
						{
							Lmap[rxt][ryt].isflooded = true;
							Lmap[rxt][ryt].rdis = rdis;
							Lmap[rxt][ryt].dir = target[k].second;
							nextpoints.push_back(ptr(rxt, ryt));					  // 추가
							flooded.push_back(ptr(x, y));					      // 추가
							//finalpoints.push_back(ptr(rx, ry)); // finalpoint만 상대좌표로 넣음
							//Lmap[rx][ry].isoutline = true;
							noescape = true;
							thereispath = true;
						}
						else // 이미 목록에 있으면
						{
							if (Lmap[rxt][ryt].rdis < rdis)
							{

								Lmap[rxt][ryt].rdis = rdis;
								//	Lmap[rxt][ryt].dir = target[k].second; // 방향 역시 갱신
								noescape = true;
								thereispath = true;
								nextpoints.push_back(ptr(rxt, ryt));

							}
						}
					}
					
					bool block = false;
					for (int p = 0; p < NUMOFDIR; p++)
					{
						block = block | blocked[p];
					}
					if (block == true) // 하나라도 코스트부족으로 못가면
					{
						if (!firstpoint)
						{
							finalpoints.push_back(ptr(rx, ry)); // finalpoint만 상대좌표로 넣음
							Lmap[rx][ry].isfinalpoint = true;
							//Lmap[rx][ry].isoutline = true;
							//outlines.push_back(ptr(ax, ay)); // first point일 경우
						}
						else
						{
							//Lmap[rx][ry].isoutline = true;
							//outlines.push_back(ptr(ax, ay)); // first point일 경우
						}
					}
					
				}


				if (noescape == false) // 더이상 변화가 없으면
					break;
				flag = true;
				endpoints.clear();
				endpoints = nextpoints;
				nextpoints.clear();
				firstpoint = false;
			}

		}
		for (int j = 0; j < Lmap.size(); j++)
		{
			for (int k = 0; k < Lmap[j].size(); k++)
			{
				Lmap[j][k].extended = false;
			}
		}
		if (flag == 0) // 더이상 iteration이 필요없으면
		{
			break;	   // 종료
		}
	}
	double iteration_time2 = (double)(clock() - iteration_time) / CLOCKS_PER_SEC;

	outlines.clear();
	for (int i = 0; i < Lmap.size(); i++)
	{
		for (int j = 0; j < Lmap[0].size(); j++)
		{
			if (Lmap[i][j].isflooded)
			{

				int rx2[4] = { i + 1, i - 1, i, i };
				int ry2[4] = { j, j, j + 1, j - 1 };
				bool surrounded = true; // 주위 4칸이 모두 flooding?
				for (int m = 0; m < 4; m++)
				{
					if (rx2[m] < 0 || ry2[m] < 0 || rx2[m] > 2 * GRIDLENGTH || ry2[m] > 2 * GRIDLENGTH)
					{
						surrounded = false;
						break;
					}
					if (Lmap[rx2[m]][ry2[m]].isflooded == false)
					{
						surrounded = false;
						break;
					}
					//surrounded = surrounded & Lmap[rx2[m]][ry2[m]].isflooded; // 모두 true면 surrounded유지
				}
				if (surrounded == false)
				{
					Lmap[i][j].isoutline = true;
					int ax = i + xgap;
					int ay = j + ygap;
					outlines.push_back(ptr(ax, ay));
				}
			}
		}
	}
	//flooded = outlines;
	/*
	flooded.clear();
	for (int i = 0; i < Lmap.size(); i++)
	{
		for (int j = 0; j < Lmap[0].size(); j++)
		{
			if (Lmap[i][j].isflooded)
			{
				int ax = i + xgap;
				int ay = j + ygap;
				flooded.push_back(ptr(ax, ay));
			}
		}
	}
	*/
	//	printf("computing time : %.3lfms\n", iteration_time2 * 1000);

	//	flooded = outlines;
	vector<ptr> temp;
//	for (int i = 0; i < flooded.size(); i += flooded.size() / 35)
//	{
//		temp.push_back(flooded[i]);
//	}
	//flooded = temp;

}




void output::stream(HYDRANT &hyd, bool accurate_mode)
{
	accurate_mode = true;

	if (hyd.type == TYPE_MONITOR)
	{
		Calculate_Stream_by_Monitor(hyd);
		return;
	}
	//accurate_mode = false;

	//printf("making stream...\n");
	clock_t time = clock();

	double STREAMLENGTH_GRID = hyd.streamlength / DISTANCE_PER_GRID;
	hyd.stream.clear();


	vector<pair<ptr, ptr>> result;
	vector<ptr> &outline = hyd.outline;


	/// 가상 이미지에 선들을 그려내는 부분
	IplImage* temps;
	temps = cvCloneImage(m_ents);
	vector<ptr> &result2 = hyd.stream;// = hyd.stream;


	ptr dst = ptr(-1,-1);

	for (int i = 0; i < outline.size(); i++)
	{
		int srcx, srcy, dstx, dsty;
		int blocked = 0;
		
		srcx = outline[i].x; srcy = outline[i].y;
		if (m_grid[srcx][srcy].iswall == true) continue;

		if (m_grid[srcx - 1][srcy].iswall == true) blocked++;
		if (m_grid[srcx + 1][srcy].iswall == true) blocked++;
		if (m_grid[srcx][srcy + 1].iswall == true) blocked++;
		if (m_grid[srcx][srcy - 1].iswall == true) blocked++;

		ptr src = ptr(srcx, srcy);
		STREAMLENGTH_GRID = hyd.streamlength / DISTANCE_PER_GRID;

		double step = 120 / STREAMLENGTH_GRID;
		if (accurate_mode == true)
			step = 120 / STREAMLENGTH_GRID;
		for (double k = 0; k < 360; k += step) // resolution 조절 가능
		{
			if (m_grid[srcx - 1][srcy].iswall == true && k > 90 && k < 270)
			{
				k = 270;
			}
			if (m_grid[srcx + 1][srcy].iswall == true && (k > 270 || k < 90))
			{
				if (k < 90) {k = 90;}
				if (k > 270) break;
			}
			if (m_grid[srcx][srcy + 1].iswall == true && k < 180)
			{
				k = 180;
			}
			if (m_grid[srcx][srcy - 1].iswall == true && k > 180)
			{
				break;
			}


			double rad = k * 3.1415 / 180;

			dstx = srcx + (int)(cos(rad)*STREAMLENGTH_GRID);
			dsty = srcy + (int)(sin(rad)*STREAMLENGTH_GRID);

			
			
			if (dstx == dst.x && dsty == dst.y)
				continue;
			dst = ptr(dstx, dsty);
			
			bool flag = false;
			int loc_x = dstx - hyd.rel_x;
			int loc_y = dsty - hyd.rel_y;
			if (loc_x < 0 || loc_x >= hyd.LMap.size() || loc_y < 0 || loc_y >= hyd.LMap[0].size())
			{

			}
			else
			{
				if (hyd.LMap[loc_x][loc_y].isflooded == true) // destination이 flood되어있
				{
					int x = srcx + (int)2*cos(rad);
					int y = srcy + (int)2*sin(rad);
					loc_x = x - hyd.rel_x;
					loc_y = y - hyd.rel_y;
					if (loc_x < 0 || loc_x >= hyd.LMap.size() || loc_y < 0 || loc_y >= hyd.LMap[0].size())
					{
					
					}
					else if (hyd.LMap[loc_x][loc_y].isflooded == true)
						continue;
					else
						flag = true;
				}
			}
			


		
			if (accurate_mode == false)
			{
				if (getcolor(temps, dst.x * GRIDSIZE + GRIDSIZE / 2, dst.y * GRIDSIZE + GRIDSIZE / 2) == COLOR(C_VIRTUAL.R, C_VIRTUAL.G, C_VIRTUAL.B)) // 이미 있으면
					continue;
			}
			ptr optr;
			if (accurate_mode == false)
				optr = collision_detection_for_stream(src, dst, temps); // 충돌을 감지하기 직전의 좌표를 뽑아냄.
			else
				optr = collision_detection_for_stream(src, dst, NULL); // 충돌을 감지하기 직전의 좌표를 뽑아냄.

			if (optr == src)
				continue;    // 벽에 걸쳐있으면 무시.
			if (optr.x == 0 && optr.y == 0)
			{
				optr = dst;
			}

			if (accurate_mode == false)
			{
				if (getcolor(temps, optr.x * GRIDSIZE + GRIDSIZE / 2, optr.y * GRIDSIZE + GRIDSIZE / 2) == COLOR(C_VIRTUAL.R, C_VIRTUAL.G, C_VIRTUAL.B) && flag == false)
					continue;
			}
			cvLine(temps, CvPoint(src.x*GRIDSIZE + GRIDSIZE / 2, src.y*GRIDSIZE + GRIDSIZE / 2), CvPoint(optr.x*GRIDSIZE + GRIDSIZE / 2, optr.y*GRIDSIZE + GRIDSIZE / 2), CvScalar(C_VIRTUAL.B, C_VIRTUAL.G, C_VIRTUAL.R), GRIDSIZE, 4);

		}
	}
	/// 선들의 2좌표들을 얻는 부분 

	/// 다시 벽에 대해서 벽색깔 덧칠

	for (int i = 0; i < temps->width / GRIDSIZE; i++)
	{
		for (int j = 0; j < temps->height / GRIDSIZE; j++)
		{
			COLOR col = COLOR(C_VIRTUAL.R, C_VIRTUAL.G, C_VIRTUAL.B);
			
			//if (m_grid[i][j].iswall == true)
			//	cvRectangle(temps, CvPoint(i*GRIDSIZE, j*GRIDSIZE), CvPoint(i*GRIDSIZE + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), CvScalar(C_WALL.B, C_WALL.G, C_WALL.R), CV_FILLED);
			if (m_grid[i][j].iswall == false && getcolor(temps, i*GRIDSIZE + GRIDSIZE / 2, j*GRIDSIZE + GRIDSIZE / 2) == col)
				result2.push_back(ptr(i, j));
		}
	}
	
	/// 다시 벽에 대해서 벽색깔 덧칠
	/*
	for (int i = GRIDSIZE + GRIDSIZE / 2; i < temps->width - GRIDSIZE; i += GRIDSIZE)
	{
		for (int j = GRIDSIZE + GRIDSIZE / 2; j < temps->height - GRIDSIZE; j += GRIDSIZE)
		{
			COLOR col = COLOR(C_VIRTUAL.R, C_VIRTUAL.G, C_VIRTUAL.B);
			if (getcolor(temps, i, j) == col && )
				result2.push_back(ptr((i) / GRIDSIZE, (j) / GRIDSIZE));
			
			for (int x = -GRIDSIZE / 2; x <= GRIDSIZE / 2; x++)
			{
				bool flag = false;
				for (int y = -GRIDSIZE / 2; y <= GRIDSIZE / 2; y++)
				{
					if (getcolor(temps, i + x, j + y) == col)
					{
						result2.push_back(ptr((i + GRIDSIZE / 2) / GRIDSIZE, (j + GRIDSIZE / 2) / GRIDSIZE));
						flag = true;
						break;
					}
				}
				if (flag == true)
					break;
			}
			
			//else if (getcolor(temps, i + GRIDSIZE, j) == col && getcolor(temps, i - GRIDSIZE, j) == col && getcolor(temps, i, j + GRIDSIZE) == col && getcolor(temps, i, j - GRIDSIZE) == col)
			//	result2.push_back(ptr((i + GRIDSIZE / 2) / GRIDSIZE, (j + GRIDSIZE / 2) / GRIDSIZE));   /// 물이 새는 에러가 나면 이거 확인해보기
		}
	}
	*/
	// 중복제거
	
	/*
	for (int i = 0; i < hyd.range.size(); i++)
	{
		int x = hyd.range[i].x;
		int y = hyd.range[i].y;
		if (m_grid[x][y].iswall == false)
			result2.push_back(ptr(x, y));
	}
	

	*/
	
	result2.insert(result2.end(), hyd.range.begin(), hyd.range.end());
	sort(result2.begin(), result2.end());
	vector<ptr>::iterator pos;
	pos = unique(result2.begin(), result2.end());
	result2.erase(pos, result2.end());	
	vector<ptr> &temp = result2;
	for (vector<ptr>::iterator it = temp.end() - 2; it != temp.begin(); it--)
	{
		int x = (it + 1)->x, y = (it + 1)->y;
		if (m_grid[x][y].iswall == true)
			temp.erase(it + 1);
	}


	cvReleaseImage(&temps);
	double time2 = (double)(clock() - time) / CLOCKS_PER_SEC;
	//printf("computing time : %.3lfms\n", time2 * 1000);

	//hyd.stream = result2;
}



void output::Calculate_Stream_by_Monitor(HYDRANT &hyd)
{
	HYDRANT &mon = hyd;
	vector<ptr> &result = mon.stream;
	result.clear();

	double length = mon.streamlength; //  sqrt(pow(mon.xx[0] - mon.xx[1], 2) + pow(mon.yy[0] - mon.yy[1], 2));
	double angle1 = atan2(mon.yy[1] - mon.yy[0], mon.xx[1] - mon.xx[0]); // -pi ~ pi의 rad값 반환
	double angle2 = atan2(mon.yy[2] - mon.yy[0], mon.xx[2] - mon.xx[0]);
	double gridlength = length / DISTANCE_PER_GRID;

	double startangle = 0, endangle = 0;
	double sweep = angle2 - angle1;

	if (sweep >= 0)
	{
		startangle = min(angle1, angle2);
		endangle = max(angle1, angle2);
	}
	else
	{
		startangle = max(angle1, angle2);
		endangle = min(angle1, angle2) + 2 * 3.1415;
	}

	int srcx, srcy, dstx, dsty;
	srcx = mon.xx[0]; srcy = mon.yy[0];
	for (double k = startangle; k <= endangle; k += 0.21 / gridlength)
	{
		int j = 0;
		dstx = srcx;
		dsty = srcy;
		for (double i = 0.5; i <= gridlength; i += 0.5)
		{
			if (i + 0.5 > gridlength)
			{
				if (j % 2 == 0)
					dstx = srcx + (int)(cos(k)*gridlength);
				else
					dsty = srcy + (int)(sin(k)*gridlength);
			}
			if (j % 2 == 0)
				dstx = srcx + (int)(cos(k)*i);
			else
				dsty = srcy + (int)(sin(k)*i);
			if (m_grid[dstx][dsty].iswall == true)
			{
				break;
			}
			else
				result.push_back(ptr(dstx, dsty));
			j++;
		}
	}
	sort(result.begin(), result.end());
	vector<ptr>::iterator pos;
	pos = unique(result.begin(), result.end());
	result.erase(pos, result.end());
}


vector<ptr> output::padding(vector<ptr> vec)
{
	vector<ptr> stored = vec;
	for (int j = 0; j < vec.size() - 1; j++)
	{
		int x1, x2, y1, y2;
		x1 = vec[j].x;  x2 = vec[j + 1].x; y1 = vec[j].y; y2 = vec[j + 1].y;
		double dist = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
		double deltax, deltay;
		deltax = (x2 - x1) / dist;
		deltay = (y2 - y1) / dist;
		vector<ptr> temp;
		for (int k = 1; k < dist / SLICERATE; k++)
		{
			ptr p = ptr(round(x1 + deltax*k*SLICERATE), round(y1 + deltay*k*SLICERATE));
			if (m_grid[p.x][p.y].isobstacle[m_currentlayer] == true || m_grid[p.x][p.y].isequipment == true)
				continue;
			if (find(vec.begin(), vec.end(), p) == vec.end())
				temp.push_back(p);
		}
		vector<ptr>::iterator it = find(stored.begin(), stored.end(), ptr(vec[j]));

		stored.insert(it + 1, temp.begin(), temp.end());  // 중간 값들을 모두 삽입
		temp.clear();														  // 클리어
	}
	return stored;
}



vector<ptr> output::three_ptr_smoothing(vector<ptr> vec, bool reversed)
{

	if (reversed)
		reverse(vec.begin(), vec.end());
	vector<ptr>::iterator it = vec.begin();
	while (1)
	{
		if ((it + 1) == vec.end() || (it + 2) == vec.end())
			break;

		/// 최근 3점을 얻음,
		ptr p1 = *it;
		ptr p3 = *(it + 2);
		/// p1과 p3을 이어서 장애물이 없으면

		if (collision_detection_for_path(p1, p3, false))  // collision이 감지되면
		{
			it++;   // 값을 더하고 다음 iteration으로
			continue;
		}
		else  // coolision이 없으면
		{
			vec.erase(it + 1); // 중간부분을 지운다.
		}
	}
	return vec;
}



vector<ptr> output::four_ptr_smoothing(vector<ptr> vec, bool reversed) // 4point smoothing을 수행하는 함수
{
	if (reversed)
		reverse(vec.begin(), vec.end());
	vector<ptr>::iterator it = vec.begin();
	while (1)
	{
		if (it == vec.end() || it + 1 == vec.end() || it + 2 == vec.end() || it + 3 == vec.end())
		{
			break;
		}
		ptr p1 = *it;  ptr p4 = *(it + 3); // 대각선
		ptr p2 = ptr(p1.x, p4.y);   // 사각형을 채움
		ptr p3 = ptr(p4.x, p1.y);

		if (collision_detection_for_path(p1, p2, true) == false && collision_detection_for_path(p2, p4, true) == false)  // 삼각형 1에 충돌이 없음 (우선권 삼각형 1)
		{
			vec.erase(it + 2);
			vec.erase(it + 1);

			vec.insert(it + 1, p2);
		}
		else if (collision_detection_for_path(p1, p3, true) == false && collision_detection_for_path(p3, p4, true) == false)  // 삼각형1에 충돌이 있고 2에 충돌이 없으면
		{
			vec.erase(it + 2);
			vec.erase(it + 1);

			vec.insert(it + 1, p3);
		}
		else
			it++;
	}
	return vec;
}



double output::totaldistance(vector<ptr> path, double total_stairpenalty)  // 생성된 길로부터 총 길이를 구함.
{
	double sum = 0;
	double distance = 0;
	for (int i = 0; i < path.size() - 1; i++)
	{
		distance += sqrt(pow(path[i].x - path[i + 1].x, 2) + pow(path[i].y - path[i + 1].y, 2)) * DISTANCE_PER_GRID;
	}
	sum += distance;
	double result = sum + total_stairpenalty;
	if (result == 0)
		return 1;
	else
		return result;
}


ptr output::collision_detection_for_stream(ptr p1, ptr p2, IplImage *img) // 물줄기 뿜을 때 벽 감지
{
	if (m_grid[p1.x][p1.y].iswall == 1)
		return p1;
	if (p1 == p2)
		return p1;

	int xmax = m_ents->width / GRIDSIZE;
	int ymax = m_ents->height / GRIDSIZE;

	double distance = sqrt(pow(p2.y - p1.y, 2) + pow(p2.x - p1.x, 2));
	double deltax = (p2.x - p1.x) / distance;
	double deltay = (p2.y - p1.y) / distance;

	int act_x = p1.x, act_y = p1.y, pre_x = p1.x, pre_y = p1.y; // round된 결과
	int j = 0;
	bool colorscanstart = false;
	for (double i = 0.5; i <= distance; i += 0.5)
	{
		if (j % 2 == 0)
			act_x = p1.x + (int)(deltax*i); //floored(deltax, i);
		else if (j % 2 == 1)
			act_y = p1.y + (int)(deltay*i); //floored(deltay, i);
	//	if (act_x == pre_x && act_y == pre_y)
//			continue;
		if (img != NULL)
		{

			if (colorscanstart == false && !(getcolor(img, act_x * GRIDSIZE + GRIDSIZE / 2, act_y * GRIDSIZE + GRIDSIZE / 2) == COLOR(C_VIRTUAL.R, C_VIRTUAL.G, C_VIRTUAL.B)))
				colorscanstart = true;
		}

		if (act_x <= 0 || act_y <= 0 || act_x >= xmax || act_y >= ymax)
			return ptr(pre_x, pre_y);
		if (m_grid[act_x][act_y].iswall == true)
			return ptr(pre_x, pre_y);
		if (img != NULL)
		{
			if (colorscanstart && getcolor(img, act_x * GRIDSIZE + GRIDSIZE / 2, act_y * GRIDSIZE + GRIDSIZE / 2) == COLOR(C_VIRTUAL.R, C_VIRTUAL.G, C_VIRTUAL.B))
				return ptr(pre_x, pre_y);
		}
		pre_x = act_x;
		pre_y = act_y;
		j++;
	}
	if (j % 2 == 0)
		act_x = p1.x + (int)(deltax*distance);
	else
		act_y = p1.y + (int)(deltay*distance);
	if (m_grid[act_x][act_y].iswall == 1)
		return ptr(pre_x, pre_y);
	else if (m_grid[act_x + 1][act_y].iswall != 1 && m_grid[act_x - 1][act_y].iswall != 1 && m_grid[act_x][act_y + 1].iswall != 1 && m_grid[act_x][act_y - 1].iswall != 1)
		return ptr(0, 0);
	else
		return ptr(act_x, act_y);
}


bool output::collision_detection_for_path(ptr p1, ptr p2, bool fpoint) // 빨강에 대한 collision detection
{
	if (!fpoint)
	{
		if (p2.x == p1.x || p1.y == p2.y) // 직선형태면 collision 없음
			return false;
	}

	if (m_grid[p2.x][p2.y].isobstacle[m_currentlayer] == 1 || m_grid[p2.x][p2.y].isequipment == 1)
	{
		return true;
	}



	double distance = floor(sqrt(pow(p2.y - p1.y, 2) + pow(p2.x - p1.x, 2)));
	double deltax = (p2.x - p1.x) / distance;
	double deltay = (p2.y - p1.y) / distance;

	int act_x = p1.x, act_y = p1.y; // round된 결과
	int j = 0;
	double i = 0;
	for (i = 0.5; i <= distance; i += 0.5)
	{
		if (j % 2 == 0)
			act_x = p1.x + (int)(deltax*i);
		else if (j % 2 == 1)
			act_y = p1.y + (int)(deltay*i);

		if (m_grid[act_x][act_y].isobstacle[m_currentlayer] == 1 || m_grid[act_x][act_y].isequipment == 1)
			return true;
		j++;
	}
	return false;
}



double output::Get_route_overlapped_area(int hydnum1, int hydnum2)  // vec1에서 vec2가 차지하는 비율, 같은 소스넘버면 -1 반환
{
	if ((m_hydpos[hydnum1].sourcenum != 0) && m_hydpos[hydnum1].sourcenum == m_hydpos[hydnum2].sourcenum)
	{
		return -1;
	}
	set<ptr> vec1, vec2, vec3;
	vec1.insert(m_hydpos[hydnum1].range.begin(), m_hydpos[hydnum1].range.end());
	vec2.insert(m_hydpos[hydnum2].range.begin(), m_hydpos[hydnum2].range.end());
	int sum = vec1.size() + vec2.size();
	vec1.insert(m_hydpos[hydnum2].range.begin(), m_hydpos[hydnum2].range.end());
	int unionsum = vec1.size();


	double count = sum - unionsum;
	return count * pow(DISTANCE_PER_GRID, 2);
}
double output::Get_stream_overlapped_area(int hydnum1, int hydnum2)  // vec1에서 vec2가 차지하는 비율
{
	set<ptr> vec1, vec2, vec3;
	vec1.insert(m_hydpos[hydnum1].stream.begin(), m_hydpos[hydnum1].stream.end());
	vec2.insert(m_hydpos[hydnum2].stream.begin(), m_hydpos[hydnum2].stream.end());
	int sum = vec1.size() + vec2.size();
	vec1.insert(m_hydpos[hydnum2].stream.begin(), m_hydpos[hydnum2].stream.end());
	int unionsum = vec1.size();


	double count = sum - unionsum;
	return count * pow(DISTANCE_PER_GRID, 2);
}