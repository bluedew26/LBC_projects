#include "stdafx.h"
#include "hydrant.h"
#include "functions.h"
#include <queue>


#define MODE_RANDOM 0     // 백지에서 시작
#define MODE_ALLOCATED 1  // 미리 배치된 소화전으로부터 시작
#define MODE_DETERMINISTIC_ALLOCATION 2 // 준비된 알고리즘에 의해 초기 배치


void output::Opt_Init_Option1_deterministic(double defaultlength, double defaultstreamlength, bool isoption1, double goal_occupancy, bool fixed)
{
	/// Parameter 설정 ///
	cur_iter = 1; // iteration 0부터(팔각형) 시작
	cur_index = 0; // 가장 마지막 index부터 시작하도록
	fixmode = fixed;
	Default_Length = defaultlength;
	Default_Stream_Length = defaultstreamlength;
	single_n_double_coverages = pair<double, double>(0, 0);
	prev_single_n_double_coverages = pair<double, double>(0, 0);

	for (int i = 0; i < 100; i++)
	{
		local_updated[i] = true;
	}

	if (goal_occupancy > 1 || isoption1 == false)
		goal_occupancy = 1;
	required_occupy_rate = goal_occupancy; // 90% 이상 달성이 조건

	single_groups.clear();
	
	vector<vector<Node>> cur_grid = m_grid; // feasible에서 점점 제외해나감
	set<ptr> cur_occupied;
	/// 기존에 목록에 있던 hydrant들 추가
	if (fixmode)
	{
		for (int i = 0; i < m_hydpos.size(); i++)
		{
			int x = m_hydpos[i].x;
			int y = m_hydpos[i].y;
			double length = m_hydpos[i].length;
			double streamlength = m_hydpos[i].streamlength;
			int sourcenum = m_hydpos[i].sourcenum;
			single_groups.push_back(optptr(x, y, length, 0, 0, streamlength, sourcenum)); /// 이 시점에서 추가됨
			single_groups.back().fixed = true;
			single_groups.back().hyd.reset();
			flood(single_groups.back().hyd, cur_iter);
			if (!isoption1)
			{
				stream(single_groups.back().hyd);
			}
			int size = 0;

			if (isoption1 == true)
			{
				cur_occupied.insert(single_groups.back().hyd.range.begin(), single_groups.back().hyd.range.end());
				size = single_groups.back().hyd.range.size();
			}
			else
			{
				cur_occupied.insert(single_groups.back().hyd.stream.begin(), single_groups.back().hyd.stream.end());
				size = single_groups.back().hyd.stream.size();
			}
			for (int j = 0; j < size; j++)
			{
				int xx = 0, yy = 0;
				if (isoption1 == true)
				{
					xx = single_groups.back().hyd.range[j].x;
					yy = single_groups.back().hyd.range[j].y;
				}
				else
				{
					xx = single_groups.back().hyd.stream[j].x;
					yy = single_groups.back().hyd.stream[j].y;
				}
				cur_grid[xx][yy].multipurpose = true;
				cur_grid[xx][yy].isfeasible = false;
			}
		}
	}
	m_hydpos.clear();




	vector<optptr> initially_defined_groups;



	/// 초기 삽입 과정
	srand(time(NULL)); // 난수 초기화

	m_hydpos.clear();
	
	
	
	//vector<optptr> groupset;
	
	/*
	single_groups.push_back(optptr(96, 93, defaultlength, 0, 0, defaultstreamlength, 0));
	single_groups.push_back(optptr(141, 42, defaultlength, 0, 0, defaultstreamlength, 0));
	single_groups.push_back(optptr(66, 24, defaultlength, 0, 0, defaultstreamlength, 0));
	single_groups.push_back(optptr(42, 48, defaultlength, 0, 0, defaultstreamlength, 0));
	Update_Particle(single_groups, !isoption1);

	set<ptr> sett;
	
	for (int i = 0; i < single_groups.size(); i++)
	{
		sett.insert(single_groups[i].hyd.stream.begin(), single_groups[i].hyd.stream.end());
	}

	double occupying_rate = (double)sett.size() / m_streamable_point.size();
	if (occupying_rate > required_occupy_rate - 0.02)
		goto there;
		*/
	/*
	for (int i = 0; i < cur_grid.size(); i++) // 체크용으로 쓸 multipurpose를 false로
	{
		for (int j = 0; j < cur_grid[i].size(); j++)
		{
			cur_grid[i][j].multipurpose = false;
		}
	}
	*/
	
	double length = 0;
	int index = 0;
	while (1)
	{
		if (index >= length_array.size())
		{
			length_array.push_back(Default_Length);
		}
		length = length_array[index];
		

		int total_area = 0;
		if (isoption1 == true)
		{
			total_area = m_floodable_point.size();
		}
		else
		{
			total_area = m_streamable_point.size();
		}

		int max_score = 0;
		int maxx_cod = 0, maxy_cod = 0; // 가장 최적의 값을 가지는 지점의 x 좌표, y 좌표
		HYDRANT max_hydrant;
		int RESOLUTION_STEP = 40 / GRIDSIZE;
		if (RESOLUTION_STEP == 0)
			RESOLUTION_STEP = 1;
		if (isoption1 == false)
			RESOLUTION_STEP *= 2;
		for (int i = 0; i < cur_grid.size(); i += RESOLUTION_STEP)
		{
			for (int j = 0; j < cur_grid[i].size(); j += RESOLUTION_STEP)
			{
				if (cur_grid[i][j].isfeasible)
				{
					HYDRANT hyd = HYDRANT(i, j, defaultstreamlength, length, 0, 0, 0);
					flood(hyd, cur_iter);
					if (isoption1 == false) // option2면 stream까지 수행
					{
						stream(hyd);
					}
					int overlapped = 0;

					int size = 0;
					if (isoption1 == true)
						size = hyd.range.size();
					else
						size = hyd.stream.size();
					for (int k = 0; k < size; k++)
					{
						if (isoption1 == true)
						{
							if (cur_grid[hyd.range[k].x][hyd.range[k].y].multipurpose == true) // 이미 기존과 겹친부분이 있으면
								overlapped++;
						}
						else
						{
							if (cur_grid[hyd.stream[k].x][hyd.stream[k].y].multipurpose == true) // 이미 기존과 겹친부분이 있으면
								overlapped++;
						}
					}
					if (cur_occupied.empty() == true || overlapped > 0)
					{
						int occupied = 0;
						if (isoption1 == true)
						{
							occupied = 100000 + hyd.range.size() - 1.2 * overlapped;    // 이중 오버랩 적용
						}
						else
						{
							occupied = 100000 + hyd.stream.size() - 1.2 * overlapped;    // 이중 오버랩 적용
						}
						if (occupied > max_score)//+ hyd.wallblocked.size() > max_score)
						{
							max_score = occupied;// hyd.wallblocked.size();
							maxx_cod = i, maxy_cod = j;
							max_hydrant = hyd;
						}
					}
				}
			}
		}
		if (isoption1 == true)
		{
			cur_occupied.insert(max_hydrant.range.begin(), max_hydrant.range.end());
			for (int k = 0; k < max_hydrant.range.size(); k++)
			{
				cur_grid[max_hydrant.range[k].x][max_hydrant.range[k].y].multipurpose = true;
				cur_grid[max_hydrant.range[k].x][max_hydrant.range[k].y].isfeasible = false;   /// 추가할지 삭제할지 고민중

			}
		}
		else
		{
			cur_occupied.insert(max_hydrant.stream.begin(), max_hydrant.stream.end());
			for (int k = 0; k < max_hydrant.stream.size(); k++)
			{
				cur_grid[max_hydrant.stream[k].x][max_hydrant.stream[k].y].multipurpose = true;
				cur_grid[max_hydrant.stream[k].x][max_hydrant.stream[k].y].isfeasible = false;
			}

		}
		

		printf("적정 위치 발견 \n");
		single_groups.push_back(optptr(maxx_cod, maxy_cod, length, 0, 0, defaultstreamlength, 0));
		double occupying_rate = (double)cur_occupied.size() / total_area;
		printf("%.2lf%% occupied\n", occupying_rate * 100);
		if (single_groups.size() > 13)
			break;
		if (isoption1 == true)
		{
			if (occupying_rate >= required_occupy_rate)
				break;
		}
		else
		{
			if (occupying_rate >= required_occupy_rate - 0.05)
				break;
		}
		index++;
	}
	

	there:

	for (int i = 0; i < single_groups.size(); i++)
	{
		printf("index : %02d, x : %3d, y : %3d\n", i, single_groups[i].hyd.x, single_groups[i].hyd.y);
	}
	
//	if (isoption1 == false)
//		cur_iter = 0;


//	give_score(); // 기존 그리드에 스코어링

	Update_Particle(single_groups, !isoption1);
	m_hydpos.clear();
	for (int i = 0; i < single_groups.size(); i++)
	{
		if (isoption1 == false)
			stream(single_groups[i].hyd, true);
		m_hydpos.push_back(single_groups[i].hyd);
	}
	for (int i = 0; i < single_groups_spared.size(); i++)
	{
		m_hydpos.push_back(single_groups_spared[i].hyd);
	}
	cur_index = single_groups.size() - 1; // 마사지할 최초의 인덱스를 마지막걸로 

}

pair<int, int> output::cal_occupied_and_overlapped(vector<optptr> &container, int index, bool isoption1)
{
	set<ptr> groups;
	int overlapped = 0;
	int occupied = 0;

	vector<vector<Node>> &union_all = m_grid;
	for (int i = 0; i < union_all.size(); i++)
	{
		for (int j = 0; j < union_all[i].size(); j++)
		{
			union_all[i][j].multipurpose = false; // 초기화
		}
	}
	for (int i = 0; i < container.size(); i++)
	{
		if (i == index)
			continue;
		int size = 0;
		if (isoption1 == true)
			size = container[i].hyd.range.size();
		else
			size = container[i].hyd.stream.size();
		for (int k = 0; k < size; k++)
		{
			int x, y;
			if (isoption1 == true)
			{
				x = container[i].hyd.range[k].x;
				y = container[i].hyd.range[k].y;
			}
			else
			{
				x = container[i].hyd.stream[k].x;
				y = container[i].hyd.stream[k].y;
			}
			union_all[x][y].multipurpose = true;
		}
	}
	int size = 0;
	if (isoption1 == true)
		size = container[index].hyd.range.size();
	else
		size = container[index].hyd.stream.size();
	for (int i = 0; i < index; i++)
	{
		int x, y;
		if (isoption1 == true)
		{
			x = container[index].hyd.range[i].x;
			y = container[index].hyd.range[i].y;
		}
		else
		{
			x = container[index].hyd.stream[i].x;
			y = container[index].hyd.stream[i].y;
		}
		if (union_all[x][y].multipurpose == true)
			overlapped++;
		else
			occupied++;

	}
	for (int i = 0; i < union_all.size(); i++)
	{
		for (int j = 0; j < union_all[i].size(); j++)
		{
			union_all[i][j].multipurpose = false; // 초기화
		}
	}

	return pair<int, int>(occupied, overlapped);
}
pair<double,double> output::cal_total_overlapped_and_achieved(vector<optptr> &container, bool isoption1)
{
	set<ptr> temp;
	vector<vector<Node>> &union_all = m_grid;
	int sum_of_total_cell = 0;
	int sum_of_union_cell = 0;
	for (int i = 0; i < union_all.size(); i++)
	{
		for (int j = 0; j < union_all[i].size(); j++)
		{
			union_all[i][j].multipurpose = false; // 초기화
		}
	}
	for (int i = 0; i < container.size(); i++)
	{
		int size = 0;
		if (isoption1 == true)
			size = container[i].hyd.range.size();
		else
			size = container[i].hyd.stream.size();
		sum_of_total_cell += size;
		for (int k = 0; k < size; k++)
		{
			int x, y;
			if (isoption1 == true)
			{
				x = container[i].hyd.range[k].x;
				y = container[i].hyd.range[k].y;
			}
			else
			{
				x = container[i].hyd.stream[k].x;
				y = container[i].hyd.stream[k].y;
			}
			if (union_all[x][y].multipurpose == false)
			{
				union_all[x][y].multipurpose = true;
				sum_of_union_cell++;
			}
		}
	}
	for (int i = 0; i < union_all.size(); i++)
	{
		for (int j = 0; j < union_all[i].size(); j++)
		{
			union_all[i][j].multipurpose = false; // 초기화
		}
	}
	int diff = sum_of_total_cell - sum_of_union_cell;
	double overlapped_area = pow(DISTANCE_PER_GRID, 2) * (double)diff;
	double achieved = 0;
	if (isoption1 == true)
		achieved = (double)sum_of_union_cell / m_floodable_point.size();
	else
		achieved = (double)sum_of_union_cell / m_streamable_point.size();
	return pair<double,double>(overlapped_area, achieved);
}

bool output::there_is_overlapped(vector<optptr> &container, int index, bool isoption1)
{
	vector<vector<Node>> &union_all = m_grid;
	for (int i = 0; i < union_all.size(); i++)
	{
		for (int j = 0; j < union_all[i].size(); j++)
		{
			union_all[i][j].multipurpose = false; // 초기화
		}
	}
	for (int i = 0; i < container.size(); i++)
	{
		if (i == index)
			continue;
		int size = 0;
		if (isoption1 == true)
			size = container[i].hyd.range.size();
		else
			size = container[i].hyd.stream.size();
		for (int k = 0; k < size; k++)
		{
			int x, y;
			if (isoption1 == true)
			{
				x = container[i].hyd.range[k].x;
				y = container[i].hyd.range[k].y;
			}
			else
			{
				x = container[i].hyd.stream[k].x;
				y = container[i].hyd.stream[k].y;
			}
			union_all[x][y].multipurpose = true;
		}
	}
	int size = 0;
	if (isoption1 == true)
		size = container[index].hyd.range.size();
	else
		size = container[index].hyd.stream.size();
	for (int i = 0; i < index; i++)
	{
		int x, y;
		if (isoption1 == true)
		{
			x = container[index].hyd.range[i].x;
			y = container[index].hyd.range[i].y;
		}
		else
		{
			x = container[index].hyd.stream[i].x;
			y = container[index].hyd.stream[i].y;
		}
		if (union_all[x][y].multipurpose == true)
			return true;
	}
	return false;

}
pair<double, double> output::cal_single_and_double_overlapped(vector<optptr> &container) // option2 가정
{
	vector<vector<Node>> &union_all = m_grid;
	for (int i = 0; i < union_all.size(); i++)
	{
		for (int j = 0; j < union_all[i].size(); j++)
		{
			union_all[i][j].multipurpose = false; // 초기화
			union_all[i][j].doubleoverlapped = false; // 초기화
		}
	}

	int sum_of_covered = 0;
	int sum_of_double_overlapped = 0;

	for (int i = 0; i < container.size(); i++)
	{
		int size = 0;
		size = container[i].hyd.stream.size();
		for (int k = 0; k < size; k++)
		{
			int x, y;
			x = container[i].hyd.stream[k].x;
			y = container[i].hyd.stream[k].y;
			if (union_all[x][y].multipurpose == false)
			{
				union_all[x][y].multipurpose = true;
				sum_of_covered++;
			}
			else if (union_all[x][y].doubleoverlapped == false)
			{
				union_all[x][y].doubleoverlapped = true;
				sum_of_double_overlapped++;
			}
		}
	}
	for (int i = 0; i < union_all.size(); i++)
	{
		for (int j = 0; j < union_all[i].size(); j++)
		{
			union_all[i][j].multipurpose = false; // 초기화
			union_all[i][j].doubleoverlapped = false; // 초기화
		}
	}


	double covered = (double)sum_of_covered / m_streamable_point.size();
	double double_covered = (double)sum_of_double_overlapped / m_streamable_point.size();
	return pair<double, double>(covered, double_covered);
}
double output::cal_cost(vector<optptr> &src, bool isoption1) // 코스트 산정
{
	/// 1. 소화전 위치와 그 위치의 score
	/// 2. overlap 만족 수, 모든 소화전들이 걸쳐 있는지
	/// 3. overlap 된 범위, overlap만 만족된다면 이는 적을수록 좋음
	/// 4. 소화전 간의 거리? (option 2에서 고려 할만함)
	double obj = 0;
	
	pair<double, double> res = cal_total_overlapped_and_achieved(src, isoption1);
	double total_area_size = 0;
	int size = 0;
	if (isoption1 == true)
	{
		total_area_size = pow(DISTANCE_PER_GRID, 2) * m_floodable_point.size();
		size = res.second * m_floodable_point.size();
	}
	else
	{
		total_area_size = pow(DISTANCE_PER_GRID, 2) * m_streamable_point.size();
		size = res.second * m_streamable_point.size();
	}
		
	//obj = pow(res.first, 2) * 100 / pow(total_area_size, 2) * ( - res.second);
	double b = res.first;
	double c = required_occupy_rate;
	double d = res.second;
	/// minimization 문제 -> 평소엔 o verlap 줄여야 함. 달성률 만족 후엔 overlap 늘려야 함
	obj = (res.first / total_area_size) * (required_occupy_rate - res.second) * 100 - min(required_occupy_rate, res.second) * 5; // 오버랩율과 점유율의 조정
	for (int i = 0; i < src.size(); i++)
	{
	//	obj += 500000 * there_is_overlapped(src, i, isoption1);
	}

	return obj;
}


double output::cal_achieved_percent_with_new_index(vector<optptr> &container, HYDRANT &target, int index, bool isoption1)
{
	set<ptr> temp;
	for (int i = 0; i < container.size(); i++)
	{
		if (i == index)
			continue;
		if (isoption1 == true)
			temp.insert(container[i].hyd.range.begin(), container[i].hyd.range.end());
		else
			temp.insert(container[i].hyd.stream.begin(), container[i].hyd.stream.end());
	}
	if (isoption1 == true)
	{
		temp.insert(target.range.begin(), target.range.end());
		return (double)temp.size() / m_floodable_point.size();
	}
	else
	{
		temp.insert(target.stream.begin(), target.stream.end());
		return (double)temp.size() / m_streamable_point.size();
	}
}




void output::Do_PSO_Option1_Init(double goal_occupancy)
{
	log = fopen("objvalue.txt", "w");
	//cur_iter = 1;
	cur_iter = 0;
	inertia = 1.0;
	damping = 1.0;
	C1 = 2.0;
	C2 = 2.0;
	minvel = 0;
	maxvel = 5 / DISTANCE_PER_GRID; // 한번에 5m까지 이동
	int population = 50;
	cur_index = -1;
	max_iteration = 25;
	max_local_iteration = 1;
	cur_iteration = 0;
	cur_local_iteration = 0;
	m_nPop = population;
	required_occupy_rate = goal_occupancy;
	//double cost = cal_achieved_percent(single_groups, single_groups[0].hyd, 0, true);
	
	double cost = cal_cost(single_groups, true);
	for (int i = 0; i < population; i++)
	{
		cur_groups.push_back(single_groups);
		best_groups.push_back(single_groups);
		global_best = single_groups;
		cur_costs.push_back(cost);
		best_costs.push_back(cost);
		global_cost = cost;
		fprintf(log, "%d %.5lf\n", cur_iteration, global_cost);
	}

}
int output::Do_PSO_Option1_Step() // cur_index
{
	// 총 영역의 n%이상 차지하도록 함.

	if (cur_index < 0)
		cur_index = cur_groups[0].size() - 1;

	int index = cur_index;
	/// 모든 Population에 대하여 수행
	for (int p = 0; p < m_nPop; p++)
	{
		//vector<optptr> newgroup;
		optptr &src = cur_groups[p][cur_index];
		while (1)
		{
			double rand1_u = (double)rand() / RAND_MAX; // 랜덤한 0~1사이 난수형
			double rand2_u = (double)rand() / RAND_MAX;
			float diff1_u = C1*(best_groups[p][index].hyd.x - cur_groups[p][index].hyd.x);
			float diff2_u = C2*(global_best[index].hyd.x - cur_groups[p][index].hyd.x);

			double rand1_v = (double)rand() / RAND_MAX; // 랜덤한 0~1사이 난수형
			double rand2_v = (double)rand() / RAND_MAX;
			float diff1_v = C1*(best_groups[p][index].hyd.y - cur_groups[p][index].hyd.y);
			float diff2_v = C2*(global_best[index].hyd.y - cur_groups[p][index].hyd.y);

			/// 속도 계산
			int one_meter = 1 / DISTANCE_PER_GRID;
			if (one_meter < 2)
				one_meter = 2;
			double vel_u = (inertia * cur_groups[p][index].vel_u)
				+ (rand1_u * diff1_u) + (rand2_u * diff2_u) + (rand() % one_meter - one_meter / 2);
			double vel_v = (inertia * cur_groups[p][index].vel_v)
				+ (rand1_v * diff1_v) + (rand2_v * diff2_v) + (rand() % one_meter - one_meter / 2);
			if (cur_iteration == 0 && cur_local_iteration == 0)
			{
				vel_u = inertia*(rand() % (int)maxvel - maxvel / 2);
				vel_v = inertia*(rand() % (int)maxvel - maxvel / 2);
			}
			
			/// 속도 제한
			if (abs(vel_u) > maxvel)
			{
				vel_u = vel_u / abs(vel_u) * maxvel;
			}
			if (abs(vel_v) > maxvel)
			{
				vel_v = vel_v / abs(vel_v) * maxvel;
			}

			/// 위치 갱신
			int next_x = cur_groups[p][index].hyd.x + vel_u;
			int next_y = cur_groups[p][index].hyd.y + vel_v;

			// 예외처리
			if (next_x < 0)
				next_x = 0;
			else if (next_x > m_grid.size() - 2)
				next_x = m_grid.size() - 2;
			if (next_y < 0)
				next_y = 0;
			else if (next_y > m_grid[0].size() - 2)
				next_y = m_grid[0].size() - 2;

			if (m_grid[next_x][next_y].isfeasible == true)
			{
				Set_Hyd_Parameters_for_Opt(next_x, next_y, Default_Length, Default_Stream_Length, cur_groups[p][index].hyd);
				Update_Particle(cur_groups[p], false);
				break;
			}
			else
				continue;
			/// infeasible영역에서 feasible 영역으로 이동
		}
		double cost = cal_cost(cur_groups[p], true);
		cur_costs[p] = cost;
		if (cur_costs[p] < best_costs[p]) // 베스트와 글로벌 갱신
		{
			best_costs[p] = cur_costs[p];
			best_groups[p] = cur_groups[p]; // 대입
		}
		if (cur_costs[p] < global_cost) // 베스트와 글로벌 갱신
		{
			global_cost = cur_costs[p];
			global_best = cur_groups[p];
			printf("global 갱신 (pop : %d, index : %d 로 부터)\n", p, index);
			printf("obj  : %.2lf\n", global_cost);
			printf("달성률 %.2lf%%\n", cal_total_overlapped_and_achieved(cur_groups[p], true).second * 100);
			m_globalhydpos.clear();
			for (int i = 0; i < global_best.size(); i++)
			{
				m_globalhydpos.push_back(global_best[i].hyd);
				
			}
			m_hydpos = m_globalhydpos;
		}
	}

	

	//inertia *= damping; // 관성을 점점 낮춰감
	cur_local_iteration++;
	//printf("%d th iteration 완료\n", numofiter);
	if (cur_local_iteration >= max_local_iteration)
	{
		cur_local_iteration = 0;
		if (cur_index == 0)
		{
			cur_index--;
			if (cur_iteration >= max_iteration)
			{
				fclose(log);
				return -3; // 완전종료
			}
				
			else
			{
				cur_iteration++;
				fprintf(log, "%d %.5lf\n", cur_iteration, global_cost);
				printf("iteartion : %d\n", cur_iteration);
			}
				
			return -1;
		}
		cur_index--;
		if (cur_index < 0)
			cur_index = cur_groups[0].size() - 1;
		return -1;
	}
	return 0;
}
// 0 : 통상적인 리턴
//-1 : 모든 소화전이 1회 순회에도 업데이트가 없음

void output::Do_Plan2() // option2 전용
{
	single_groups_spared.clear();
	length_array.clear();
	for (int i = 0; i < single_groups.size(); i++)
	{
		single_groups[i].hyd.colorindex = 1;
		single_groups[i].hyd.sourcenum = 1;
		single_groups[i].hyd.type = 1;
	}
	single_groups_spared = single_groups; // 결과 1 복사
	
	/// 여기서부터 infeasible region 설정
	for (int i = 0; i < single_groups.size(); i++)
	{
		int x = single_groups[i].hyd.x;
		int y = single_groups[i].hyd.y;
		HYDRANT hyd = HYDRANT(x, y, 0, RESTRICTED_DISTANCE, 0, 0, 0);
		flood(hyd, 0); // 최대 iteration으로
		for (int k = 0; k < hyd.range.size(); k++)
		{
			int xx = hyd.range[k].x;
			int yy = hyd.range[k].y;
			m_grid[xx][yy].isfeasible = false; // 배치불가능
			m_grid[xx][yy].isinfeasible = true;
		}
	}

	single_groups.clear();
	m_hydpos.clear();
	/// 이러한 전제로 재시작한다.
}


#define WEIGHT_OF_OVERLAP2 (0) // 이전에 처리된 index에 대한 union과의 overlap을 어떤 계수로 처리할 것인지
int output::Do_deterministic_1_step(bool isoption1)
{
	if (cur_index < 0)
	{
		cur_index = single_groups.size() - 1;
	}
	if (single_groups[cur_index].fixed == true)
	{
		local_updated[cur_index] = false;
		cur_index--;
		return 0;
	}

	static bool nochanged = false;

	/// 각 union 요소 초기화
	static vector<vector<Node>> union_all, union_remained, union_remained2; // union_all은 cur_index를 제외한 모든 합집합, union_remained는 0 ~ cur_index - 1까지의 합집합

	
	//	vector<vector<vector<Node>>> union_adjacents;   // union_adjacent는 모든 인접한 각 소화전
	
	
	union_all = m_grid;
	for (int i = 0; i < union_all.size(); i++)
	{
		for (int j = 0; j < union_all[i].size(); j++)
		{
			union_all[i][j].multipurpose = false; // 초기화
			union_all[i][j].checked = false;
		}
	}
	union_remained = union_all;
	union_remained2 = union_all;
//	vector<vector<Node>> emptyset = union_all;
	

	optptr &src = single_groups[cur_index];
	for (int i = 0; i < single_groups.size(); i++)
	{
		if (i == cur_index)				// 모든 union은 cur_index와 관련없음
			continue;
		//vector<vector<Node>> indivisualset = emptyset;

		int size = 0;
		if (isoption1)
		{
			size = single_groups[i].hyd.range.size();
		}
		else
		{
			size = single_groups[i].hyd.stream.size();
		}
		for (int k = 0; k < size; k++)
		{
			int x = 0, y = 0;
			if (isoption1)
			{
				x = single_groups[i].hyd.range[k].x;
				y = single_groups[i].hyd.range[k].y;
			}
			else
			{
				x = single_groups[i].hyd.stream[k].x;
				y = single_groups[i].hyd.stream[k].y;
			} 
			// i == index면 아무것도 수행x, union_remained, union_all은 i<index일떄 수행, union_all만 i>index일 때 수행
			
			if (i > cur_index)
			{
				/*indivisualset[x][y].multipurpose = true;*/
				union_all[x][y].multipurpose = true;
				union_remained2[x][y].multipurpose = true;
			}
			else if (i < cur_index)
			{
	/*			indivisualset[x][y].multipurpose = true;*/
				union_all[x][y].multipurpose = true;
				union_remained[x][y].multipurpose = true;
			}
		}
	/*	bool overlap_with_indivisual = false;*/
		/*
		int srcsize = 0;
		if (isoption1)
		{
			srcsize = src.hyd.range.size();
		}
		else
		{
			srcsize = src.hyd.stream.size();
		}
		*/
		/*
		for (int j = 0; j < srcsize; j++)
		{
			int x = 0, y = 0;
			if (isoption1)
			{
				x = src.hyd.range[j].x;
				y = src.hyd.range[j].y;
			}
			else
			{
				x = src.hyd.stream[j].x;
				y = src.hyd.stream[j].y;
			}
			//if (indivisualset[x][y].multipurpose == true) // index를 제외한 모든 합집합으로부터 차지 안된 공간에 대해
			//{
			//	overlap_with_indivisual = true;
			//	break;
			//}
		}
		*/
		//if (overlap_with_indivisual == true)
		//{
		//	union_adjacents.push_back(indivisualset);
		//}
	}
	/// 여기까지 Union 집합 얻음

	int cur_x = src.hyd.x;
	int cur_y = src.hyd.y;
	int cur_occupied_score = 0;
	int cur_overlap_score = 0;
	int cur_overlap_score2 = 0;
	double length = src.hyd.length;
	double streamlength = src.hyd.streamlength;
	
	/// 이제 index에 대해 초기점수 검증
	int size = 0;
	if (isoption1 == true)
	{
		size = src.hyd.range.size();
	}
	else
	{
		size = src.hyd.stream.size();
	}
	for (int i = 0; i < size; i++)
	{
		int x = 0, y = 0;
		if (isoption1 == true)
		{
			x = src.hyd.range[i].x;
			y = src.hyd.range[i].y;
		}
		else
		{
			x = src.hyd.stream[i].x;
			y = src.hyd.stream[i].y;
		}
		if (union_all[x][y].multipurpose == false) // index를 제외한 모든 합집합으로부터 차지 안된 공간에 대해
		{
			cur_occupied_score++;
		}
		if (union_remained[x][y].multipurpose == true) // 이미 세부조정 수행한 hydrant를 제외한 나머지 합집합에 대한 overlap 영역 점수에 대해
		{
			cur_overlap_score++;
		}
		if (union_remained2[x][y].multipurpose == true) // 이미 세부조정 수행한 hydrant를 제외한 나머지 합집합에 대한 overlap 영역 점수에 대해
		{
			cur_overlap_score2++;
		}
		
	}

	/*
	/// 만약 모종의 이유로 어느 요소와도 겹치지 않으면
	if (cur_overlap_score == 0)
		return -1;
	/// 재배치에 들어감 ///
	*/
	cur_overlap_score += WEIGHT_OF_OVERLAP2*cur_overlap_score2;
	





	bool updated2 = false;
	bool first = true;
	vector<ptr> ref_range = src.hyd.range;

	union_all[cur_x][cur_y].checked = true;
	while (1) // 더이상 이동이 의미가 없을 때 까지 반복
	{
		int itersize = 0;
		int step = 1;
		ptr target[24]; // 주위 24개좌표 탐색
		target[0] = ptr(cur_x + 1, cur_y);
		target[1] = ptr(cur_x - 1, cur_y);
		target[2] = ptr(cur_x, cur_y + 1);
		target[3] = ptr(cur_x, cur_y - 1);
		target[4] = ptr(cur_x + 1, cur_y + 1);
		target[5] = ptr(cur_x + 1, cur_y - 1);
		target[6] = ptr(cur_x - 1, cur_y + 1);
		target[7] = ptr(cur_x - 1, cur_y - 1);
		target[8] = ptr(cur_x + 2, cur_y);
		target[9] = ptr(cur_x - 2, cur_y);
		target[10] = ptr(cur_x, cur_y - 2);
		target[11] = ptr(cur_x, cur_y + 2);
		target[12] = ptr(cur_x - 2, cur_y - 1);
		target[13] = ptr(cur_x - 2, cur_y + 1);
		target[14] = ptr(cur_x + 2, cur_y - 1);
		target[15] = ptr(cur_x + 2, cur_y + 1);
		target[16] = ptr(cur_x - 1, cur_y + 2);
		target[17] = ptr(cur_x + 1, cur_y + 2);
		target[18] = ptr(cur_x - 1, cur_y - 2);
		target[19] = ptr(cur_x + 1, cur_y - 2);
		target[20] = ptr(cur_x + 3, cur_y);
		target[21] = ptr(cur_x - 3, cur_y);
		target[22] = ptr(cur_x, cur_y + 3);
		target[23] = ptr(cur_x, cur_y - 3);
		int num = 8;
		vector<ptr> container;
	
		if (isoption1 == true)
		{
			itersize = 24;
		}
		else
		{
			
			for (int k = 0; k < num; k++)
			{
				container.push_back(target[k]);
			}
			int rnd = rand() % 10;

			int step = ref_range.size() / (15+rnd); // 15 ~ 24개 추출
			if (step == 0)
				step = 1;
			if (first == true)
			{
				for (int p = 0; p < ref_range.size(); p += step) // 30개 추출
				{
					if (p == 0)
						continue;
					container.push_back(ref_range[p]);
				}
			}
			itersize = container.size();
				
		}
		int max_occupied_score = cur_occupied_score;
		int max_index = -1;
		int max_overlap_score = cur_overlap_score;

			first = false;

		bool updated = false;   /// 하나라도 개선되는 부분이 생기면 true로 전환
		for (int k = 0; k < itersize; k++)
		{
			int x = 0, y = 0;
			if (isoption1 == true)
			{
				x = target[k].x, y = target[k].y;
			}
			else
			{
			//	if (max_index >= 0 && k >= num)
			//		break;
			//	else
					x = container[k].x, y = container[k].y;
			}
//if (cur_index == 0 && k == 21)
//				int a = 3;
			if (union_all[x][y].checked == true)
			{
				continue;
			}
			union_all[x][y].checked = true; // 이미 탐색한 지점 중복 탐색 방지
			
			//if (isoption1 == true)
			if (k <= num)
			{
				if (collision_detection_for_path(ptr(cur_x, cur_y), ptr(x, y), true) == true)
					continue;
			}
			if (m_grid[x][y].isobstacle[m_currentlayer] == true || m_grid[x][y].isinfeasible == true)
				continue;
			int local_occupied_score = 0;
			int local_overlap_score = 0;
			int local_overlap_score2 = 0;

			bool overlapped = false;
			HYDRANT hyd = HYDRANT(x, y, streamlength, length, 0, 0, 0);
			flood(hyd, cur_iter);
			ref_range = hyd.range;
			if (isoption1 == false)
			{
				//stream(hyd, true);
				stream(hyd);
			}

			//vector<bool> checked;
			//checked.assign(union_adjacents.size(), false);

			int hydsize = 0;
			if (isoption1 == true)
			{
				hydsize = hyd.range.size();
			}
			else
			{
				hydsize = hyd.stream.size();
			}
			for (int i = 0; i < hydsize; i++)
			{
				int ax = 0, ay = 0;
				if (isoption1 == true)
				{
					ax = hyd.range[i].x;
					ay = hyd.range[i].y;
				}
				else
				{
					ax = hyd.stream[i].x;
					ay = hyd.stream[i].y;
				}

				if (union_all[ax][ay].multipurpose == false) // index를 제외한 모든 합집합으로부터 차지 안된 공간에 대해
				{
					local_occupied_score++;
				}
				if (union_remained[ax][ay].multipurpose == true) // 이미 세부조정 수행한 hydrant를 제외한 나머지 합집합에 대한 overlap 영역 점수에 대해
				{
					local_overlap_score++;
				}
				if (union_remained2[ax][ay].multipurpose == true) // 이미 세부조정 수행한 hydrant를 제외한 나머지 합집합에 대한 overlap 영역 점수에 대해
				{
					local_overlap_score2++;
				}

				//for (int j = 0; j < union_adjacents.size(); j++)
				//{
				//	if (union_adjacents[j][x][y].multipurpose == true) // 원래 인접했던데와 접하는지 확인
				//	{
				//		checked[j] = true;
				//		break;
				//	}
				//}
			}
			bool nooverlapped = false;
			if (local_overlap_score == 0 && local_overlap_score2 == 0)
				nooverlapped = true;


			local_overlap_score += WEIGHT_OF_OVERLAP2*local_overlap_score2;

			//bool allchecked = true;
			//for (int i = 0; i < checked.size(); i++) // 모두 체크되면 OK
			//{
			//	if (checked[i] == false)
			//		allchecked = false;
			//}
			//overlapped = allchecked;

			if (local_occupied_score < cur_occupied_score) /// 탈락조건
				continue;
			if (local_occupied_score == cur_occupied_score && local_overlap_score < cur_overlap_score) /// 역시 탈락조건
				continue;
			if (isoption1 == true)
			{
				if (nooverlapped == true)
					continue;
			}
 			if (local_occupied_score >= max_occupied_score) /// 1차 선호도
			{
				if (local_occupied_score > max_occupied_score)
					max_overlap_score = -50000; // 오버랩에 대한 조건 초기화
				max_occupied_score = local_occupied_score;
				//max_occupied_index = k;
				
				if (local_overlap_score > max_overlap_score) /// 2차 선호도
				{
					/// 개선 가능한 방향이 존재
					local_updated[cur_index] = { true };
					max_overlap_score = local_overlap_score;
					max_index = k;
					updated = true;
					updated2 = true;
				}
			}
			
		}

		/// 이제 여기서 선호방향 판정
		if (updated == false) // 개선점이 없으면 탈출
		{
			if (updated2 == false)
			{
				local_updated[cur_index] = false; // 이번 인덱스에서 한번도 이동이 없었으면
			}
			break;
		}
		if (isoption1 == true)
		{
			cur_x = target[max_index].x;
			cur_y = target[max_index].y;
		}
		else
		{
			cur_x = container[max_index].x;
			cur_y = container[max_index].y;
		}
		//already_searched.insert(ptr(cur_x, cur_y));
		printf("x : %d, y : %d로 이동\n", cur_x, cur_y);

		cur_occupied_score = max_occupied_score;
		cur_overlap_score = max_overlap_score;
	}
	/*
	if (isoption1 == true)
	{
		src.overlap_rate = (double)cur_occupied_score / src.hyd.range.size();
	}
	else
	{
		src.overlap_rate = (double)cur_occupied_score / src.hyd.stream.size();
	}
	*/
	printf("index : %d\n", cur_index);
	Set_Hyd_Parameters_for_Opt(cur_x, cur_y, length, streamlength, src.hyd);
	
	Update_Particle(single_groups, !isoption1);
	m_hydpos.clear();
	for (int i = 0; i < single_groups.size(); i++)
	{
		m_hydpos.push_back(single_groups[i].hyd);
	}
	for (int i = 0; i < single_groups_spared.size(); i++)
	{
		m_hydpos.push_back(single_groups_spared[i].hyd);
	}


	cur_index--;

	bool flag = false;
	for (int i = 0; i < single_groups.size(); i++)
	{
		flag |= local_updated[i];
	}
	if (flag == false) // 단 하나도 개선이 없었으면
	{
		printf("배치 이동 iteration 수렴\n");
		for (int i = 0; i < single_groups.size(); i++)
		{
			local_updated[i] = true;
		}
		cur_index = -1;
		return -1;
	}
	return 0;
}



int output::Do_deterministic_move_prefer_area_option2()
{
	return 0;
}
int output::Do_deterministic_move_prefer_area(bool isoption1)
{
	static int index = 0;
	while (possible_boundary.size() < single_groups.size())
	{
		possible_boundary.push_back(vector<pair<ptr, double>>());
	}



	if (index >= single_groups.size())
	{
		index = 0;
		if (isoption1 == false)
		{
			prev_single_n_double_coverages = single_n_double_coverages;
			single_n_double_coverages = cal_single_and_double_overlapped(single_groups); // prev와 single이 동일하고 updated가 false라면
			printf("prev single : %.2lf, prev double : %.2lf\n cur single : %.2lf   cur double : %.2lf\n", prev_single_n_double_coverages.first, prev_single_n_double_coverages.second, single_n_double_coverages.first, single_n_double_coverages.second);

			if (prev_single_n_double_coverages.first == single_n_double_coverages.first && prev_single_n_double_coverages.second == single_n_double_coverages.second)
			{
				for (int i = 0; i < 100; i++)
				{
					local_updated[i] = true;
				}
				printf("preference 탐색 수렴\n");
				return -1;
			}
		}
	}

	


	if (single_groups[index].fixed == true)
	{
		local_updated[index] = false;
		index++;
		return 0;
	}

	vector<vector<Node>> union_all = m_grid;
	for (int i = 0; i < union_all.size(); i++)
	{
		for (int j = 0; j < union_all[i].size(); j++)
		{
			union_all[i][j].multipurpose = false; // 초기화
			union_all[i][j].checked = false;
		}
	}
	/*
	int total_size = 0;

	for (int i = 0; i < single_groups.size(); i++)
	{
		if (i == index)				// 모든 union은 cur_index와 관련없음
			continue;
		int size = 0;
		if (isoption1)
		{
			size = single_groups[i].hyd.range.size();
		}
		else
		{
			size = single_groups[i].hyd.stream.size();
		}
		for (int k = 0; k < size; k++)
		{

			int x = 0, y = 0;
			if (isoption1)
			{
				x = single_groups[i].hyd.range[k].x;
				y = single_groups[i].hyd.range[k].y;
			}
			else
			{
				x = single_groups[i].hyd.stream[k].x;
				y = single_groups[i].hyd.stream[k].y;
			}
			// i == index면 아무것도 수행x, union_remained, union_all은 i<index일떄 수행, union_all만 i>index일 때 수행
			if (union_all[x][y].multipurpose == false)
			{
				union_all[x][y].multipurpose = true;
				total_size++;
			}
		}
	}
	*/
	optptr &src = single_groups[index];

	// 지금보다 preference가 높은 포인트들을 얻는다. (weight가 더 낮은)
	double current_weight = union_all[src.hyd.x][src.hyd.y].costweight;
	
	multimap<double, ptr> possibleset; // key는 weight 순서, 값은 그 좌표
	int RESOLUTION_STEP = pow(20.0 / GRIDSIZE, 2);
	if (RESOLUTION_STEP < 1)
		RESOLUTION_STEP = 1;


	for (int i = 0; i < src.hyd.range.size(); i += RESOLUTION_STEP)
	{
		double weight = union_all[src.hyd.range[i].x][src.hyd.range[i].y].costweight;
		if (union_all[src.hyd.range[i].x][src.hyd.range[i].y].isfeasible == true)
		{
			if (weight < current_weight)
			{
				possibleset.insert(pair<double, ptr>(weight, ptr(src.hyd.range[i].x, src.hyd.range[i].y)));
			}
		}
	}

	/// 모든 possibleset에 대해 테스트 수행
	/// 판단기준 : 요구 %를 만족하는가?
	///            weight가 동률이라면, 가능한한 단독 소방 영역이 넓은가?
	///            소방 영역이 동률이라면, 가능한한 overlap이 적은가?
	possible_boundary[index].clear();
	vector<pair<ptr, double>> possible_bnd;
	

	int cur_occupied = 0;
	if (isoption1 == false)
	{
		for (int i = 0; i < src.hyd.stream.size(); i++)
		{
			int x = src.hyd.stream[i].x;
			int y = src.hyd.stream[i].y;
			if (union_all[x][y].multipurpose == false)
				cur_occupied++;
		}
	}

	double min_weight = 100;
	double max_occupied = cur_occupied; 
	double min_overlapped = 0;
	double length = src.hyd.length;
	double streamlength = src.hyd.streamlength;
	double achiverate_of_optimal = 0;
	bool moved = false;
	ptr optimal_ptr;



	
	for (multimap<double, ptr>::iterator it = possibleset.begin(); it != possibleset.end(); it++)
	{
		double overlapped = 0;
		double single_occupied = 0;

		int x1 = it->second.x, y1 = it->second.y;
		double weight = union_all[x1][y1].costweight;

		if (m_grid[x1][y1].isinfeasible == true) // infeasible point는 무시함
			continue;

		if (weight > min_weight)
			break;

		HYDRANT hyd = HYDRANT(x1, y1, streamlength, length, 0, 0, 0);
		flood(hyd, cur_iter);
		if (isoption1 == false)
		{
			stream(hyd, true);
		}
		int hydsize = 0;
		if (isoption1 == true)
		{
			hydsize = hyd.range.size();
		}
		else
		{
			hydsize = hyd.stream.size();
		}
		for (int i = 0; i < hydsize; i++)
		{
			int x = 0, y = 0;
			if (isoption1 == true)
			{
				x = hyd.range[i].x;
				y = hyd.range[i].y;
			}
			else
			{
				x = hyd.stream[i].x;
				y = hyd.stream[i].y;
			}

			if (union_all[x][y].multipurpose == true) // index를 제외한 모든 합집합으로부터 차지 안된 공간에 대해
			{
				overlapped++;
			}
			else
			{
				single_occupied++;
			}
		}
		if (isoption1 == false)
		{
			if (cur_occupied > single_occupied)
				continue;
		}
		double achieve_rate = 0;
//		pair<double, double> res1 = cal_occupied_and_overlapped(single_groups, index, isoption1);
		double res2 = cal_achieved_percent_with_new_index(single_groups, hyd, index, isoption1);
		if (isoption1 == true)
			achieve_rate = res2;
		else
			achieve_rate = res2;
		if (achieve_rate >= required_occupy_rate)
		{
			if (moved == false)
			{
				moved = true;
				min_weight = weight;
				max_occupied = single_occupied;
				min_overlapped = overlapped;
				optimal_ptr = ptr(x1, y1);
				achiverate_of_optimal = achieve_rate;
			}
			else // 처음이 아닐경우 더 좋은 지점이 나타나면 갱신
			{
				if (weight <= min_weight)
				{
					if (weight < min_weight)
					{
						min_weight = weight;
						max_occupied = single_occupied;
						min_overlapped = overlapped;
						optimal_ptr = ptr(x1, y1);
						achiverate_of_optimal = achieve_rate;
					}
					else if (max_occupied <= single_occupied)
					{
						if (max_occupied < single_occupied)
						{
							min_weight = weight;
							max_occupied = single_occupied;
							min_overlapped = overlapped;
							optimal_ptr = ptr(x1, y1);
							achiverate_of_optimal = achieve_rate;
						}
						else if (overlapped < min_overlapped)
						{
							min_weight = weight;
							max_occupied = single_occupied;
							min_overlapped = overlapped;
							optimal_ptr = ptr(x1, y1);
							achiverate_of_optimal = achieve_rate;
						}
					}
				}
			}
			if (isoption1 == false)
			{
				possible_bnd.push_back(pair<ptr, double>(ptr(x1, y1), weight));
			}
		}
	}

	printf("Index : %d\n", index);
	if (moved == true) // 갱신이 발생하면 소화전을 그리로 이동
	{
		int x = optimal_ptr.x;
		int y = optimal_ptr.y;
		printf(" x : %d, y : %d로 이동 \n", x, y);
		if (achiverate_of_optimal > 1)
			achiverate_of_optimal = 1;
		printf(" 달성률 : %.2lf\n", achiverate_of_optimal * 100);
		Set_Hyd_Parameters_for_Opt(x, y, length, streamlength, src.hyd);
		flood(src.hyd, cur_iter); /// 이건 너무 오래걸리니까, iteration 1로 설정
		if (isoption1 == false)
			stream(src.hyd, true);
		
		m_hydpos.clear();
		for (int i = 0; i < single_groups.size(); i++)
		{
			m_hydpos.push_back(single_groups[i].hyd);
		}
		for (int i = 0; i < single_groups_spared.size(); i++)
		{
			m_hydpos.push_back(single_groups_spared[i].hyd);
		}
		local_updated[index] = true;
	}
	else
		local_updated[index] = false;

	possible_boundary[index] = possible_bnd;
	index++;

	

	bool updated = false;
	for (int i = 0; i < single_groups.size(); i++)
	{
		updated |= local_updated[i];
	}
	if (updated == true)
		return 0;
	else
	{
		index = 0;
		for (int i = 0; i < 100; i++)
		{
			local_updated[i] = true;
		}
		return -1;
	}
		
}

int output::Do_deterministic_move_step(bool isoption1)
{
	static deque<int> history;
	static bool added = false;
	bool specialcase = false;
	
	/// 가장 중복율이 높은 index를 찾는다.
	double max_overlap_rate = 0;
	int max_index = -1;
	for (int i = 0; i < single_groups.size(); i++)
	{
		if (single_groups[i].fixed == true) // fixed된건 건드리지 않는다.
			continue; 

		vector<vector<Node>> p_Union = m_grid;
		for (int r = 0; r < p_Union.size(); r++)
		{
			for (int t = 0; t < p_Union[i].size(); t++)
			{
				p_Union[r][t].multipurpose = false; // 초기화
			}
		}
		for (int j = 0; j < single_groups.size(); j++)
		{
			if (i == j)
				continue;
			if (isoption1 == true)
			{
				for (int k = 0; k < single_groups[j].hyd.range.size(); k++)
				{
					int x = single_groups[j].hyd.range[k].x;
					int y = single_groups[j].hyd.range[k].y;
					p_Union[x][y].multipurpose = true;
				}
			}
			else
			{
				for (int k = 0; k < single_groups[j].hyd.stream.size(); k++)
				{
					int x = single_groups[j].hyd.stream[k].x;
					int y = single_groups[j].hyd.stream[k].y;
					p_Union[x][y].multipurpose = true;
				}
			}
		}
		int overlapped = 0;
		if (isoption1 == true)
		{
			for (int k = 0; k < single_groups[i].hyd.range.size(); k++)
			{
				int x = single_groups[i].hyd.range[k].x;
				int y = single_groups[i].hyd.range[k].y;
				if (p_Union[x][y].multipurpose == true)
				{
					overlapped++;
				}
			}
		}
		else
		{
			for (int k = 0; k < single_groups[i].hyd.stream.size(); k++)
			{
				int x = single_groups[i].hyd.stream[k].x;
				int y = single_groups[i].hyd.stream[k].y;
				if (p_Union[x][y].multipurpose == true)
				{
					overlapped++;
				}
			}
		}
		if (isoption1 == true)
			single_groups[i].overlap_rate = (double)overlapped / single_groups[i].hyd.range.size();
		else
			single_groups[i].overlap_rate = (double)overlapped / single_groups[i].hyd.stream.size();
		if (single_groups[i].overlap_rate == 0 && isoption1 == true) // 하나도 오버랩 안되어 있으면
		{
			/// 재배치가 필요한 요소이기 때문에 우선적으로 선택
			max_index = i;
			specialcase = true;
			break;
		}
		if (max_overlap_rate < single_groups[i].overlap_rate && single_groups[i].hyd.length == Default_Length)
		{
			max_index = i;
			max_overlap_rate = single_groups[i].overlap_rate;
		}
	}
	
	if (max_index < 0) // 아무것도 변동될게 없다면
	{
		if (isoption1 == false)
		{
			for (int k = 0; k < single_groups.size(); k++)
			{
				single_n_double_coverages = cal_single_and_double_overlapped(single_groups);
				prev_single_n_double_coverages = pair<double, double>(0, 0);
			}
		}
		return -1; // 
	}


	double length = single_groups[max_index].hyd.length;
	
	set<ptr> total_area;
	double completed = 0;
	if (isoption1 == true)
	{
		for (int i = 0; i < single_groups.size(); i++)
		{
			total_area.insert(single_groups[i].hyd.range.begin(), single_groups[i].hyd.range.end());
		}
		completed = (double)total_area.size() / m_floodable_point.size();
	}
	else
	{
		for (int i = 0; i < single_groups.size(); i++)
		{
			total_area.insert(single_groups[i].hyd.stream.begin(), single_groups[i].hyd.stream.end());
		}
		completed = (double)total_area.size() / m_streamable_point.size();
	}
	printf("제외될 소화전 : %d\n", max_index);
	if (completed > 1)
		completed = 1;
	printf("달성률 : %.2lf%%\n", completed * 100);

	if (specialcase)
	{
		printf("overlap이 없는 소화전이 발견되어 적정 위치로 이동 \n");

		completed = 0;
		vector<optptr>::iterator it = single_groups.begin();
		it += max_index;
		single_groups.erase(it);
	}

	/// 제외하는 부분
	if (completed >= required_occupy_rate - 0.0003) // 요구 퍼센티지보다 높으면 제거 모자르면  추가
	{
		for (int i = 1; i < history.size(); i++)
		{

			if (abs((total_area.size() - history[i])) / (double)total_area.size() < 0.001)
			{

				history.clear();
				printf("수렴 (next step)\n");

				for (int i = 0; i < single_groups.size(); i++)
				{
					printf("x : %d, y : %d \n", single_groups[i].hyd.x, single_groups[i].hyd.y);
					local_updated[i] = true;
				}
				if (isoption1 == false)
				{
					for (int k = 0; k < single_groups.size(); k++)
					{
						single_n_double_coverages = cal_single_and_double_overlapped(single_groups);
						prev_single_n_double_coverages = pair<double, double>(0, 0);
					}
				}
				return -1; // 알고리즘이 종료되었음을 알림
			}

		}
		vector<optptr>::iterator it = single_groups.begin();
		it += max_index;
		single_groups.erase(it);
	}
	/// 추가하는 부분
	else
	{
		/// maxindex를 제외한 union field를 만든다.
		vector<vector<Node>> union_all;
		union_all = m_grid;
		for (int i = 0; i < union_all.size(); i++)
		{
			for (int j = 0; j < union_all[i].size(); j++)
			{
				union_all[i][j].multipurpose = false; // 초기화
			}
		}
		int cur_index = max_index;
		for (int i = 0; i < single_groups.size(); i++)
		{
			//		if (i == cur_index)
			//			continue;
			if (isoption1 == true)
			{
				for (int j = 0; j < single_groups[i].hyd.range.size(); j++)
				{
					int x = single_groups[i].hyd.range[j].x;
					int y = single_groups[i].hyd.range[j].y;
					union_all[x][y].multipurpose = true;
					union_all[x][y].isfeasible = false;
				}
			}
			else
			{
				for (int j = 0; j < single_groups[i].hyd.stream.size(); j++)
				{
					int x = single_groups[i].hyd.stream[j].x;
					int y = single_groups[i].hyd.stream[j].y;
					union_all[x][y].multipurpose = true;
					union_all[x][y].isfeasible = false;
				}
			}
		}


		int max_score = 0;
		int maxx_cod = 0, maxy_cod = 0;
		HYDRANT max_hydrant;
		int RESOLUTION_STEP = 40 / GRIDSIZE;
		if (RESOLUTION_STEP == 0)
			RESOLUTION_STEP = 1;
		if (isoption1 == false)
			RESOLUTION_STEP *= 2;

		bool updated = false;
		thr:
		for (int i = 0; i < union_all.size(); i += RESOLUTION_STEP)
		{
			for (int j = 0; j < union_all[i].size(); j += RESOLUTION_STEP)
			{
				if (union_all[i][j].isfeasible)
				{
					HYDRANT hyd = HYDRANT(i, j, Default_Stream_Length, length, 0, 0, 0);
					flood(hyd, 1); /// 이 작업은 너무 오래걸리므로, iteartion 1로 수행
					if (isoption1 == false) // option2면 stream까지 수행
					{
						stream(hyd);
					}
					int overlapped = 0;
					int singlelapped = 0;
					int size = 0;
					if (isoption1 == true)
						size = hyd.range.size();
					else
						size = hyd.stream.size();
					for (int k = 0; k < size; k++)
					{
						if (isoption1 == true)
						{
							if (union_all[hyd.range[k].x][hyd.range[k].y].multipurpose == true) // 이미 기존과 겹친부분이 있으면
								overlapped++;
							else
								singlelapped++;
						}
						else
						{
							if (union_all[hyd.stream[k].x][hyd.stream[k].y].multipurpose == true) // 이미 기존과 겹친부분이 있으면
								overlapped++;
							else
								singlelapped++;
						}
					}
					if (union_all.empty() == true || overlapped > 0)
					//if (union_all.empty() == true || singlelapped > 0)
					{
						int occupied = 0;
						if (isoption1 == true)
						{
							occupied = 100000 + hyd.range.size() - 2 * overlapped;    // 이중 오버랩 적용
						}
						else
						{
							occupied = 100000 + hyd.stream.size() - 2 * overlapped;    // 이중 오버랩 적용
						}
						if (occupied > max_score)//+ hyd.wallblocked.size() > max_score)
						//if (singlelapped > max_score)//+ hyd.wallblocked.size() > max_score)
						{
							max_score = occupied;// hyd.wallblocked.size();
							//max_score = singlelapped;// hyd.wallblocked.size();
							updated = true;
							maxx_cod = i, maxy_cod = j;
							max_hydrant = hyd;
						}
					}
				}
			}
		}
		if (updated)
		{
			printf("새로운 삽입 위치 \n");
			single_groups.push_back(optptr(maxx_cod, maxy_cod, length, 0, 0, Default_Stream_Length, 0)); /// 추가하는 위치가 맨끝인데, 괜찮은건지 검토
		}
		else
		{
			RESOLUTION_STEP = 1;
			goto thr;
		}
	}

	Update_Particle(single_groups, !isoption1);
	m_hydpos.clear();
	for (int i = 0; i < single_groups.size(); i++)
	{
		m_hydpos.push_back(single_groups[i].hyd);
	}
	for (int i = 0; i < single_groups_spared.size(); i++)
	{
		m_hydpos.push_back(single_groups_spared[i].hyd);
	}
	if (history.empty()) // 값의 변화가 있었을 때만 히스토리에 추가함
	{
		history.push_back(total_area.size()); // 처리 영역 기억
	}
	else
	{
		if (isoption1 == true)
		{
			if (history.back() != total_area.size())
				history.push_back(total_area.size()); // 처리 영역 기억
		}
		else
			history.push_back(total_area.size()); // 처리 영역 기억
	}
#define HISTORY_SIZE 5
	if (history.size() > HISTORY_SIZE) // 최대 기억 수
	{
		history.pop_front();
	}
	return 0; //정상 종료
}


void output::Set_Hyd_Parameters_for_Opt(int x, int y, double length, double streamlength, HYDRANT &target)
{
	if (target.x != x)
	{
		target.x = x;
		target.needtoupdate = 1;
	}
	if (target.y != y)
	{
		target.y = y;
		target.needtoupdate = 1;
	}
	if (target.length != length)
	{
		target.length = length;
		target.needtoupdate = 1;
	}
	if (target.streamlength != streamlength)
	{
		target.streamlength = streamlength;
		target.needtoupdate = 1;
	}
}
void output::Set_Iteration(int num)
{
	cur_iter = num;
	for (int i = 0; i < cur_groups.size(); i++)
	{
		for (int j = 0; j < cur_groups[i].size(); j++)
		{
			cur_groups[i][j].hyd.needtoupdate = 1;
		}
	}
}

void output::Update_Particle(vector<optptr> &src, bool isoption2)
{
	for (int i = 0; i < src.size(); i++)
	{
		if (src[i].hyd.needtoupdate != 0)
		{
			flood(src[i].hyd, cur_iter);
			src[i].hyd.needtoupdate = 0;
			if (isoption2 == true)
			{
				stream(src[i].hyd, true);
			}
		}
	}
}

void output::give_score() // 각 grid셀당 스코어 산정 (거리 의존)
{
	/// path 근처가 좋다
	/// 문 근처는 더 좋다
	/// 소화전끼린 겹치면 안된다 (이는 코스트산정시)

	int dnum = 0;
	vector<vector<Node>> &grid = m_grid;

	for (int i = 0; i < grid.size(); i++)
	{
		for (int j = 0; j < grid[i].size(); j++)
		{
			grid[i][j].costweight = 1;
		}
	}

	float MIN_SCORE = 0.1; // 스코어링을 가할 수 있는 최저한
	float RESOLUTION = 3; // 한번에 몇도를 움직이는가

	
	
	/// path 근처에서 스코어 산정 (낮을수록 좋음)

	
	for (int i = 0; i < m_recommended_point.size(); i++)
	{
	
		int curx = m_recommended_point[i].x;
		int cury = m_recommended_point[i].y;
		int prex = curx-1;
		int prey = cury-1;
		int x = curx;
		int y = cury;
		for (float a = 0; a < 360; a += RESOLUTION) // 15도마다 검색
		{
			float deltax = cos(a / 180 * 3.1415);
			float deltay = sin(a / 180 * 3.1415);
			double cur_score = path_preference;
			int step = 0;
			while (cur_score > MIN_SCORE)
			{
				curx = x + (int)(deltax*step);
				cury = y + (int)(deltay*step);
				cur_score = path_preference * pow(path_damping, sqrt(pow(x - curx, 2) + pow(y - cury, 2))*DISTANCE_PER_GRID);
				if (!(curx == prex && cury == prey)) // 둘이 같지 않으면
				{
					if (grid[curx][cury].isobstacle[m_currentlayer] == true)
						break;

					double score = 1 + cur_score;
					double actual_score = 1.0 / score; // 코스트 산정
					if (grid[curx][cury].costweight > actual_score) // 기존 코스트보다 낮으면
					{
						grid[curx][cury].costweight = actual_score; // 갱신
					}
				}
				step++;
				prex = curx;
				prey = cury;
				
			}
		}
	}


	/// 문 근처에서 score 산정


	for (int i = 0; i < m_doors.size(); i++)
	{
		int curx = m_doors[i].x;
		int cury = m_doors[i].y;
		int prex = curx - 1;
		int prey = cury - 1;
		int x = curx;
		int y = cury;
		for (float a = 0; a < 360; a += RESOLUTION) // 15도마다 검색
		{
			float deltax = cos(a / 180 * 3.1415);
			float deltay = sin(a / 180 * 3.1415);
			double cur_score = door_preference;
			int step = 0;
			while (cur_score > MIN_SCORE)
			{
				curx = x + (int)(deltax*step);
				cury = y + (int)(deltay*step);
				cur_score = door_preference * pow(door_damping, sqrt(pow(x - curx, 2) + pow(y - cury, 2))*DISTANCE_PER_GRID);				if (!(curx == prex && cury == prey)) // 둘이 같지 않으면
				{
					if (grid[curx][cury].isobstacle[m_currentlayer] == true)
						break;
					double score = 1 + cur_score;
					double actual_score = 1.0 / score; // 코스트 산정
					if (grid[curx][cury].costweight > actual_score) // 기존 코스트보다 낮으면
					{
						grid[curx][cury].costweight = actual_score; // 갱신
					}
				}
				step++;
				prex = curx;
				prey = cury;
			}
		}
	}


	/// 스코어링 종료?
}


void output::PSO_by_step() // 글로벌 베스트의 오차가 안생기면 종료?
{
	
}


void output::Set_Scoring_Parameters(double door_pref, double door_damp, double path_pref, double path_damp)
{
	if (door_pref != -1)
	{
		door_preference = door_pref;
	}
	if (door_damp != -1)
	{
		door_damping = door_damp;
	}
	if (path_pref != -1)
	{
		path_preference = path_pref;
	}
	if (path_damp != -1)
	{
		path_damping = path_damp;
	}
}


void output::Update_Feasible_Region()
{
	/// infeasible 판정
	m_feasible_point.clear();

	for (int i = 0; i < m_grid.size(); i++)
	{
		for (int j = 0; j < m_grid[i].size(); j++)
		{
			if (m_grid[i][j].isfloodable)
				m_grid[i][j].isfeasible = true;
			m_grid[i][j].isinfeasible = false;
		}
	}

	//printf("infeasible 지역 판정 중 \n");

	double lengthofrestrictedgrid = Infeasible_range / DISTANCE_PER_GRID; // 몇그리드만큼 infeasibe인가?

	for (int i = 2; i < m_grid.size() - 6; i++)
	{
		for (int j = 2; j < m_grid[i].size() - 6; j++)
		{
			if (m_grid[i][j].isequipment)
			{
				float RESOLUTION = 10.0; // 어느 각도 단위로 검색할건지
				int curx = i;
				int cury = j;
				int prex = curx;
				int prey = cury;
				int x = curx;
				int y = cury;
				for (float a = 0; a < 360; a += RESOLUTION) // 15도마다 검색
				{
					float deltax = cos(a / 180 * 3.1415);
					float deltay = sin(a / 180 * 3.1415);
					float step = 1 / max(abs(deltax), abs(deltay)) + 0.01; // 처음에 흰색 영역으로 가도록
					for (double k = step; k < lengthofrestrictedgrid; k += 0.5)
					{
						curx = x + (int)(deltax*k);
						cury = y + (int)(deltay*k);
						if (!(curx == prex && cury == prey)) // 둘이 같지 않으면
						{
							if (m_grid[curx][cury].isobstacle[m_currentlayer] == true)
								break;
							else
							{
								m_grid[curx][cury].isfeasible = false; // infeasible로 지정
								m_grid[curx][cury].isinfeasible = true;
							}
						}

						prex = curx;
						prey = cury;
					}
				}
			}
		}
	}


	/// feasible 얻기
	for (int x = 0; x < m_grid.size() - 4; x++)
	{
		for (int y = 0; y < m_grid[x].size() - 4; y++)
		{
			if (m_grid[x][y].isfeasible == true)
			{
				m_feasible_point.push_back(node(x, y));
			}
		}
	}
}