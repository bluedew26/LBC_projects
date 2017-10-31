#include "stdafx.h"
#include "hydrant.h"


/// drawing 관련 ///
void output::Draw_2_Point(int x1, int y1, int x2, int y2, int type_2p, int type_object, int layernum)
{
	static int noaction = 0;
	static bool noaction2 = false;
	static int noaction3 = 0;
	if (noaction == 0)
	{
		switch (type_2p)
		{
		case TYPE_2P_RECTANGLE:
			noaction = 3; break; // 앞으로 3번의 입력은 액션스택에 안넣음
		case TYPE_2P_DOUBLELINE:
			noaction = 1; break; // 앞으로 1번의 입력은 액션스택에 안 넣음.
		default: break;
		}
	}
	int maxx = m_grid.size() - 2;
	int maxy = m_grid[0].size() - 2;
//	if (x1 < 0 || x1 > maxx || y1 < 0 || y1 > maxy || x2 < 0 || x2 > maxx || y2 < 0 || y2 > maxy)
//	{
//		noaction2 = true;
//		return;
//	}
	if (noaction2 == true && noaction > 0)
	{
		noaction--;
		if (noaction == 0)
			noaction2 = false;
		return;
	}



	/// 액션스택에 삽입
	if (noaction3 == 0)
	{
		switch (type_2p)
		{
		case TYPE_2P_LINE:
			m_ActStack.push_back(DRAW_LINE); break;
		case TYPE_2P_RECTANGLE:
			m_ActStack.push_back(DRAW_RECTANGLE); noaction3 = 3; break; // 앞으로 3번의 입력은 액션스택에 안넣음
		case TYPE_2P_CIRCLE:
			m_ActStack.push_back(DRAW_CIRCLE); break;
		case TYPE_2P_DOUBLELINE:
			m_ActStack.push_back(DRAW_DOUBLELINE); noaction3 = 1; break; // 앞으로 1번의 입력은 액션스택에 안 넣음.
		default: break;
		}
		if (m_ActStack.size() > MAX_STACK)
		{
			m_ActStack.pop_front();
		}
	}
	else
		noaction3--;


	int unum = 1; // 1부터 시작
	while (1)
	{
		if (find(m_drawlinenodes.begin(), m_drawlinenodes.end(), unum) == m_drawlinenodes.end()) // 고유번호 0부터 없는 번호 탐색
		{
			break;
		}
		unum++;
	}
	m_drawlinenodes.push_back(drawinginfo(x1, y1, x2, y2, type_2p, type_object, unum, layernum));
	Update_Drawing();
}
void output::Update_Drawing()
{
	/// clear
	for (int i = 1; i < m_grid.size() - 4; i++)
	{
		for (int j = 1; j < m_grid[i].size() - 4; j++)
		{
			for (int k = 0; k < NUMOFLAYER; k++)
			{
				m_grid[i][j].isobstacle[k] = false;
			}
			m_grid[i][j].iswall = false;
			m_grid[i][j].isequipment = false;
			m_grid[i][j].ispath = false;
			m_grid[i][j].unum = 0;
		}
	}
	for (int i = 0; i < m_drawlinenodes.size(); i++)
	{
		int unum = m_drawlinenodes[i].unum;
		int &x1 = m_drawlinenodes[i].x1; int &y1 = m_drawlinenodes[i].y1;
		int &x2 = m_drawlinenodes[i].x2; int &y2 = m_drawlinenodes[i].y2;
		int type_object = m_drawlinenodes[i].type_object;
		int type_2p = m_drawlinenodes[i].type_2p;
		int layernum = m_drawlinenodes[i].layernum;
		double dis = floor(sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2)));
		double deltax = (x2 - x1) / dis;
		double deltay = (y2 - y1) / dis;
		



		int maxx = m_grid.size() - 3;
		int maxy = m_grid[0].size() - 3;

		if (x1 < 0 || x1 > maxx || y1 < 0 || y1 > maxy)
		{
			/// x1과 x2를 스왑
			int tempx = x1, tempy = y1;
			x1 = x2, y1 = y2;
			x2 = tempx, y2 = tempy;
			deltax = -deltax, deltay = -deltay;
		}

		int x = x1;
		int y = y1;
		int prevx = x1, prevy = y2;
		for (double i = 0; i <= dis; i += 0.5)
		{
			x = x1 + (int)(deltax*i);
			y = y1 + (int)(deltay*i);
			if (x < 0 || x > maxx || y < 0 || y > maxy)
			{
				break;
			}

			switch (type_object)
			{
			case OBJECT_WALL:
				m_grid[x][y].iswall = true; break;
			case OBJECT_OBSTACLE:
				m_grid[x][y].isobstacle[layernum] = true; break;
			case OBJECT_PATH:
				m_grid[x][y].ispath = true; break;
			case OBJECT_EQUIPMENT:
				m_grid[x][y].isequipment = true; break;
			default:
				break;
			}
			prevx = x, prevy = y;
			m_grid[x][y].unum = unum;
		}
		
		if (x2 < 0 || x2 > maxx || y2 < 0 || y2 > maxy)
		{
			x2 = prevx; y2 = prevy;
			continue;
		}
		else
		{
			x = x2;
			y = y2;
			switch (type_object)
			{
			case OBJECT_WALL:
				m_grid[x][y].iswall = true; break;
			case OBJECT_OBSTACLE:
				m_grid[x][y].isobstacle[layernum] = true; break;
			case OBJECT_PATH:
				m_grid[x][y].ispath = true; break;
			case OBJECT_EQUIPMENT:
				m_grid[x][y].isequipment = true; break;
			default:
				break;
			}
			m_grid[x][y].unum = unum;
		}
		
	}
}


void output::reset_drawing()
{
	m_drawlinenodes.clear();
	m_ActStack.clear();
	m_hydpos.clear();
	Update_Drawing();
}

int output::Trim_Line(int x, int y) // 마우스 위치를 자름
{
	int unum = m_grid[x][y].unum;
	if (unum == 0) return -1; // 선 없는 곳이면 무시 아니면 계속
	vector<drawinginfo>::iterator it = find(m_drawlinenodes.begin(), m_drawlinenodes.end(), unum);
	if (it != m_drawlinenodes.end()) // 찾으면
	{
		int index = 0;
		vector<drawinginfo>::iterator it2 = it;
		while (1)
		{
			if (it2 == m_drawlinenodes.begin())
				break;
			index++;
			it2--;
		}
		/// vector의 몇번째 인덱스에 있는 선인지 알아냄
		drawinginfo trimmed = *it;

		int type = it->type_object;
		int type_2p = it->type_2p;
		int layernum = it->layernum;
		ptr start = ptr(it->x1, it->y1);
		ptr end = ptr(it->x2, it->y2);
		int startx = start.x;
		int starty = start.y;

		m_drawlinenodes.erase(it); // 이 시점에서 지우고 다시 그림
		Update_Drawing();



		int sx = 0, sy = 0, curx = 0, cury = 0;
		int i = 0;
		int numofadded = 0;
		ptr col1, col2;

		bool blocked_towarding_start = false;
		bool blocked_towarding_end = false;

		/// 1. trim 지점으로부터 시작점으로 향함
		/// 중간에 만날 경우 시작점 - 충돌점을 잇는 선을 새로 생성
		sx = x;
		sy = y;
		double dis = floor(sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2)));
		double deltax = (end.x - start.x) / dis;
		double deltay = (end.y - start.y) / dis;

		while (1)
		{
			bool escape_flag = false;
			curx = sx + (int)(deltax*i);
			cury = sy + (int)(deltay*i);

			switch (type)
			{
			case OBJECT_WALL:
				if (curx == start.x && cury == start.y || sqrt(pow(curx - start.x, 2) + pow(cury - start.y, 2)) <= 1.1) // starting point를 만났다면 루프 종료
				{
					escape_flag = true;
					break;  // 무시해도 됌
				}
				else if (m_grid[curx][cury].iswall == true)
				{
					blocked_towarding_start = true;
					numofadded++;
					escape_flag = true;
					col1 = ptr(curx, cury);
				}
				break;
			case OBJECT_OBSTACLE:
				if (curx == start.x && cury == start.y || sqrt(pow(curx - start.x, 2) + pow(cury - start.y, 2)) <= 1.1) // starting point를 만났다면 루프 종료
				{
					escape_flag = true;
					break;  // 무시해도 됌
				}
				else if (m_grid[curx][cury].isobstacle[layernum] == true)
				{
					blocked_towarding_start = true;
					numofadded++;
					escape_flag = true;
					col1 = ptr(curx, cury);
				}
				break;
			case OBJECT_PATH:
				if (curx == start.x && cury == start.y || sqrt(pow(curx - start.x, 2) + pow(cury - start.y, 2)) <= 1.1) // starting point를 만났다면 루프 종료
				{
					escape_flag = true;
					break;  // 무시해도 됌
				}
				else if (m_grid[curx][cury].ispath == true)
				{
					blocked_towarding_start = true;
					numofadded++;
					escape_flag = true;
					col1 = ptr(curx, cury);
				}
				break;
			case OBJECT_EQUIPMENT:
				if (curx == start.x && cury == start.y || sqrt(pow(curx - start.x, 2) + pow(cury - start.y, 2)) <= 1.1) // starting point를 만났다면 루프 종료
				{
					escape_flag = true;
					break;  // 무시해도 됌
				}
				else if (m_grid[curx][cury].isequipment == true)
				{
					blocked_towarding_start = true;
					numofadded++;
					escape_flag = true;
					col1 = ptr(curx, cury);
				}
				break;
			default:
				break;
			}
			if (escape_flag == true)
				break;
			i--;
		}
		/// 2. trim 지점으로부터 도착점으로 향함
		/// 중간에 만날 경우 시작점 - 충돌점을 잇는 선을 새로 생성

		curx = 0, cury = 0, i = 0;
		while (1)
		{
			bool escape_flag = false;
			curx = sx + (int)(deltax*i);
			cury = sy + (int)(deltay*i);

			switch (type)
			{
			case OBJECT_WALL:
				if (curx == end.x && cury == end.y || sqrt(pow(curx-end.x, 2) + pow(cury-end.y, 2)) <= 1.1) // starting point를 만났다면 루프 종료
				{
					escape_flag = true;
					break;  // 무시해도 됌
				}
				else if (m_grid[curx][cury].iswall == true)
				{
					blocked_towarding_end = true;
					numofadded++;
					escape_flag = true;
					col2 = ptr(curx, cury);
				}
				break;
			case OBJECT_OBSTACLE:
				if (curx == end.x && cury == end.y || sqrt(pow(curx - end.x, 2) + pow(cury - end.y, 2)) <= 1.1) // starting point를 만났다면 루프 종료
				{
					escape_flag = true;
					break;  // 무시해도 됌
				}
				else if (m_grid[curx][cury].isobstacle[layernum] == true)
				{
					blocked_towarding_end = true;
					numofadded++;
					escape_flag = true;
					col2 = ptr(curx, cury);
				}
				break;
			case OBJECT_PATH:
				if (curx == end.x && cury == end.y || sqrt(pow(curx - end.x, 2) + pow(cury - end.y, 2)) <= 1.1) // starting point를 만났다면 루프 종료
				{
					escape_flag = true;
					break;  // 무시해도 됌
				}
				else if (m_grid[curx][cury].ispath == true)
				{
					blocked_towarding_end = true;
					numofadded++;
					escape_flag = true;
					col2 = ptr(curx, cury);
				}
				break;
			case OBJECT_EQUIPMENT:
				if (curx == end.x && cury == end.y || sqrt(pow(curx - end.x, 2) + pow(cury - end.y, 2)) <= 1.1) // starting point를 만났다면 루프 종료
				{
					escape_flag = true;
					break;  // 무시해도 됌
				}
				else if (m_grid[curx][cury].isequipment == true)
				{
					blocked_towarding_end = true;
					numofadded++;
					escape_flag = true;
					col2 = ptr(curx, cury);
				}
				break;
			default:
				break;
			}
			if (escape_flag == true)
				break;
			i++;
		}
		/// 기존 충돌점은 제거
		m_ActStack.push_back(ActionStack(DRAW_TRIM, trimmed, index, numofadded));
		
		if (blocked_towarding_start)
		{
			int unum = 1; // 1부터 시작
			while (1)
			{
				if (find(m_drawlinenodes.begin(), m_drawlinenodes.end(), unum) == m_drawlinenodes.end()) // 고유번호 0부터 없는 번호 탐색
				{
					break;
				}
				unum++;
			}
			m_drawlinenodes.insert(m_drawlinenodes.begin() + index, drawinginfo(start.x, start.y, col1.x, col1.y, type_2p, type, unum, layernum));
		}
		if (blocked_towarding_end)
		{
			int unum = 1; // 1부터 시작
			while (1)
			{
				if (find(m_drawlinenodes.begin(), m_drawlinenodes.end(), unum) == m_drawlinenodes.end()) // 고유번호 0부터 없는 번호 탐색
				{
					break;
				}
				unum++;
			}
			m_drawlinenodes.insert(m_drawlinenodes.begin() + index, drawinginfo(col2.x, col2.y, end.x, end.y, type_2p, type, unum, layernum));
		}

		Update_Drawing();
		return 1;
	}
	return -1;
}

void output::Undo()
{
	if (m_ActStack.empty())
		return;

	ActionStack top = m_ActStack.back();
	ActionType type = top.type;
	drawinginfo trimmed = top.trimmed;
	int triindex = top.trimmedindex;
	HYDRANT hyd = top.prev_hyd;
	int numofadded = top.numofadded;
	vector<HYDRANT>::iterator it = find(m_hydpos.begin(), m_hydpos.end(), hyd.unum);
	bool need_to_update_drawing = false;
	switch (type)
	{
	case DRAW_LINE: m_drawlinenodes.pop_back(); need_to_update_drawing = true; break;
	case DRAW_DOUBLELINE: m_drawlinenodes.pop_back(); m_drawlinenodes.pop_back(); need_to_update_drawing = true; break;
	case DRAW_RECTANGLE: m_drawlinenodes.pop_back(); m_drawlinenodes.pop_back(); m_drawlinenodes.pop_back(); m_drawlinenodes.pop_back(); need_to_update_drawing = true; break;
	case DRAW_CIRCLE: need_to_update_drawing = true; break;
	case DRAW_TRIM:
		if (numofadded > 0)
		{
			m_drawlinenodes.erase(m_drawlinenodes.begin() + triindex, m_drawlinenodes.begin() + triindex + numofadded);
		}
		m_drawlinenodes.insert(m_drawlinenodes.begin() + triindex, trimmed);
		need_to_update_drawing = true;
		break;
	case ADD_HYDRANT:
		if (it != m_hydpos.end()) // 고유넘버 탐색시 발견하면
		{
			m_hydpos.erase(it); // 지운다
		}
		sorthyds();
		break;
	case MODIFY_HYDRANT:  // 수정 혹은 MOVE
		if (it != m_hydpos.end()) // 고유넘버 탐색시 발견하면
		{
			int count = 0;
			while (1)
			{
				if (it == m_hydpos.begin())
					break;
				count++;
				it--;
			}
			int hydnum = count; // 하이드런트 넘버를 얻음
			m_hydpos[hydnum] = hyd;
		}
		sorthyds();
		break;
	case DELETE_HYDRANT:
		m_hydpos.push_back(hyd);
		sorthyds();
		break;
	default: break;
	}
	if (need_to_update_drawing) // 드로잉 관련으로 손 댄 경우 업데이트
		Update_Drawing();

	m_ActStack.pop_back();
}