#include "stdafx.h"
#include "hydrant.h"

void output::calculate_hydrant_area(int hydnum, bool update) // update = false면 무조건 재계산, true면 선별적 재계산
{
	printf("\n[calculating area] : hydnum : %d\n", hydnum);
	if (update == true)
	{
		bool ntu = m_hydpos[hydnum].needtoupdate;
		m_hydpos[hydnum].needtoupdate = 0;
		if (m_hydpos[hydnum].activate == true)
		{
			if (ntu == 1)
			{
				flood(m_hydpos[hydnum]);
				stream(m_hydpos[hydnum], true);
			}
			else if (ntu == 2)
				stream(m_hydpos[hydnum], true);
		}
	}
	else
	{
		flood(m_hydpos[hydnum]);
		stream(m_hydpos[hydnum]);
	}
}

void output::Highlight_Hyd(int hydnum)
{
	m_hydpos[hydnum].highlighted = true;
}

void output::Add_Hyd(int xpos, int ypos, double length, double streamlength, int type, int sourcenum, int colorindex, int activation, bool calculation)
{
	HYDRANT hyd;
	hyd.x = xpos; hyd.y = ypos;
	hyd.length = length;
	hyd.streamlength = streamlength;
	hyd.sourcenum = sourcenum;
	hyd.colorindex = colorindex;
	hyd.type = type;
	hyd.activate = activation;
	hyd.highlighted = 0;
	hyd.needtoupdate = 1;
	int unum = 0;
	while (1)
	{
		if (find(m_hydpos.begin(), m_hydpos.end(), unum) == m_hydpos.end()) // 고유번호 0부터 없는 번호 탐색
		{
			hyd.unum = unum;
			break;
		}
		unum++;
	}
	//m_hydpos.push_back(HYDRANT(xpos,ypos,cos(angle),sin(angle),streamlength,length,sourcenum));
	m_hydpos.push_back(hyd);
	int hydnum = m_hydpos.size() - 1;
	if (calculation)
		calculate_hydrant_area(hydnum, true);
	sorthyds();

	m_ActStack.push_back(ActionStack(ADD_HYDRANT, hyd));

}

void output::Add_Monitor(int x1, int x2, int x3, int y1, int y2, int y3, double streamlength, int colorindex)
{
	HYDRANT hyd = HYDRANT(x1, x2, x3, y1, y2, y3, streamlength, colorindex);

	int unum = 0;
	while (1)
	{
		if (find(m_hydpos.begin(), m_hydpos.end(), unum) == m_hydpos.end()) // 고유번호 0부터 없는 번호 탐색
		{
			hyd.unum = unum;
			break;
		}
		unum++;
	}
	m_hydpos.push_back(hyd);
	if (m_user_loaded == false)
		calculate_hydrant_area(m_hydpos.size() - 1, true);
	sorthyds();
	m_ActStack.push_back(ActionStack(ADD_HYDRANT, hyd));
}

void output::Delete_Hyd(int hydnum)
{
	vector<HYDRANT>::iterator it;
	it = m_hydpos.begin();
	it += hydnum;
	HYDRANT prehyd = *it;
	m_hydpos.erase(it);
	m_ActStack.push_back(ActionStack(DELETE_HYDRANT, prehyd));
}

int output::Update_Single_Hydrant() // 업데이트가 필요한 하나의 소화전만 계산 
{
	for (int i = 0; i < m_hydpos.size(); i++)
	{
		if (m_hydpos[i].needtoupdate && m_hydpos[i].activate)
		{
			calculate_hydrant_area(i, m_hydpos[i].needtoupdate);
			m_hydpos[i].needtoupdate = 0;
			return 1;
		}

	}
	return -1; // 더이상 갱신할게 없을 떄 -1 리턴
}


void output::Recalculate_All()
{
	for (int j = 0; j < m_hydpos.size(); j++)
	{
		m_hydpos[j].needtoupdate = 1;
	}
}
void output::Detect_Uncontinuous_Point()
{
	detect_closed_loop();
}



int output::istherehydrant(int x, int y)
{
	vector<HYDRANT>::iterator it;
	it = find(m_hydpos.begin(), m_hydpos.end(), HYDRANT(x, y, 0, 0, 0, 0));
	if (it == m_hydpos.end())
		return -1;
	else
	{
		if (it->activate == false)
			return -1;
		int i = 0;
		while (1)
		{
			if (it - i == m_hydpos.begin())
				break;
			i++;
		}
		return i;
	}
}

int output::istherestair(int x, int y)   // 그리드 단위
{
	if (m_grid[x][y].isstair == true)
	{
		return m_grid[x][y].stairindex;
	}
	else
		return -1;
}


