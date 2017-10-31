#include "stdafx.h"
#include "hydrant.h"

output::output() // ������
{
	/// ���� ���� ���� ///
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
	/// ���ٱ�� path

	/// ĳ�� ���
	C_WALL = COLOR(0, 255, 255);
	C_OBSTACLE[0] = COLOR(0, 255, 0); // walkable area
	C_OBSTACLE[1] = COLOR(250, 0, 0); // path ��
	C_OBSTACLE[2] = COLOR(0, 50, 100);
	C_EQUIPMENT = COLOR(255, 127, 0); // ���
	C_SCALE = COLOR(127, 0, 255);  // ��ô��
	C_PATH = COLOR(255, 0, 0);


	C_HYDRANT[0] = COLOR(255, 0, 255); // ��ȭ��
	C_HYDRANT[1] = COLOR(12, 12, 12); // ��ȭ��_2 (���Ǽ���)
	C_HYDRANT[2] = COLOR(23, 23, 23); // ��ȭ��_3 (���Ǽ���)
	C_HYDRANT[3] = COLOR(0, 24, 24); // ȣ����
	C_HYDRANT[4] = COLOR(123, 124, 123); // ��ȭ�� (���Ǽ���)
	C_HYDRANT[5] = COLOR(100, 0, 100); // fire monitor

	C_STAIR = COLOR(99, 111, 122);

	/// ���� ���� ���
	C_GUIDELINE = COLOR(255, 0, 127); // ���̵����
	C_HIGHLIGHTED_HYDRANT = COLOR(255, 255, 0); // ���̶���Ʈ�� ��ȭ��
	C_UNACTIVATED_HYDRANT = COLOR(122, 0, 0); // ��Ȱ��ȭ�� hydrant

	C_VIRTUAL = COLOR(111, 111, 111);

	C_MONITOR_SYMBOL = COLOR(255, 0, 255);
	C_MONITOR_STREAM = COLOR(0, 0, 255);
	///

	DEFAULT_PATH_LENGTH = 15;    /// ȣ�� �⺻����
	DEFAULT_STREAM_LENGTH = 15;     /// ���ٱ� �⺻����


	m_highlighted_stair = -1; // �ƹ��͵� ���̶���Ʈ ���� �� -1

	m_currentlayer = 0; // ���° OBSTACLE�� �� ������.
	m_drawingmode = false;

	

	/// ����ȭ �Ķ���� ///
	Infeasible_range = 0.6;
	door_preference = 5;
	door_damping = 0.5;
	path_preference = 3;
	path_damping = 0.5;

	/// ����Ʈ ����
	GRIDSIZE = 3;   /// default �׸��� ������
	TEMP_GRIDSIZE = 3;
	DISTANCE_PER_PIXEL = 0.5;
	DISTANCE_PER_GRID = DISTANCE_PER_PIXEL * GRIDSIZE;


	m_numofstair = 0;
	for (int j = 0; j < MAX_STAIRS; j++)
	{
		STAIR_PENALTY[j] = 2;
	}
	m_user_loaded = false;
	SLICERATE = 1;			// smoothing ������ padding���� ���ҷ���
	SCALELENGTH = 100;		// ��ô���� ���� ����

}

void output::Init(int mode)   // ���α׷� �ʱ�ȭ. mode 0 = ����ȭ���, mode 1 = �׸�����, mode 2 = ����ȭ���
{


	static bool first = true;
	
	if (m_user_loaded == false)
		GRIDSIZE = TEMP_GRIDSIZE;

	printf("<<�ʱ�ȭ ��>>\n");
	/// ��Ʈ ����
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, GRIDSIZE / 4 + 1, GRIDSIZE / 4 + 1, 0, GRIDSIZE / 4 + 1); // ��Ʈ ����
	///




	/// ���� �ε� ///
	printf("%s �ʱ�ȭ ��\n", m_file);
	m_ents = cvLoadImage(m_file);

	m_program_mode = mode;

	/// ��ô���� DISTANCE_PER_GRID ���� ///
	if (m_user_loaded == false)
	{
		printf("��ô�� ���� ��\n");
		detect_scaleline();
	}
	else
	{
		Set_ScaleLine(m_scaleline.x1, m_scaleline.x2, m_scaleline.y, m_scaleline.length);
	}
	/// �ʱ� ��ġ�� ��ȭ���� ��ġ�� ��� �κ� ///  ó�� �ε��ҋ��� ����
	if (mode != MODE_DRAWING)
	{

	}

	/// �׸��� �ʱ�ȭ ///
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

	/// �׸��� �ʱ�ȭ ///


	if (first == true && m_user_loaded == false) // ó�� �ʱ�ȭ ȣ�� ��
	{
		printf("�ʱ� ��ġ ��ȭ�� ��� ��\n");
		m_hydpos = detect_initial_hydrants();
		sorthyds(); // �ҹ��ҵ� ����(Ÿ�Ժ���)
	}
	else
	{
		for (int i = 0; i < m_hydpos.size(); i++) // ��ȭ�� ��ǥ �籸��
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
	int stairindex = 0; // ��� �ѹ���
	int y = 0;


	for (int i = 0; i < img->width; i++)
	{
		for (int j = 0; j < img->height; j++)
		{
			/// ��ô ǥ�� �κ� �д� ���� (���μ����� ����)
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
			/// ��ô ǥ�� �κ� �д� ����
		}

	}
	if (minscale == 65535 && maxscale == 0)
	{
		printf("error : ��ô ������ �����ϴ�");
	}
	else
	{
		m_scaleline = ScaleLine(minscale, maxscale, y, SCALELENGTH);
		DISTANCE_PER_PIXEL = SCALELENGTH / (maxscale - minscale);
		DISTANCE_PER_GRID = DISTANCE_PER_PIXEL * GRIDSIZE;
	}
}


vector<HYDRANT> output::detect_initial_hydrants() // �ʱ� ��ġ�� ��ȭ���� ��ġ�� ���Ⱚ�� ��� �Լ�
{
	IplImage *img = m_ents;
#define RADIUS_OF_SYMBOL 2.0 // �νı��� 2m
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
				/// ROI�� ���ϰ� ���������� Ž��
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
							setcolor(img, x, y, 0, 0, 0); // �������� ����
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

				container.push_back(HYDRANT(round((double)centerx / GRIDSIZE), round((double)centery / GRIDSIZE), DEFAULT_STREAM_LENGTH, DEFAULT_PATH_LENGTH, type, 0, container.size() % 9));   // �׸���󿡼��� ��ǥ�� ����
			}
		}
	}
	return container;
}


void output::Initialize_grid() // �� ����
{
	printf("�׸��� ������\n");
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


	printf("wall, walkable_area, path �ν� �� \n");
	for (int x = 0; x < maxx; x++)
	{
		for (int y = 0; y < maxy; y++)
		{
			// ��� �ν� //
			int xx = x / GRIDSIZE;
			int yy = y / GRIDSIZE;
			if (iswall(img, x, y))		// ���̸�
			{
				m_grid[xx][yy].iswall = true;
			}
			else if (isstair(img, x, y))							// ����̸�
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
					if (isobstacle(img, x, y, i))		// ��κ��̸�
					{
						m_grid[xx][yy].isobstacle[i] = true;
					}
				}
			}
		}
	}
	/// ���(��, ��κ�, ���, ��ٸ� ��) �ν� �κ� //




	int stairindex = 0;
	printf("��� �ν� ��\n");
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
	printf("��� �ν� ���� \n");
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



	printf("�� �ν� �� \n");

	/// ���ӵ� �� �ν�
	vector<vector<Node>> grid = m_grid; // ���纻
	vector<vector<Node>> grid2 = m_grid; // ���纻

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

						// �ð�������� Ž��

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

						// �ݽð�
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


						if (numofdirection == 0) // ���� ������
						{
							if ((last_dir == 4 && grid2[x - 1][y].isobstacle[k] == false && grid2[x - 1][y + 1].isobstacle[k] == false && grid2[x - 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false)
								|| (last_dir == 5 && grid2[x - 1][y + 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x - 1][y - 1].isobstacle[k] == false)
								|| (last_dir == 6 && grid2[x - 1][y + 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y].isobstacle[k] == false) // �¿�
								|| (last_dir == 7 && grid2[x + 1][y].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x - 1][y + 1].isobstacle[k] == false)
								|| (last_dir == 0 && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x + 1][y].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false)
								|| (last_dir == 1 && grid2[x + 1][y].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x - 1][y - 1].isobstacle[k] == false)
								|| (last_dir == 2 && grid2[x - 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y].isobstacle[k] == false)
								|| (last_dir == 3 && grid2[x - 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x - 1][y + 1].isobstacle[k] == false))

							{
								grid2[x][y].isobstacle[k] = false;
							}
							//grid2[x][y].isstair = true;
							if (memorized.empty()) // �׸��� �ѹ� ���Ƽ� �°���, ���� �������
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

					// �ð�������� Ž��

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

					// �ݽð�
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

					if (numofdirection == 0) // ���� ������
					{
						if ((last_dir == 4 && grid2[x - 1][y].iswall == false && grid2[x - 1][y + 1].iswall == false && grid2[x - 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x][y + 1].iswall == false)
							|| (last_dir == 5 && grid2[x - 1][y + 1].iswall == false && grid2[x][y + 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x - 1][y - 1].iswall == false)
							|| (last_dir == 6 && grid2[x - 1][y + 1].iswall == false && grid2[x][y + 1].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y].iswall == false) // �¿�
							|| (last_dir == 7 && grid2[x + 1][y].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x][y + 1].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x - 1][y + 1].iswall == false)
							|| (last_dir == 0 && grid2[x + 1][y + 1].iswall == false && grid2[x + 1][y].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x][y + 1].iswall == false)
							|| (last_dir == 1 && grid2[x + 1][y].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x - 1][y - 1].iswall == false)
							|| (last_dir == 2 && grid2[x - 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y].iswall == false)
							|| (last_dir == 3 && grid2[x - 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x - 1][y + 1].iswall == false))

						{
							grid2[x][y].iswall = false;
						}
						//grid2[x][y].isstair = true;
						if (memorized.empty()) // �׸��� �ѹ� ���Ƽ� �°���, ���� �������
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

	/// ���ӵ� �� �ν�
	grid = grid2; // ���纻

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

						// �ð�������� Ž��

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

						// �ݽð�
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

						if (numofdirection == 0) // ���� ������
						{
							if ((last_dir == 4 && grid2[x - 1][y].isobstacle[k] == false && grid2[x - 1][y + 1].isobstacle[k] == false && grid2[x - 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false)
								|| (last_dir == 5 && grid2[x - 1][y + 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x - 1][y - 1].isobstacle[k] == false)
								|| (last_dir == 6 && grid2[x - 1][y + 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y].isobstacle[k] == false) // �¿�
								|| (last_dir == 7 && grid2[x + 1][y].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x - 1][y + 1].isobstacle[k] == false)
								|| (last_dir == 0 && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x + 1][y].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x][y + 1].isobstacle[k] == false)
								|| (last_dir == 1 && grid2[x + 1][y].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x + 1][y + 1].isobstacle[k] == false && grid2[x - 1][y - 1].isobstacle[k] == false)
								|| (last_dir == 2 && grid2[x - 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y].isobstacle[k] == false)
								|| (last_dir == 3 && grid2[x - 1][y - 1].isobstacle[k] == false && grid2[x][y - 1].isobstacle[k] == false && grid2[x - 1][y].isobstacle[k] == false && grid2[x + 1][y - 1].isobstacle[k] == false && grid2[x - 1][y + 1].isobstacle[k] == false))

							{
								m_grid[x][y].lossed = true;             // �ν��� �κ����� �˸�
							}
							//grid2[x][y].isstair = true;
							if (memorized.empty()) // �׸��� �ѹ� ���Ƽ� �°���, ���� �������
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

					// �ð�������� Ž��

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

					// �ݽð�
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



					if (numofdirection == 0) // ���� ������
					{
						if ((last_dir == 4 && grid2[x - 1][y].iswall == false && grid2[x - 1][y + 1].iswall == false && grid2[x - 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x][y + 1].iswall == false)
							|| (last_dir == 5 && grid2[x - 1][y + 1].iswall == false && grid2[x][y + 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x - 1][y - 1].iswall == false)
							|| (last_dir == 6 && grid2[x - 1][y + 1].iswall == false && grid2[x][y + 1].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y].iswall == false) // �¿�
							|| (last_dir == 7 && grid2[x + 1][y].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x][y + 1].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x - 1][y + 1].iswall == false)
							|| (last_dir == 0 && grid2[x + 1][y + 1].iswall == false && grid2[x + 1][y].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x][y + 1].iswall == false)
							|| (last_dir == 1 && grid2[x + 1][y].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x + 1][y + 1].iswall == false && grid2[x - 1][y - 1].iswall == false)
							|| (last_dir == 2 && grid2[x - 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y].iswall == false)
							|| (last_dir == 3 && grid2[x - 1][y - 1].iswall == false && grid2[x][y - 1].iswall == false && grid2[x - 1][y].iswall == false && grid2[x + 1][y - 1].iswall == false && grid2[x - 1][y + 1].iswall == false))

						{
							m_grid[x][y].lossed = true;
						}
						if (memorized.empty()) // �׸��� �ѹ� ���Ƽ� �°���, ���� �������
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

	printf("OUT_OF_ROI �ν� �� \n");
	/// ���� ���� �ν�
	int width = m_grid.size();
	int height = m_grid[0].size();

	pair<int, int> start = pair<int, int>(2, 2); // flood-fill ��������
	/// flood-fill ����
	stack<pair<int, int>> ff;
	ff.push(start);

	/// wall�� ����
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
	/// walkable distance�� ����
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
	/// path�� ����
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


	printf("streamable, floodable, recommended point �ν� ��\n");
	bool finished_detecting_wd = false; // �ѹ� �÷��� ���� ���̻� wd�� ���� flood-fill ����
	/// ���� �νĿ� ���Ŵ� flood-fill
	for (int x = 0; x < width - 4; x++)
	{
		for (int y = 0; y < height - 4; y++)
		{
			Node point = m_grid[x][y];
			/// steramable flood-fill ����

			if (point.iswall == false && point.isoutofwall == false && point.istempstreamable == false)
			{
				bool there_can_be_moved = false; // ����� �̵��� �� �ִ� �������

				pair<int, int> start = pair<int, int>(x, y);
				stack<pair<int, int>> ff;
				ff.push(start);
				set<node> tempset;
				while (1)
				{
					if (ff.empty() == true)
					{
						if (there_can_be_moved == true) // ���� ������ ����̸�
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


			if (point.isobstacle[m_currentlayer] == false && point.isoutofwd == false && point.isfloodable == false && finished_detecting_wd == false && point.isequipment == false) // walkable distance�� �� ���� ä��� �ȴ�
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
	

	/// infeasible ����
	printf("infeasible ���� ���� �� \n");

	double lengthofrestrictedgrid = Infeasible_range / DISTANCE_PER_GRID; // ��׸��常ŭ infeasibe�ΰ�?

	for (int i = 2; i < m_grid.size() - 6; i++)
	{
		for (int j = 2; j < m_grid[i].size() - 6; j++)
		{
			if (m_grid[i][j].isequipment)
			{
				float RESOLUTION = 10.0; // ��� ���� ������ �˻��Ұ���
				int curx = i;
				int cury = j;
				int prex = curx;
				int prey = cury;
				int x = curx;
				int y = cury;
				for (float a = 0; a < 360; a += RESOLUTION) // 15������ �˻�
				{
					float deltax = cos(a / 180 * 3.1415);
					float deltay = sin(a / 180 * 3.1415);
					float step = 1 / max(abs(deltax), abs(deltay)) + 0.01; // ó���� ��� �������� ������
					for (double k = step; k < lengthofrestrictedgrid; k += 0.5)
					{
						curx = x + (int)(deltax*k);
						cury = y + (int)(deltay*k);
						if (!(curx == prex && cury == prey)) // ���� ���� ������
						{
							if (m_grid[curx][cury].isobstacle[m_currentlayer] == true)
								break;
							else
							{
								m_grid[curx][cury].isfeasible = false; // infeasible�� ����
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


	/// feasible ���
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


	/// �� �ν� ///
	printf("door �ν� �� \n");
	vector<vector<node>> candidates;
	for (int x = 0; x < width - 4; x++)
	{
		for (int y = 0; y < height - 4; y++)
		{
			if (m_grid[x][y].iswall && m_grid[x][y].isfloodable && m_grid[x][y].checked == false) // ���ε� walkable area�� �ִ� ���
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
	/// �ߺ����� 

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


