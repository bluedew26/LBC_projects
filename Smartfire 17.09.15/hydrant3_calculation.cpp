#include "stdafx.h"
#include "hydrant.h"

void output::flood(HYDRANT &pos, int numofiter) // m_grid�� �迭 ����. ù��° Index�� ���� ��ȣ, �ι�°���� ��ǥ
{
//	static bool first = true;
	//if (first == true)
	//{
	//	numofiter = 1;
	//	first = false;
	//}


	/// �ð� üũ
	//	printf("now flooding... ( : %d, hydnum : %d)\n". hydnum);

	clock_t iteration_time = clock();
	///
	if (pos.type == TYPE_MONITOR) // ������ϰ�� �ٸ��� ����
		return;

	vector<vector<LocMap>> &Lmap = pos.LMap;
	vector<vector<Node>> &grid = m_grid;

	// local map �Ҵ� //
	pos.reset();
	int GRIDLENGTH = pos.length / DISTANCE_PER_GRID;
	Lmap.assign(GRIDLENGTH * 2 + 1, vector<LocMap>());
	for (int i = 0; i < Lmap.size(); i++)
	{
		Lmap[i].assign(GRIDLENGTH * 2 + 1, LocMap());
	}
	// local map �Ҵ� //

	// ��� ��ǥ ȹ�� //

	int rx = GRIDLENGTH;
	int ry = GRIDLENGTH;
	// ��� ��ǥ ȹ�� //

	int ax = pos.x; // ������ǥ
	int ay = pos.y;

	int xgap = ax - rx; // gap�� ���ϸ� �����ǥ�� ������ǥ�� ���� �� ����
	int ygap = ay - ry; // gap�� ���ϸ� �����ǥ�� ������ǥ�� ���� �� ����

	pos.rel_x = xgap;
	pos.rel_y = ygap;
	/// �ʱ�ȭ 
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
	endpoints.push_back(ptr(rx, ry)); // �����ǥ�� ����
	flooded.push_back(ptr(ax, ay));
	/// ���� ����
	double cur_rdistance = 0.0; // ���� ���� ����
	///

	/// 1�� flooding ///


#define NUMOFDIR 8
	while (1)
	{
		bool noescape = false;  // ���� Ż��� ���õ� �÷���

		for (int i = 0; i < endpoints.size(); i++)
		{


			double cost = 0;
			rx = endpoints[i].x;
			ry = endpoints[i].y;
			ax = rx + xgap; // endpoint�� ���� �ε���
			ay = ry + ygap;

			cur_rdistance = Lmap[rx][ry].rdis;

			pair<ptr, DIRECTION> target[8]; // 8���� �̵����� ����Ʈ�� ���� (������ǥ��)
			target[0] = pair<ptr, DIRECTION>(ptr(ax + 1, ay), DIRECTION(LEFT));
			target[1] = pair<ptr, DIRECTION>(ptr(ax - 1, ay), DIRECTION(RIGHT));
			target[2] = pair<ptr, DIRECTION>(ptr(ax, ay + 1), DIRECTION(DOWN));
			target[3] = pair<ptr, DIRECTION>(ptr(ax, ay - 1), DIRECTION(UP));
			target[4] = pair<ptr, DIRECTION>(ptr(ax + 1, ay + 1), DIRECTION(LEFT_DOWN));
			target[5] = pair<ptr, DIRECTION>(ptr(ax + 1, ay - 1), DIRECTION(LEFT_UP));
			target[6] = pair<ptr, DIRECTION>(ptr(ax - 1, ay + 1), DIRECTION(RIGHT_DOWN));
			target[7] = pair<ptr, DIRECTION>(ptr(ax - 1, ay - 1), DIRECTION(RIGHT_UP));

			bool blocked[8] = { 0 };


			/// 8���� �˻� ///
			for (int k = 0; k < NUMOFDIR; k++)
			{
				int x = target[k].first.x; // ������ǥ (�����ǥ�� �ٲٷ��� gap�� ��)
				int y = target[k].first.y;
				int rxt = x - xgap; // Ÿ���� rx, ry
				int ryt = y - ygap;


				if (rxt < 0 || ryt < 0 || rxt > GRIDLENGTH * 2 || ryt > GRIDLENGTH * 2)
				{
					continue;
				}


				/// ��� ���̸� �������� ���°� ���� ///
				if (grid[x][y].isstair == true && grid[ax][ay].isstair == true)
					continue;
				/// ��� ���̸� �������� ���°� ���� ///

				/// �̵� ������ ��ֹ��̸� �Ȱ� ///

				if (k >= 4) // �밢���̵���
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

				/// �̵� �ڽ�Ʈ ���� (���, ��ٸ�, �濡 ���� �ٸ�) ///
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
				/// �̵� �ڽ�Ʈ ���� (���, ��ٸ�, �濡 ���� �ٸ�) ///

				double rdis = cur_rdistance - cost;

				/// �ڽ�Ʈ ���� ���� ///
				if (rdis < 0) // �ڽ�Ʈ �����ϸ� �̵��Ұ�. ����������
				{
					blocked[k] = true;
					continue;
				}
				/// �ڽ�Ʈ ���� ���� ///

				if (Lmap[rxt][ryt].isflooded == false) // ��Ͽ� ������
				{
					Lmap[rxt][ryt].isflooded = true;
					Lmap[rxt][ryt].rdis = rdis;
					Lmap[rxt][ryt].dir = target[k].second;
					nextpoints.push_back(ptr(rxt, ryt));					  // �߰�
					flooded.push_back(ptr(x, y));					      // �߰�
				//	Lmap[rx][ry].isoutline = true;
					noescape = true;
				}
				else // �̹� ��Ͽ� ������
				{

					if (Lmap[rxt][ryt].rdis < rdis) // �� ���� remain_dis�� ���� �� �ִ� ���
					{
						Lmap[rxt][ryt].rdis = rdis;
						Lmap[rxt][ryt].dir = target[k].second; // ���� ���� ����
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
			if (block == true) // �ϳ��� �ڽ�Ʈ�������� ������
			{
				finalpoints.push_back(ptr(rx, ry)); // finalpoint�� �����ǥ�� ����
				Lmap[rx][ry].isfinalpoint = true;
			//	Lmap[rx][ry].isoutline = true;
			}
			
		}
		if (noescape == false) // ���̻� ��ȭ�� ������
			break;
		endpoints.clear();
		endpoints = nextpoints;
		nextpoints.clear();
	}
	/// �ʱ� flooding �κ� ///, �� �������� �������� finalpoints�鿡 ����.
	nextpoints.clear();

	int iteration = 1;

	/// �ļ� flooding �κ� ///
	while (1) // iteration�� �Ұ����Ҷ����� �ݺ�
	{
		vector<ptr> fpts = finalpoints;
		finalpoints.clear();

		if (iteration >= numofiter && numofiter != 0) // ���ϴ� iteration Ƚ���� �ʰ��� ��� ����;
			break;



		iteration++;


		bool flag = 0;  // ��ȭ�� �����ϸ� flag on, ��ȭ�� ������ �������� ��������


		/// ���̸� ���ϴ� �κ� ///
		for (int t = 0; t < fpts.size(); t++)
		{
			// 4���� �� flood�Ǿ������� ����
			rx = fpts[t].x;
			ry = fpts[t].y; // �����ǥ ����

			int rx2[4] = { rx + 1, rx - 1, rx, rx };
			int ry2[4] = { ry, ry, ry + 1, ry - 1 };
			bool surrounded = true; // ���� 4ĭ�� ��� flooding?
			for (int m = 0; m < 4; m++)
			{
				if (rx2[m] < 0 || ry2[m] < 0 || rx2[m] > 2 * GRIDLENGTH || ry2[m] > 2 * GRIDLENGTH)
				{
					surrounded = false;
					break;
				}
				surrounded = surrounded & Lmap[rx2[m]][ry2[m]].isflooded; // ��� true�� surrounded����
			}
			if (surrounded == true)
			{
				Lmap[rx][ry].isoutline = false;
				continue;
			}

			ax = rx + xgap;
			ay = ry + ygap;


			vector<vector<ptr>> temppaths;   /// ���� ����
			vector<ptr> temppath;



			bool firstpoint = true;


			temppath.push_back(ptr(ax, ay)); /// ���� ������ �������� ����.
			double total_stairpenalty = 0;
			/// �������� ���� ������ ��θ� �����ϴ� �κ�. ///
			while (1)
			{
				/// fptr�� rx�� ry�� �̹� �����.
				if (grid[ax][ay].isstair == true) // ��ܸ�����
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
				temppath.push_back(ptr(ax, ay)); // �װ��� ��ο� ����.
				if (ax == pos.x && ay == pos.y) // �������� ���ƿ���
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

				if (abs(totaldistance(padded, total_stairpenalty) - totaldistance(smoothen3, total_stairpenalty)) / totaldistance(smoothen3, total_stairpenalty) < 0.1)  // ���� 10% �̳�
				{
					temppath = padded;
					break;
				}
				else
				{
					smoothen3 = padded;
				}
				iterated++;

				if (iterated >= 3) // �ִ� �ݺ��� ����
				{
					temppath = padded;
					break;
				}
			}
			/// ������� ���� ���� ����
			





			rx = fpts[t].x;
			ry = fpts[t].y;
			ax = rx + xgap;
			ay = rx + ygap;
			remain_dis = avail_dis - totaldistance(temppath, total_stairpenalty);
			endpoints.clear();
			nextpoints.clear();
			Lmap[rx][ry].rdis = remain_dis;
			endpoints.push_back(ptr(rx, ry)); // �����ǥ

			/// �������� flooding �������� (�Ѱ��� final point�� ����?)
			while (1)
			{

				// ���� Ż�� ��� �����ϱ� //
				bool noescape = false;

				for (int i = 0; i < endpoints.size(); i++)
				{
					double cost = 0;
					rx = endpoints[i].x;
					ry = endpoints[i].y;
					ax = rx + xgap; // endpoint�� ���� �ε���
					ay = ry + ygap;
					cur_rdistance = Lmap[rx][ry].rdis;

					pair<ptr, DIRECTION> target[8]; // 8���� �̵����� ����Ʈ�� ���� (������ǥ��)
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
					/// 8���� �˻� ///
					for (int k = 0; k < NUMOFDIR; k++)
					{
						int x = target[k].first.x; // ������ǥ (�����ǥ�� �ٲٷ��� gap�� ��)
						int y = target[k].first.y;
						int rxt = x - xgap; // Ÿ���� rx, ry
						int ryt = y - ygap;
						if (rxt < 0 || ryt < 0 || rxt > GRIDLENGTH * 2 || ryt > GRIDLENGTH * 2)
						{
							continue;
						}

						/// ��� ���̸� �������� ���°� ���� ///
						if (grid[x][y].isstair == true && grid[ax][ay].isstair == true)
							continue;
						/// ��� ���̸� �������� ���°� ���� ///

						/// �̵� ������ ��ֹ��̸� �Ȱ� ///

						if (k >= 4) // �밢���̵���
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

						/// �̵� �ڽ�Ʈ ���� (���, ��ٸ�, �濡 ���� �ٸ�) ///
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
						/// �̵� �ڽ�Ʈ ���� (���, ��ٸ�, �濡 ���� �ٸ�) ///

						double rdis = cur_rdistance - cost;

						/// �ڽ�Ʈ ���� ���� ///
						if (rdis < 0) // �ڽ�Ʈ �����ϸ� �̵��Ұ�. ����������
						{
							blocked[k] = 1;
							continue;
						}
						/// �ڽ�Ʈ ���� ���� ///

						if (Lmap[rxt][ryt].isflooded == false) // ��Ͽ� ������
						{
							Lmap[rxt][ryt].isflooded = true;
							Lmap[rxt][ryt].rdis = rdis;
							Lmap[rxt][ryt].dir = target[k].second;
							nextpoints.push_back(ptr(rxt, ryt));					  // �߰�
							flooded.push_back(ptr(x, y));					      // �߰�
							//finalpoints.push_back(ptr(rx, ry)); // finalpoint�� �����ǥ�� ����
							//Lmap[rx][ry].isoutline = true;
							noescape = true;
							thereispath = true;
						}
						else // �̹� ��Ͽ� ������
						{
							if (Lmap[rxt][ryt].rdis < rdis)
							{

								Lmap[rxt][ryt].rdis = rdis;
								//	Lmap[rxt][ryt].dir = target[k].second; // ���� ���� ����
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
					if (block == true) // �ϳ��� �ڽ�Ʈ�������� ������
					{
						if (!firstpoint)
						{
							finalpoints.push_back(ptr(rx, ry)); // finalpoint�� �����ǥ�� ����
							Lmap[rx][ry].isfinalpoint = true;
							//Lmap[rx][ry].isoutline = true;
							//outlines.push_back(ptr(ax, ay)); // first point�� ���
						}
						else
						{
							//Lmap[rx][ry].isoutline = true;
							//outlines.push_back(ptr(ax, ay)); // first point�� ���
						}
					}
					
				}


				if (noescape == false) // ���̻� ��ȭ�� ������
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
		if (flag == 0) // ���̻� iteration�� �ʿ������
		{
			break;	   // ����
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
				bool surrounded = true; // ���� 4ĭ�� ��� flooding?
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
					//surrounded = surrounded & Lmap[rx2[m]][ry2[m]].isflooded; // ��� true�� surrounded����
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


	/// ���� �̹����� ������ �׷����� �κ�
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
		for (double k = 0; k < 360; k += step) // resolution ���� ����
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
				if (hyd.LMap[loc_x][loc_y].isflooded == true) // destination�� flood�Ǿ���
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
				if (getcolor(temps, dst.x * GRIDSIZE + GRIDSIZE / 2, dst.y * GRIDSIZE + GRIDSIZE / 2) == COLOR(C_VIRTUAL.R, C_VIRTUAL.G, C_VIRTUAL.B)) // �̹� ������
					continue;
			}
			ptr optr;
			if (accurate_mode == false)
				optr = collision_detection_for_stream(src, dst, temps); // �浹�� �����ϱ� ������ ��ǥ�� �̾Ƴ�.
			else
				optr = collision_detection_for_stream(src, dst, NULL); // �浹�� �����ϱ� ������ ��ǥ�� �̾Ƴ�.

			if (optr == src)
				continue;    // ���� ���������� ����.
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
	/// ������ 2��ǥ���� ��� �κ� 

	/// �ٽ� ���� ���ؼ� ������ ��ĥ

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
	
	/// �ٽ� ���� ���ؼ� ������ ��ĥ
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
			//	result2.push_back(ptr((i + GRIDSIZE / 2) / GRIDSIZE, (j + GRIDSIZE / 2) / GRIDSIZE));   /// ���� ���� ������ ���� �̰� Ȯ���غ���
		}
	}
	*/
	// �ߺ�����
	
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
	double angle1 = atan2(mon.yy[1] - mon.yy[0], mon.xx[1] - mon.xx[0]); // -pi ~ pi�� rad�� ��ȯ
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

		stored.insert(it + 1, temp.begin(), temp.end());  // �߰� ������ ��� ����
		temp.clear();														  // Ŭ����
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

		/// �ֱ� 3���� ����,
		ptr p1 = *it;
		ptr p3 = *(it + 2);
		/// p1�� p3�� �̾ ��ֹ��� ������

		if (collision_detection_for_path(p1, p3, false))  // collision�� �����Ǹ�
		{
			it++;   // ���� ���ϰ� ���� iteration����
			continue;
		}
		else  // coolision�� ������
		{
			vec.erase(it + 1); // �߰��κ��� �����.
		}
	}
	return vec;
}



vector<ptr> output::four_ptr_smoothing(vector<ptr> vec, bool reversed) // 4point smoothing�� �����ϴ� �Լ�
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
		ptr p1 = *it;  ptr p4 = *(it + 3); // �밢��
		ptr p2 = ptr(p1.x, p4.y);   // �簢���� ä��
		ptr p3 = ptr(p4.x, p1.y);

		if (collision_detection_for_path(p1, p2, true) == false && collision_detection_for_path(p2, p4, true) == false)  // �ﰢ�� 1�� �浹�� ���� (�켱�� �ﰢ�� 1)
		{
			vec.erase(it + 2);
			vec.erase(it + 1);

			vec.insert(it + 1, p2);
		}
		else if (collision_detection_for_path(p1, p3, true) == false && collision_detection_for_path(p3, p4, true) == false)  // �ﰢ��1�� �浹�� �ְ� 2�� �浹�� ������
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



double output::totaldistance(vector<ptr> path, double total_stairpenalty)  // ������ ��κ��� �� ���̸� ����.
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


ptr output::collision_detection_for_stream(ptr p1, ptr p2, IplImage *img) // ���ٱ� ���� �� �� ����
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

	int act_x = p1.x, act_y = p1.y, pre_x = p1.x, pre_y = p1.y; // round�� ���
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


bool output::collision_detection_for_path(ptr p1, ptr p2, bool fpoint) // ������ ���� collision detection
{
	if (!fpoint)
	{
		if (p2.x == p1.x || p1.y == p2.y) // �������¸� collision ����
			return false;
	}

	if (m_grid[p2.x][p2.y].isobstacle[m_currentlayer] == 1 || m_grid[p2.x][p2.y].isequipment == 1)
	{
		return true;
	}



	double distance = floor(sqrt(pow(p2.y - p1.y, 2) + pow(p2.x - p1.x, 2)));
	double deltax = (p2.x - p1.x) / distance;
	double deltay = (p2.y - p1.y) / distance;

	int act_x = p1.x, act_y = p1.y; // round�� ���
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



double output::Get_route_overlapped_area(int hydnum1, int hydnum2)  // vec1���� vec2�� �����ϴ� ����, ���� �ҽ��ѹ��� -1 ��ȯ
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
double output::Get_stream_overlapped_area(int hydnum1, int hydnum2)  // vec1���� vec2�� �����ϴ� ����
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