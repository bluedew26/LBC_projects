#include "stdafx.h"
#include "hydrant.h"

output::output() // 생성자
{
	/// 색상 정의 영역 ///
	C_STREAM[0] = COLOR(120, 120, 255);
	C_STREAM[1] = COLOR(0, 128, 255);
	C_STREAM[2] = COLOR(255, 255, 128);
	C_STREAM[3] = COLOR(127, 127, 127);
	C_STREAM[4] = COLOR(0, 255, 255);
	C_STREAM[5] = COLOR(255, 0, 0);
	C_STREAM[6] = COLOR(50, 10, 255);
	C_STREAM[7] = COLOR(255, 0, 255);
	C_STREAM[8] = COLOR(255, 100, 100);
	C_ROUTE[0] = COLOR(0, 0, 0);
	C_ROUTE[1] = COLOR(241, 95, 95);
	C_ROUTE[2] = COLOR(242, 150, 97);
	C_ROUTE[3] = COLOR(229, 216, 192);
	C_ROUTE[4] = COLOR(188, 229, 92);
	C_ROUTE[5] = COLOR(92, 209, 229);
	C_ROUTE[6] = COLOR(103, 153, 255);
	C_ROUTE[7] = COLOR(165, 102, 255);
	C_ROUTE[8] = COLOR(243, 97, 166);
	/// 물줄기와 path

	/// 캐드 요소
	C_WALL = COLOR(0, 255, 255);
	C_OBSTACLE[0] = COLOR(0, 255, 0); // walkable area
	C_OBSTACLE[1] = COLOR(250, 0, 0); // path 벽
	C_OBSTACLE[2] = COLOR(0, 50, 100);
	C_EQUIPMENT = COLOR(255, 127, 0); // 장비
	C_SCALE = COLOR(127, 0, 255);  // 축척선
	C_PATH = COLOR(255, 0, 0);


	C_HYDRANT[0] = COLOR(255, 0, 255); // 소화전
	C_HYDRANT[1] = COLOR(12, 12, 12); // 소화전_2 (임의설정)
	C_HYDRANT[2] = COLOR(23, 23, 23); // 소화전_3 (임의설정)
	C_HYDRANT[3] = COLOR(0, 24, 24); // 호스릴
	C_HYDRANT[4] = COLOR(123, 124, 123); // 소화기 (임의설정)
	C_HYDRANT[5] = COLOR(100, 0, 100); // fire monitor

	C_STAIR = COLOR(99, 111, 122);

	/// 임의 정의 요소
	C_GUIDELINE = COLOR(255, 0, 127); // 가이드라인
	C_HIGHLIGHTED_HYDRANT = COLOR(255, 255, 0); // 하이라이트된 소화전
	C_UNACTIVATED_HYDRANT = COLOR(122, 0, 0); // 비활성화된 hydrant

	C_VIRTUAL = COLOR(111, 111, 111);

	C_MONITOR_SYMBOL = COLOR(255, 0, 255);
	C_MONITOR_STREAM = COLOR(0, 0, 255);
	///

	DEFAULT_PATH_LENGTH = 15;    /// 호스 기본길이
	DEFAULT_STREAM_LENGTH = 15;     /// 물줄기 기본길이


	m_highlighted_stair = -1; // 아무것도 하이라이트 안할 떄 -1

	m_currentlayer = 0; // 몇번째 OBSTACLE을 쓸 것인지.
	m_drawingmode = false;

	

	/// 최적화 파라미터 ///
	Infeasible_range = 0.6;
	door_preference = 5;
	door_damping = 0.5;
	path_preference = 3;
	path_damping = 0.5;

	/// 디폴트 값들
	GRIDSIZE = 3;   /// default 그리드 사이즈
	TEMP_GRIDSIZE = 3;
	DISTANCE_PER_PIXEL = 0.5;
	DISTANCE_PER_GRID = DISTANCE_PER_PIXEL * GRIDSIZE;


	m_numofstair = 0;
	for (int j = 0; j < MAX_STAIRS; j++)
	{
		STAIR_PENALTY[j] = 2;
	}
	m_user_loaded = false;
	SLICERATE = 1;			// smoothing 과정중 padding에서 분할레벨
	SCALELENGTH = 100;		// 축척선의 길이 단위

}

void output::Init(int mode)   // 프로그램 초기화. mode 0 = 가시화모드, mode 1 = 그리기모드, mode 2 = 최적화모드
{


	static bool first = true;
	
	if (m_user_loaded == false)
		GRIDSIZE = TEMP_GRIDSIZE;

	printf("<<초기화 중>>\n");
	/// 폰트 정의
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, GRIDSIZE / 4 + 1, GRIDSIZE / 4 + 1, 0, GRIDSIZE / 4 + 1); // 폰트 정의
	///




	/// 파일 로드 ///
	printf("%s 초기화 중\n", m_file);
	m_ents = cvLoadImage(m_file);

	m_program_mode = mode;

	/// 축척선과 DISTANCE_PER_GRID 얻음 ///
	if (m_user_loaded == false)
	{
		printf("축척선 감지 중\n");
		detect_scaleline();
	}
	else
	{
		Set_ScaleLine(m_scaleline.x1, m_scaleline.x2, m_scaleline.y, m_scaleline.length);
	}
	/// 초기 배치된 소화전의 위치를 얻는 부분 ///  처음 로드할떄만 적용
	if (mode != MODE_DRAWING)
	{

	}

	/// 그리드 초기화 ///
	Initialize_grid();
	detect_cad_elements();
	if (mode == MODE_DRAWING)
	{
		swap_grid_to_background();
	}
	else
	{
		detect_closed_loop();
	}
	
	if (mode == MODE_OPTIMIZATION)
	{
		detect_optimization_elements();
		give_score();
	}

	/// 그리드 초기화 ///


	if (first == true && m_user_loaded == false) // 처음 초기화 호출 시
	{
		printf("초기 배치 소화전 얻는 중\n");
		m_hydpos = detect_initial_hydrants();
		sorthyds(); // 소방요소들 정렬(타입별로)
	}
	else
	{
		for (int i = 0; i < m_hydpos.size(); i++) // 소화전 좌표 재구성
		{
			printf("prevx : %d, prevy : %d\n", m_hydpos[i].x, m_hydpos[i].y);
			m_hydpos[i].x = round((m_hydpos[i].x * PRE_GRIDSIZE) / (double)GRIDSIZE);
			m_hydpos[i].y = round((m_hydpos[i].y * PRE_GRIDSIZE) / (double)GRIDSIZE);
			printf("curx : %d, cury : %d\n", m_hydpos[i].x, m_hydpos[i].y);
			m_hydpos[i].reset();
			if (mode == MODE_OPTIMIZATION)
			{
				flood(m_hydpos[i]);
				stream(m_hydpos[i]);
			}
		}
		for (int i = 0; i < single_groups.size(); i++)
		{
			single_groups[i].hyd.x = round((single_groups[i].hyd.x * PRE_GRIDSIZE)  / (double)GRIDSIZE);
			single_groups[i].hyd.y = round((single_groups[i].hyd.y * PRE_GRIDSIZE)  / (double)GRIDSIZE);
			single_groups[i].hyd.reset();
			if (mode == MODE_OPTIMIZATION)
			{
				flood(single_groups[i].hyd);
				stream(single_groups[i].hyd);
			}
		}
	}


	first = false;
	PRE_GRIDSIZE = GRIDSIZE;

	if (m_user_loaded == true)
	{
		//Set_ScaleLine(m_scaleline.x1, m_scaleline.x2, m_scaleline.y, m_scaleline.length);
		sorthyds();
		Update_Drawing();
	}
}

void output::swap_grid_to_background()
{
	m_background_grid = m_grid;
	IplImage *img = m_ents;
	int maxx = m_grid.size();
	int maxy = m_grid[0].size();

	for (int x = 0; x < maxx; x++)
	{
		for (int y = 0; y < maxy; y++)
		{
			if (x == 0 || x == maxx - 4 || y == 0 || y == maxy - 4)
			{
				for (int k = 0; k < NUMOFLAYER; k++)
					m_grid[x][y].isobstacle[k] = true;
				m_grid[x][y].iswall = true;
				m_grid[x][y].ispath = true;
				m_grid[x][y].isequipment = true;
			}
			else
			{
				for (int k = 0; k < NUMOFLAYER; k++)
					m_grid[x][y].isobstacle[k] = false;
				m_grid[x][y].isstair = false;
				m_grid[x][y].iswall = false;
				m_grid[x][y].ispath = false;
				m_grid[x][y].isequipment = false;
			}
		}
	}
}

void output::detect_scaleline()
{
	IplImage *img = m_ents;
	int minscale = 65535;
	int maxscale = 0;
	int stairindex = 0; // 계단 넘버링
	int y = 0;


	for (int i = 0; i < img->width; i++)
	{
		for (int j = 0; j < img->height; j++)
		{
			/// 축척 표시 부분 읽는 구간 (가로선으로 판정)
			if (isscale(img, i, j))
			{
				if (i < minscale)
				{
					minscale = i;
					y = j;
					setcolor(img, i, j, 255, 255, 255);
				}
				if (i > maxscale)
				{
					maxscale = i;
					y = j;
					setcolor(img, i, j, 255, 255, 255);
				}
			}
			/// 축척 표시 부분 읽는 구간
		}

	}
	if (minscale == 65535 && maxscale == 0)
	{
		printf("error : 축척 정보가 없습니다");
	}
	else
	{
		m_scaleline = ScaleLine(minscale, maxscale, y, SCALELENGTH);
		DISTANCE_PER_PIXEL = SCALELENGTH / (maxscale - minscale);
		DISTANCE_PER_GRID = DISTANCE_PER_PIXEL * GRIDSIZE;
	}
}


vector<HYDRANT> output::detect_initial_hydrants() // 초기 배치된 소화전의 위치와 방향값을 얻는 함수
{
	IplImage *img = m_ents;
#define RADIUS_OF_SYMBOL 2.0 // 인식길이 2m
	vector<HYDRANT> container;
	int num[NUMOFTYPE] = { 0 };
	for (int i = 0; i < img->width; i++)
	{
		for (int j = 0; j < img->height; j++)
		{
			bool ishyd = false;
			int type = 0;
			for (int k = 0; k < NUMOFTYPE; k++)
			{
				if (ishydrant(img, i, j, k))
				{
					type = k;
					ishyd = true;
					num[k]++;
					break;
				}
			}
			if (ishyd)
			{
				/// ROI를 정하고 범위내에서 탐색
				int minx = img->width, miny = img->height, maxx = 0, maxy = 0;
				double deltax = 0.0, deltay = 0.0;

				vector<int> dirx, diry;
				int ROI_RANGE = RADIUS_OF_SYMBOL / DISTANCE_PER_PIXEL + 1;


				int xrange_min = i - ROI_RANGE;
				int yrange_min = j - ROI_RANGE;
				int xrange_max = i + ROI_RANGE;
				int yrange_max = j + ROI_RANGE;

				if (xrange_min < 0) xrange_min = 0;
				if (yrange_min < 0) yrange_min = 0;
				if (xrange_max > img->width) xrange_max = img->width;
				if (yrange_max > img->height) yrange_max = img->height;

				for (int x = xrange_min; x < xrange_max; x++)
				{
					for (int y = yrange_min; y < yrange_max; y++)
					{
						bool ishyd2 = false;
						for (int k = 0; k < NUMOFTYPE; k++)
						{
							if (ishydrant(img, x, y, k))
							{
								ishyd2 = true;
								break;
							}
						}
						if (ishyd2)
						{
							minx = min(minx, x);
							miny = min(miny, y);
							maxx = max(maxx, x);
							maxy = max(maxy, y);
							setcolor(img, x, y, 0, 0, 0); // 검정으로 설정
						}
					}
				}

				int centerx, centery;
				centerx = (maxx + minx) / 2;
				centery = (maxy + miny) / 2;
				double maxdist = 0;
				int maxindex = 0;
				for (int i = 0; i<dirx.size(); i++)
				{
					double dist = sqrt(pow(diry[i] - centery, 2) + pow(dirx[i] - centerx, 2));
					if (maxdist < dist)
					{
						maxdist = dist;
						maxindex = i;
					}
				}

				container.push_back(HYDRANT(round((double)centerx / GRIDSIZE), round((double)centery / GRIDSIZE), DEFAULT_STREAM_LENGTH, DEFAULT_PATH_LENGTH, type, 0, container.size() % 9));   // 그리드상에서의 좌표를 리턴
			}
		}
	}
	return container;
}


void output::Initialize_grid() // 맵 생성
{
	printf("그리드 생성중\n");
	printf("GRIDSIZE : %d\n", GRIDSIZE);
	m_grid.clear();

	IplImage *img = m_ents;
	int maxx = img->width;
	int maxy = img->height;

	m_grid.assign(maxx / GRIDSIZE + 3, vector<Node>());
	for (int a = 0; a < m_grid.size(); a++)
	{
		m_grid[a].assign(maxy / GRIDSIZE + 3, Node());
	}

	maxx = m_grid.size();
	maxy = m_grid[0].size();
	for (int x = 0; x < maxx; x++)
	{
		for (int y = 0; y < maxy; y++)
		{
			if (x == 0 || x == maxx - 4 || y == 0 || y == maxy - 4)
			{
				for (int k = 0; k < NUMOFLAYER; k++)
					m_grid[x][y].isobstacle[k] = true;
				m_grid[x][y].iswall = true;

				m_grid[x][y].ispath = true;
				m_grid[x][y].isequipment = true;
			}
			else
			{
				for (int k = 0; k < NUMOFLAYER; k++)
					m_grid[x][y].isobstacle[k] = false;
				m_grid[x][y].isstair = false;
				m_grid[x][y].iswall = false;
				m_grid[x][y].ispath = false;
				m_grid[x][y].isequipment = false;
			}
		}
	}
}

void output::detect_cad_elements()
{
	IplImage *img = m_ents;
	int maxx = img->width;
	int maxy = img->height;


	printf("wall, walkable_area, path 인식 중 \n");
	for (int x = 0; x < maxx; x++)
	{
		for (int y = 0; y < maxy; y++)
		{
			// 요소 인식 //
			int xx = x / GRIDSIZE;
			int yy = y / GRIDSIZE;
			if (iswall(img, x, y))		// 벽이면
			{
				m_grid[xx][yy].iswall = true;
			}
			else if (isstair(img, x, y))							// 계단이면
			{
				m_grid[xx][yy].isstair = true;
			}
			else if (isequipment(img, x, y))
			{
				m_grid[xx][yy].isequipment = true;
			}
			else if (ispath(img, x, y))
			{
				m_grid[xx][yy].ispath = true;
			}
			else
			{
				for (int i = 0; i < NUMOFLAYER; i++)
				{
					if (isobstacle(img, x, y, i))		// 통로벽이면
					{
						m_grid[xx][yy].isobstacle[i] = true;
					}
				}
			}
		}
	}
	/// 요소(벽, 통로벽, 계단, 사다리 등) 인식 부분 //




	int stairindex = 0;
	printf("계단 인식 중\n");
	for (int i = 0; i < maxx / GRIDSIZE; i++)
	{
		for (int j = 0; j < maxy / GRIDSIZE; j++)
		{
			if (m_grid[i][j].isstair == true)
			{
				if (m_grid[i][j].stairindex == -1)
				{
					m_grid[i][j].stairindex = stairindex;
					int x = i;
					int y = j;
					while (1)
					{
						if (m_grid[x + 1][y].isstair == true && m_grid[x + 1][y].stairindex == -1)
						{
							x++;
						}
						else if (m_grid[x - 1][y].isstair == true && m_grid[x - 1][y].stairindex == -1)
						{
							x--;
						}
						else if (m_grid[x][y + 1].isstair == true && m_grid[x][y + 1].stairindex == -1)
						{
							y++;
						}
						else if (m_grid[x][y - 1].isstair == true && m_grid[x][y - 1].stairindex == -1)
						{
							y--;
						}
						else
							break;
						m_grid[x][y].stairindex = stairindex;
					}
					stairindex++;
					m_numofstair++;
				}
			}
		}
	}
	printf("계단 인식 종료 \n");
}

void output::detect_closed_loop()
{
	IplImage *img = m_ents;
	int maxx = img->width;
	int maxy = img->height;


	for (int i = 0; i < NUMOFLAYER; i++)
	{
		m_obstacle_outlines[i].clear();
	}
	m_wall_outlines.clear();
	for (int i = 0; i < m_grid.size() - 4; i++)
	{
		for (int j = 0; j < m_grid[0].size() - 4; j++)
		{
			m_grid[i][j].lossed = 0;
		}
	}



	printf("폐곡선 인식 중 \n");

	/// 연속된 폐곡선 인식
	vector<vector<Node>> grid = m_grid; // 복사본
	vector<vector<Node>> grid2 = m_grid; // 복사본

	for (int i = 2; i < m_grid.size() - 6; i++)
	{
		for (int j = 2; j < m_grid[0].size() - 6; j++)
		{
			for (int k = 0; k < NUMOFLAYER; k++)
			{
				if (grid[i][j].isobstacle[k] == true)
				{
					set<node> adjacents;
					set<node> singularity;
					stack<node> memorized;
					int x = i;
					int y = j;
					int nextx = i;
					int nexty = j;
					int last_dir = 0;
					singularity.insert(node(x, y));
					grid[x][y].isobstacle[k] = false;
					bool once = false;
					while (1)
					{
						x = nextx;
						y = nexty;


						int numofdirection = 0;

						bool WESN = false;
						bool continued = false;

						// 시계방향으로 탐색

						if (grid[x + 1][y].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;
						if (grid[x + 1][y - 1].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;
						if (grid[x][y - 1].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;
						if (grid[x - 1][y - 1].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;

						if (grid[x - 1][y].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;
						if (grid[x - 1][y + 1].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;

						if (grid[x][y + 1].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;
						if (grid[x + 1][y + 1].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;

						// 반시계
						if (grid[x + 1][y].isobstacle[k] && grid[x + 1][y + 1].isobstacle[k])
							numofdirection--;



						if (grid[x + 1][y].isobstacle[k])
						{
							WESN = true; last_dir = 0;
							nextx = x + 1; nexty = y;
						}
						if (grid[x - 1][y].isobstacle[k])
						{
							WESN = true; last_dir = 4;
							nextx = x - 1; nexty = y;
						}
						if (grid[x][y + 1].isobstacle[k])
						{
							WESN = true; last_dir = 6;
							nextx = x; nexty = y + 1;
						}
						if (grid[x][y - 1].isobstacle[k])
						{
							WESN = true; last_dir = 2;
							nextx = x; nexty = y - 1;
						}
						if (WESN == false)
						{
							if (grid[x + 1][y - 1].isobstacle[k])
							{
								nextx = x + 1; nexty = y - 1;
								last_dir = 1;
							}
							if (grid[x - 1][y - 1].isobstacle[k])
							{
								nextx = x - 1; nexty = y - 1;
								last_dir = 3;
							}
							if (grid[x + 1][y + 1].isobstacle[k])
							{
								nextx = x + 1; nexty = y + 1;
								last_dir = 7;
							}
							if (grid[x - 1][y + 1].isobstacle[k])
							{
								nextx = x - 1; nexty = y + 1;
								last_dir = 5;
							}
						}


						node n = node(x, y);
						n.last_dir = last_dir;
						adjacents.insert(n);
						grid[x][y].isobstacle[k] = false;


						if (numofdirection == 0) // 길이 없으면
						{
							if ((last_dir == 4 && grid2[x - 1][y].isobstacle[k] == false && grid2[x - 1][y + 1].isobstacle[k] == false && grid2[x - 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false)
								|| (last_dir == 5 && grid2[x - 1][y + 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x - 1][y - 1].isobstacle[k] == false)
								|| (last_dir == 6 && grid2[x - 1][y + 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y].isobstacle[k] == false) // 좌우
								|| (last_dir == 7 && grid2[x + 1][y].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x - 1][y + 1].isobstacle[k] == false)
								|| (last_dir == 0 && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x + 1][y].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false)
								|| (last_dir == 1 && grid2[x + 1][y].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x - 1][y - 1].isobstacle[k] == false)
								|| (last_dir == 2 && grid2[x - 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y].isobstacle[k] == false)
								|| (last_dir == 3 && grid2[x - 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x - 1][y + 1].isobstacle[k] == false))

							{
								grid2[x][y].isobstacle[k] = false;
							}
							//grid2[x][y].isstair = true;
							if (memorized.empty()) // 그리고 한번 돌아서 온건지, 정말 끊긴건지
							{
								break;
							}
							else
							{
								node n = memorized.top();
								nextx = n.x;
								nexty = n.y;
								memorized.pop();
							}
						}
						else if (numofdirection == 2)
						{
							memorized.push(node(x, y));
							singularity.insert(node(x, y));
						}

					}
				}
			}
			if (grid[i][j].iswall == true)
			{
				set<node> adjacents;
				set<node> singularity;
				stack<node> memorized;
				int x = i;
				int y = j;
				int nextx = i;
				int nexty = j;
				int last_dir = 0;
				singularity.insert(node(x, y));
				grid[x][y].iswall = false;
				bool once = false;
				while (1)
				{
					x = nextx;
					y = nexty;


					int numofdirection = 0;

					bool WESN = false;
					bool continued = false;

					// 시계방향으로 탐색

					if (grid[x + 1][y].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;
					if (grid[x + 1][y - 1].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;
					if (grid[x][y - 1].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;
					if (grid[x - 1][y - 1].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;

					if (grid[x - 1][y].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;
					if (grid[x - 1][y + 1].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;

					if (grid[x][y + 1].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;
					if (grid[x + 1][y + 1].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;

					// 반시계
					if (grid[x + 1][y].iswall && grid[x + 1][y + 1].iswall)
						numofdirection--;



					if (grid[x + 1][y].iswall)
					{
						WESN = true; last_dir = 0;
						nextx = x + 1; nexty = y;
					}
					if (grid[x - 1][y].iswall)
					{
						WESN = true; last_dir = 4;
						nextx = x - 1; nexty = y;
					}
					if (grid[x][y + 1].iswall)
					{
						WESN = true; last_dir = 6;
						nextx = x; nexty = y + 1;
					}
					if (grid[x][y - 1].iswall)
					{
						WESN = true; last_dir = 2;
						nextx = x; nexty = y - 1;
					}
					if (WESN == false)
					{
						if (grid[x + 1][y - 1].iswall)
						{
							nextx = x + 1; nexty = y - 1;
							last_dir = 1;
						}
						if (grid[x - 1][y - 1].iswall)
						{
							nextx = x - 1; nexty = y - 1;
							last_dir = 3;
						}
						if (grid[x + 1][y + 1].iswall)
						{
							nextx = x + 1; nexty = y + 1;
							last_dir = 7;
						}
						if (grid[x - 1][y + 1].iswall)
						{
							nextx = x - 1; nexty = y + 1;
							last_dir = 5;
						}
					}


					node n = node(x, y);
					n.last_dir = last_dir;
					adjacents.insert(n);
					grid[x][y].iswall = false;

					if (numofdirection == 0) // 길이 없으면
					{
						if ((last_dir == 4 && grid2[x - 1][y].iswall == false && grid2[x - 1][y + 1].iswall == false && grid2[x - 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x][y + 1].iswall == false)
							|| (last_dir == 5 && grid2[x - 1][y + 1].iswall == false && grid2[x][y + 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x - 1][y - 1].iswall == false)
							|| (last_dir == 6 && grid2[x - 1][y + 1].iswall == false && grid2[x][y + 1].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y].iswall == false) // 좌우
							|| (last_dir == 7 && grid2[x + 1][y].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x][y + 1].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x - 1][y + 1].iswall == false)
							|| (last_dir == 0 && grid2[x + 1][y + 1].iswall == false && grid2[x + 1][y].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x][y + 1].iswall == false)
							|| (last_dir == 1 && grid2[x + 1][y].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x - 1][y - 1].iswall == false)
							|| (last_dir == 2 && grid2[x - 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y].iswall == false)
							|| (last_dir == 3 && grid2[x - 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x - 1][y + 1].iswall == false))

						{
							grid2[x][y].iswall = false;
						}
						//grid2[x][y].isstair = true;
						if (memorized.empty()) // 그리고 한번 돌아서 온건지, 정말 끊긴건지
						{
							break;
						}
						else
						{
							node n = memorized.top();
							nextx = n.x;
							nexty = n.y;
							memorized.pop();
						}
					}
					else if (numofdirection == 2)
					{
						memorized.push(node(x, y));
						singularity.insert(node(x, y));
					}

				}
			}
		}
	}

	/// 연속된 폐곡선 인식
	grid = grid2; // 복사본

	for (int i = 2; i < m_grid.size() - 6; i++)
	{
		for (int j = 2; j < m_grid[0].size() - 6; j++)
		{

			for (int k = 0; k < NUMOFLAYER; k++)
			{
				//	grid2[i][j].isobstacle[k] = false;
				if (grid[i][j].isobstacle[k] == true)
				{
					set<node> adjacents;
					set<node> singularity;
					stack<node> memorized;
					int x = i;
					int y = j;
					int nextx = i;
					int nexty = j;
					int last_dir = 0;
					singularity.insert(node(x, y));
					grid[x][y].isobstacle[k] = false;
					bool once = false;
					while (1)
					{
						x = nextx;
						y = nexty;


						int numofdirection = 0;

						bool WESN = false;
						bool continued = false;

						// 시계방향으로 탐색

						if (grid[x + 1][y].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;
						if (grid[x + 1][y - 1].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;
						if (grid[x][y - 1].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;
						if (grid[x - 1][y - 1].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;

						if (grid[x - 1][y].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;
						if (grid[x - 1][y + 1].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;

						if (grid[x][y + 1].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;
						if (grid[x + 1][y + 1].isobstacle[k])
						{
							if (continued == false)
							{
								numofdirection++;
								continued = true;
							}
						}
						else
							continued = false;

						// 반시계
						if (grid[x + 1][y].isobstacle[k] && grid[x + 1][y + 1].isobstacle[k])
							numofdirection--;



						if (grid[x + 1][y].isobstacle[k])
						{
							WESN = true; last_dir = 0;
							nextx = x + 1; nexty = y;
						}
						if (grid[x - 1][y].isobstacle[k])
						{
							WESN = true; last_dir = 4;
							nextx = x - 1; nexty = y;
						}
						if (grid[x][y + 1].isobstacle[k])
						{
							WESN = true; last_dir = 6;
							nextx = x; nexty = y + 1;
						}
						if (grid[x][y - 1].isobstacle[k])
						{
							WESN = true; last_dir = 2;
							nextx = x; nexty = y - 1;
						}
						if (WESN == false)
						{
							if (grid[x + 1][y - 1].isobstacle[k])
							{
								nextx = x + 1; nexty = y - 1;
								last_dir = 1;
							}
							if (grid[x - 1][y - 1].isobstacle[k])
							{
								nextx = x - 1; nexty = y - 1;
								last_dir = 3;
							}
							if (grid[x + 1][y + 1].isobstacle[k])
							{
								nextx = x + 1; nexty = y + 1;
								last_dir = 7;
							}
							if (grid[x - 1][y + 1].isobstacle[k])
							{
								nextx = x - 1; nexty = y + 1;
								last_dir = 5;
							}
						}


						node n = node(x, y);
						n.last_dir = last_dir;
						adjacents.insert(n);
						grid[x][y].isobstacle[k] = false;

						if (numofdirection == 0) // 길이 없으면
						{
							if ((last_dir == 4 && grid2[x - 1][y].isobstacle[k] == false && grid2[x - 1][y + 1].isobstacle[k] == false && grid2[x - 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false)
								|| (last_dir == 5 && grid2[x - 1][y + 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x - 1][y - 1].isobstacle[k] == false)
								|| (last_dir == 6 && grid2[x - 1][y + 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y].isobstacle[k] == false) // 좌우
								|| (last_dir == 7 && grid2[x + 1][y].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x - 1][y + 1].isobstacle[k] == false)
								|| (last_dir == 0 && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x + 1][y].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false)
								|| (last_dir == 1 && grid2[x + 1][y].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x - 1][y - 1].isobstacle[k] == false)
								|| (last_dir == 2 && grid2[x - 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y].isobstacle[k] == false)
								|| (last_dir == 3 && grid2[x - 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x - 1][y + 1].isobstacle[k] == false))

							{
								m_grid[x][y].lossed = true;             // 로스된 부분임을 알림
							}
							//grid2[x][y].isstair = true;
							if (memorized.empty()) // 그리고 한번 돌아서 온건지, 정말 끊긴건지
							{
								break;
							}
							else
							{
								node n = memorized.top();
								nextx = n.x;
								nexty = n.y;
								memorized.pop();
							}
						}
						else if (numofdirection == 2)
						{
							memorized.push(node(x, y));
							singularity.insert(node(x, y));
						}

					}
					m_obstacle_outlines[k].push_back(adjacents);
				}
			}
			if (grid[i][j].iswall == true)
			{
				set<node> adjacents;
				set<node> singularity;
				stack<node> memorized;
				int x = i;
				int y = j;
				int nextx = i;
				int nexty = j;
				int last_dir = 0;
				singularity.insert(node(x, y));
				grid[x][y].iswall = false;
				bool once = false;
				while (1)
				{
					x = nextx;
					y = nexty;


					int numofdirection = 0;

					bool WESN = false;
					bool continued = false;

					// 시계방향으로 탐색

					if (grid[x + 1][y].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;
					if (grid[x + 1][y - 1].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;
					if (grid[x][y - 1].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;
					if (grid[x - 1][y - 1].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;

					if (grid[x - 1][y].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;
					if (grid[x - 1][y + 1].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;

					if (grid[x][y + 1].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;
					if (grid[x + 1][y + 1].iswall)
					{
						if (continued == false)
						{
							numofdirection++;
							continued = true;
						}
					}
					else
						continued = false;

					// 반시계
					if (grid[x + 1][y].iswall && grid[x + 1][y + 1].iswall)
						numofdirection--;



					if (grid[x + 1][y].iswall)
					{
						WESN = true; last_dir = 0;
						nextx = x + 1; nexty = y;
					}
					if (grid[x - 1][y].iswall)
					{
						WESN = true; last_dir = 4;
						nextx = x - 1; nexty = y;
					}
					if (grid[x][y + 1].iswall)
					{
						WESN = true; last_dir = 6;
						nextx = x; nexty = y + 1;
					}
					if (grid[x][y - 1].iswall)
					{
						WESN = true; last_dir = 2;
						nextx = x; nexty = y - 1;
					}
					if (WESN == false)
					{
						if (grid[x + 1][y - 1].iswall)
						{
							nextx = x + 1; nexty = y - 1;
							last_dir = 1;
						}
						if (grid[x - 1][y - 1].iswall)
						{
							nextx = x - 1; nexty = y - 1;
							last_dir = 3;
						}
						if (grid[x + 1][y + 1].iswall)
						{
							nextx = x + 1; nexty = y + 1;
							last_dir = 7;
						}
						if (grid[x - 1][y + 1].iswall)
						{
							nextx = x - 1; nexty = y + 1;
							last_dir = 5;
						}
					}


					node n = node(x, y);
					n.last_dir = last_dir;
					adjacents.insert(n);
					grid[x][y].iswall = false;



					if (numofdirection == 0) // 길이 없으면
					{
						if ((last_dir == 4 && grid2[x - 1][y].iswall == false && grid2[x - 1][y + 1].iswall == false && grid2[x - 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x][y + 1].iswall == false)
							|| (last_dir == 5 && grid2[x - 1][y + 1].iswall == false && grid2[x][y + 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x - 1][y - 1].iswall == false)
							|| (last_dir == 6 && grid2[x - 1][y + 1].iswall == false && grid2[x][y + 1].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y].iswall == false) // 좌우
							|| (last_dir == 7 && grid2[x + 1][y].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x][y + 1].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x - 1][y + 1].iswall == false)
							|| (last_dir == 0 && grid2[x + 1][y + 1].iswall == false && grid2[x + 1][y].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x][y + 1].iswall == false)
							|| (last_dir == 1 && grid2[x + 1][y].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x - 1][y - 1].iswall == false)
							|| (last_dir == 2 && grid2[x - 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y].iswall == false)
							|| (last_dir == 3 && grid2[x - 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x - 1][y + 1].iswall == false))

						{
							m_grid[x][y].lossed = true;
						}
						if (memorized.empty()) // 그리고 한번 돌아서 온건지, 정말 끊긴건지
						{
							break;
						}
						else
						{
							node n = memorized.top();
							nextx = n.x;
							nexty = n.y;
							memorized.pop();
						}
					}
					else if (numofdirection == 2)
					{
						memorized.push(node(x, y));
						singularity.insert(node(x, y));
					}

				}
				m_wall_outlines.push_back(adjacents);
			}
		}
	}
}




void output::detect_optimization_elements()
{
	m_floodable_point.clear();
	m_feasible_point.clear();
	m_recommended_point.clear();
	m_ActStack.clear();
	m_doors.clear();
	m_streamable_point.clear();
	m_wall_outlines.clear();
	for (int i = 0; i < NUMOFLAYER; i++)
		m_obstacle_outlines[i].clear();

	printf("OUT_OF_ROI 인식 중 \n");
	/// 각종 범위 인식
	int width = m_grid.size();
	int height = m_grid[0].size();

	pair<int, int> start = pair<int, int>(2, 2); // flood-fill 시작지점
	/// flood-fill 수행
	stack<pair<int, int>> ff;
	ff.push(start);

	/// wall에 대해
	while (1)
	{
		if (ff.empty() == true)
			break;
		pair<int, int> cur = ff.top();
		m_grid[cur.first][cur.second].isoutofwall = true;
		ff.pop();
		pair<int, int> target[4];
		target[0] = pair<int, int>(cur.first + 1, cur.second);
		target[1] = pair<int, int>(cur.first - 1, cur.second);
		target[2] = pair<int, int>(cur.first, cur.second + 1);
		target[3] = pair<int, int>(cur.first, cur.second - 1);
		for (int i = 0; i < 4; i++)
		{
			if (m_grid[target[i].first][target[i].second].iswall == false && m_grid[target[i].first][target[i].second].isoutofwall == false)
			{
				ff.push(target[i]);
			}
		}
	}
	/// walkable distance에 대해
	ff.push(start);
	while (1)
	{
		if (ff.empty() == true)
			break;
		pair<int, int> cur = ff.top();
		m_grid[cur.first][cur.second].isoutofwd = true;
		ff.pop();
		pair<int, int> target[4];
		target[0] = pair<int, int>(cur.first + 1, cur.second);
		target[1] = pair<int, int>(cur.first - 1, cur.second);
		target[2] = pair<int, int>(cur.first, cur.second + 1);
		target[3] = pair<int, int>(cur.first, cur.second - 1);
		for (int i = 0; i < 4; i++)
		{
			if (m_grid[target[i].first][target[i].second].isobstacle[m_currentlayer] == false && m_grid[target[i].first][target[i].second].isoutofwd == false)
			{
				ff.push(target[i]);
			}
		}
	}
	/// path에 대해
	ff.push(start);
	while (1)
	{
		if (ff.empty() == true)
			break;
		pair<int, int> cur = ff.top();
		m_grid[cur.first][cur.second].isoutofpath = true;
		ff.pop();
		pair<int, int> target[4];
		target[0] = pair<int, int>(cur.first + 1, cur.second);
		target[1] = pair<int, int>(cur.first - 1, cur.second);
		target[2] = pair<int, int>(cur.first, cur.second + 1);
		target[3] = pair<int, int>(cur.first, cur.second - 1);
		for (int i = 0; i < 4; i++)
		{
			if (m_grid[target[i].first][target[i].second].ispath == false && m_grid[target[i].first][target[i].second].isoutofpath == false)
			{
				ff.push(target[i]);
			}
		}
	}


	printf("streamable, floodable, recommended point 인식 중\n");
	bool finished_detecting_wd = false; // 한번 플래그 서면 더이상 wd에 대해 flood-fill 안함
	/// 실제 인식에 사용돼는 flood-fill
	for (int x = 0; x < width - 4; x++)
	{
		for (int y = 0; y < height - 4; y++)
		{
			Node point = m_grid[x][y];
			/// steramable flood-fill 수행

			if (point.iswall == false && point.isoutofwall == false && point.istempstreamable == false)
			{
				bool there_can_be_moved = false; // 사람이 이동할 수 있는 블록인지

				pair<int, int> start = pair<int, int>(x, y);
				stack<pair<int, int>> ff;
				ff.push(start);
				set<node> tempset;
				while (1)
				{
					if (ff.empty() == true)
					{
						if (there_can_be_moved == true) // 도달 가능한 블록이면
						{
							m_streamable_point.insert(m_streamable_point.end(), tempset.begin(), tempset.end());
							for (set<node>::iterator it = tempset.begin(); it != tempset.end(); it++)
							{
								m_grid[it->x][it->y].isstreamable = true;
							}
						}
						break;
					}

					pair<int, int> cur = ff.top();
					int xx = cur.first;
					int yy = cur.second;
					if (m_grid[xx][yy].ispath == true || m_grid[xx][yy].isobstacle[m_currentlayer] == true || m_grid[xx][yy].isequipment == true)
					{
						there_can_be_moved = true;
					}

					m_grid[xx][yy].istempstreamable = true; ///
					tempset.insert(node(xx, yy));  ///
					ff.pop();
					pair<int, int> target[4];
					target[0] = pair<int, int>(xx + 1, yy);
					target[1] = pair<int, int>(xx - 1, yy);
					target[2] = pair<int, int>(xx, yy + 1);
					target[3] = pair<int, int>(xx, yy - 1);
					for (int i = 0; i < 4; i++)
					{
						if (m_grid[target[i].first][target[i].second].iswall == false && m_grid[target[i].first][target[i].second].istempstreamable == false)
						{ ///
							ff.push(target[i]);
						}
					}
				}
			}


			if (point.isobstacle[m_currentlayer] == false && point.isoutofwd == false && point.isfloodable == false && finished_detecting_wd == false && point.isequipment == false) // walkable distance는 한 번만 채우면 된다
			{
				pair<int, int> start = pair<int, int>(x, y);
				stack<pair<int, int>> ff;
				ff.push(start);
				set<node> tempset;
				while (1)
				{
					if (ff.empty() == true)
					{
						finished_detecting_wd = true;
						break;
					}

					pair<int, int> cur = ff.top();
					int xx = cur.first;
					int yy = cur.second;
					m_grid[xx][yy].isfloodable = true;
					m_grid[xx][yy].isfeasible = true;
					m_floodable_point.push_back(node(xx, yy));
					ff.pop();
					pair<int, int> target[4];
					target[0] = pair<int, int>(xx + 1, yy);
					target[1] = pair<int, int>(xx - 1, yy);
					target[2] = pair<int, int>(xx, yy + 1);
					target[3] = pair<int, int>(xx, yy - 1);
					for (int i = 0; i < 4; i++)
					{
						if (m_grid[target[i].first][target[i].second].isobstacle[m_currentlayer] == false && m_grid[target[i].first][target[i].second].isequipment == false && m_grid[target[i].first][target[i].second].isfloodable == false)
						{ ///
							ff.push(target[i]);
						}
					}
				}
			}
			if (point.ispath == false && point.isoutofpath == false && point.istemprecommended == false)
			{
				pair<int, int> start = pair<int, int>(x, y);
				stack<pair<int, int>> ff;
				ff.push(start);
				set<node> tempset;
				while (1)
				{
					if (ff.empty() == true)
					{
						break;
					}

					pair<int, int> cur = ff.top();
					int xx = cur.first;
					int yy = cur.second;
					m_grid[cur.first][cur.second].isrecommended = true; ///
					m_recommended_point.push_back(node(xx, yy));
					ff.pop();
					pair<int, int> target[4];
					target[0] = pair<int, int>(xx + 1, yy);
					target[1] = pair<int, int>(xx - 1, yy);
					target[2] = pair<int, int>(xx, yy + 1);
					target[3] = pair<int, int>(xx, yy - 1);
					for (int i = 0; i < 4; i++)
					{
						if (m_grid[target[i].first][target[i].second].ispath == false && m_grid[target[i].first][target[i].second].isrecommended == false)
						{ ///
							ff.push(target[i]);
						}
					}
				}
			}

		}
	}
	

	/// infeasible 판정
	printf("infeasible 지역 판정 중 \n");

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
	for (int x = 0; x < width - 4; x++)
	{
		for (int y = 0; y < height - 4; y++)
		{
			if (m_grid[x][y].isfeasible == true)
			{
				m_feasible_point.push_back(node(x, y));
			}
		}
	}


	/// 문 인식 ///
	printf("door 인식 중 \n");
	vector<vector<node>> candidates;
	for (int x = 0; x < width - 4; x++)
	{
		for (int y = 0; y < height - 4; y++)
		{
			if (m_grid[x][y].iswall && m_grid[x][y].isfloodable && m_grid[x][y].checked == false) // 벽인데 walkable area에 있는 경우
			{
				vector<node> candid;
				stack<node> st;
				node cur = node(x, y);
				st.push(cur);
				candid.push_back(cur);
				while (1)
				{
					if (st.empty() == 0)
						break;
					cur = st.top();
					st.pop();
					node target[8];
					target[0] = node(cur.x + 1, cur.y);
					target[1] = node(cur.x - 1, cur.y);
					target[2] = node(cur.x, cur.y + 1);
					target[3] = node(cur.x, cur.y - 1);
					target[4] = node(cur.x + 1, cur.y + 1);
					target[5] = node(cur.x + 1, cur.y - 1);
					target[6] = node(cur.x - 1, cur.y + 1);
					target[7] = node(cur.x - 1, cur.y - 1);
					for (int i = 0; i < 8; i++)
					{
						if (m_grid[x][y].iswall && m_grid[x][y].isfloodable)
						{
							st.push(target[i]);
							candid.push_back(target[i]);
						}
					}
				}
				candidates.push_back(candid);
			}
		}
	}

	for (int i = 0; i < candidates.size(); i++)
	{
		if (candidates[i].size() * DISTANCE_PER_GRID < THRESHOLD_OF_DOOR)
		{
			for (int j = 0; j < candidates[i].size(); j++)
			{
				int xx = candidates[i][j].x;
				int yy = candidates[i][j].y;

				m_grid[xx][yy].isdoor = true;
				m_doors.push_back(node(xx, yy));
			}
		}
	}
	/// 중복제거 

	sort(m_floodable_point.begin(), m_floodable_point.end());
	vector<node>::iterator it1;
	it1 = unique(m_floodable_point.begin(), m_floodable_point.end());
	m_floodable_point.erase(it1, m_floodable_point.end());
	
	//vector<node>::iterator itt = m_streamable_point.end() - 1;
	//m_streamable_point.insert(m_streamable_point.end(), m_floodable_point.begin(), m_floodable_point.end());
	//for (vector<node>::iterator itt2 = m_streamable_point.end() - 2; itt2 != m_streamable_point.begin(); itt2--)
	//{
		//int x = (itt2 + 1)->x, y = (itt2 + 1)->y;
		//if (m_grid[x][y].iswall == true)
//			m_streamable_point.erase(itt2 + 1);
//	}

	sort(m_streamable_point.begin(), m_streamable_point.end());
	vector<node>::iterator it2;
	it2 = unique(m_streamable_point.begin(), m_streamable_point.end());
	m_streamable_point.erase(it2, m_streamable_point.end());
	
	sort(m_feasible_point.begin(), m_feasible_point.end());
	vector<node>::iterator it3;
	it3 = unique(m_feasible_point.begin(), m_feasible_point.end());
	m_feasible_point.erase(it3, m_feasible_point.end());
	
	for (int i = 1; i < m_grid.size() - 4; i++)
	{
		for (int j = 1; j < m_grid[i].size() - 4; j++)
		{
			if (m_grid[i][j].ispath == true)
				m_recommended_point.push_back(node(i, j));
		}
	}
	sort(m_recommended_point.begin(), m_recommended_point.end());
	vector<node>::iterator it4;
	it4 = unique(m_recommended_point.begin(), m_recommended_point.end());
	m_recommended_point.erase(it4, m_recommended_point.end());



	printf("floodable points:%d\n", m_floodable_point.size());
	printf("streamable points:%d\n", m_streamable_point.size());
	printf("recommended points:%d\n", m_recommended_point.size());
}


