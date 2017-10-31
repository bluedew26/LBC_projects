#include "stdafx.h"
#include "hydrant.h"
#include "functions.h"

#define DISPLAYSTREAM		 0x1
#define DISPLAYPATH			 0x2
#define DISPLAYTEXT			 0x4
#define DISPLAYGUIDELINE	 0x8
#define DISPLAYMISSINGLINE	 0x10
#define DISPLAYSINGLE		 0x20
#define DISPLAYOVERLAP		 0x40
#define DISPLAYBACKGROUNDDRAWING 0x80
#define DISPLAY_SIMPLECHECK  0x100
#define DISPLAY_DETECTION_FLOODABLE 0x200
#define DISPLAY_DETECTION_STREAMABLE 0x400
#define DISPLAY_DETECTION_PATH 0x800
#define DISPLAY_DETECTION_FEASIBLE 0x1000
#define DISPLAY_DETECTION_DOOR 0x2000
#define DISPLAY_DETECTION_PREFERENCE 0x4000
#define DISPLAY_DETECTION_CHECK 0x8000


#define IND_LIMIT 1

void output::saveinfo(const char *path)
{
	FILE *file;
	file = fopen(path, "w"); // write

	/// GRIDSIZE, 축척길이, 기준선
	fprintf(file, "<MODE>\n%d\n", m_program_mode);
	fprintf(file, "<GRIDSIZE>\n%d\n", GRIDSIZE);
	fprintf(file, "<축척데이터>\n%d %d %d %.2lf\n", m_scaleline.x1, m_scaleline.x2, m_scaleline.y, m_scaleline.length);

	/// 소화전위치
	fprintf(file, "<소화전위치>\n");
	int numofnotmonitor = 0;
	int numofmonitor = 0;
	for (int i = 0; i < m_hydpos.size(); i++)
	{
		if (m_hydpos[i].type != TYPE_MONITOR)
			numofnotmonitor++;
		else
			numofmonitor++;
	}
	fprintf(file, "%d\n", numofnotmonitor);
	for (int i = 0; i < m_hydpos.size(); i++)
	{
		HYDRANT &hyd = m_hydpos[i];
		if (hyd.type != TYPE_MONITOR)
			fprintf(file, "%d %d %.2lf %.2lf %d %d %d\n", hyd.x, hyd.y, hyd.length, hyd.streamlength, hyd.type, hyd.sourcenum, hyd.colorindex); // 순서는 이와같음
	}
	/// 모니터위치
	fprintf(file, "<모니터위치>\n");
	fprintf(file, "%d\n", numofmonitor);
	for (int i = 0; i < m_hydpos.size(); i++)
	{
		HYDRANT &hyd = m_hydpos[i];
		if (hyd.type == TYPE_MONITOR)
			fprintf(file, "%d %d %d %d %d %d %.2lf %d\n", hyd.xx[0], hyd.xx[1], hyd.xx[2], hyd.yy[0], hyd.yy[1], hyd.yy[2], hyd.streamlength, hyd.colorindex);
	}
	/// DrawingInfo
	fprintf(file, "<그리기정보>\n");
	fprintf(file, "%d\n", m_drawlinenodes.size());
	for (int i = 0; i < m_drawlinenodes.size(); i++)
	{
		drawinginfo &dr = m_drawlinenodes[i];
		fprintf(file, "%d %d %d %d %d %d %d %d\n", dr.x1, dr.x2, dr.y1, dr.y2, dr.type_2p, dr.type_object, dr.layernum, dr.unum);
	}
	///



	fclose(file);
}

void output::loadinfo(const char *path)
{
	m_user_loaded = true;
	FILE *file;
	file = fopen(path, "r"); // write

	/// GRIDSIZE, 축척길이, 기준선
	fscanf(file, "%*s\n");
	fscanf(file, "%d", &m_program_mode);

	fscanf(file, "%*s\n");
	fscanf(file, "%d", &GRIDSIZE);

	int scale_x1, scale_x2, scale_y;
	double scale_length;
	fscanf(file, "%*s\n");
	fscanf(file, "%d %d %d %lf\n", &scale_x1, &scale_x2, &scale_y, &scale_length);
	m_scaleline = ScaleLine(scale_x1, scale_x2, scale_y, scale_length);

	m_hydpos.clear();
	fscanf(file, "%*s\n");
	int numofnotmonitor = 0;
	fscanf(file, "%d\n", &numofnotmonitor);
	for (int i = 0; i < numofnotmonitor; i++)
	{
		int hydx, hydy, hyd_type, hyd_sourcenum, hyd_colorindex;
		double hyd_length, hyd_streamlength;
		fscanf(file, "%d %d %lf %lf %d %d %d\n", &hydx, &hydy, &hyd_length, &hyd_streamlength, &hyd_type, &hyd_sourcenum, &hyd_colorindex);
		Add_Hyd(hydx, hydy, hyd_length, hyd_streamlength, hyd_sourcenum, hyd_type, hyd_colorindex);
	}

	fscanf(file, "%*s\n");
	int numofmonitor = 0;
	fscanf(file, "%d\n", &numofmonitor);
	for (int i = 0; i < numofmonitor; i++)
	{
		int xx[3], yy[3], colorindex;
		double streamlength;
		fscanf(file, "%d %d %d %d %d %d %lf %d\n", &xx[0], &xx[1], &xx[2], &yy[0], &yy[1], &yy[2], &streamlength, &colorindex);
		Add_Monitor(xx[0], xx[1], xx[2], yy[0], yy[1], yy[2], streamlength, colorindex);
	}

	m_drawlinenodes.clear();
	fscanf(file, "%*s\n");
	int numofdrawn = 0;
	fscanf(file, "%d\n", &numofdrawn);
	for (int i = 0; i < numofdrawn; i++)
	{
		int x1, x2, y1, y2, type_2p, type_object, layernum, unum;
		fscanf(file, "%d %d %d %d %d %d %d %d\n", &x1, &x2, &y1, &y2, &type_2p, &type_object, &layernum, &unum);
		m_drawlinenodes.push_back(drawinginfo(x1, y1, x2, y2, type_2p, type_object, unum, layernum));
	}
	fclose(file);


	PRE_GRIDSIZE = GRIDSIZE;
	//Init(m_program_mode);
	//Update_Drawing();
}



int output::saveimg(int displaycode, const char* filepath)
{
	static int ind = 0;
	ind++;
	if (ind > IND_LIMIT)
		ind = 0;


	IplImage* result;
	vector<vector<COLORPATTERN>> pattern; // 실제 색상 정보들이 들어갈 곳


	result = cvCloneImage(m_ents); //값 복사 수행
	cvSet(result, cvScalarAll(255), 0);
	vector<vector<Node>> &Grid = m_grid;
	vector<vector<Node>> &bGrid = m_background_grid;


	pattern.assign(result->width / GRIDSIZE, vector<COLORPATTERN>());
	for (int j = 0; j < pattern.size(); j++)
	{
		pattern[j].assign(result->height / GRIDSIZE, COLORPATTERN()); /// 컨테이너 채우는 작업
	}




	if (displaycode & DISPLAY_SIMPLECHECK) /// simplecheck 모드면
	{
		for (int j = 0; j < m_hydpos.size(); j++)
		{
			int x = m_hydpos[j].x;
			int y = m_hydpos[j].y;
			double length = m_hydpos[j].length / DISTANCE_PER_PIXEL;
			double streamlength = m_hydpos[j].streamlength + length / DISTANCE_PER_PIXEL;
			COLOR col = C_HYDRANT[m_hydpos[j].colorindex];
			COLOR col2 = C_STREAM[m_hydpos[j].colorindex];
			cvCircle(result, CvPoint(x*GRIDSIZE + GRIDSIZE / 2, y*GRIDSIZE + GRIDSIZE / 2), length, CvScalar(col.B, col.G, col.R), GRIDSIZE);
			cvCircle(result, CvPoint(x*GRIDSIZE + GRIDSIZE / 2, y*GRIDSIZE + GRIDSIZE / 2), streamlength, CvScalar(col.B, col.G, col.R), GRIDSIZE);
		}
	}
	else /// 아니면
	{
		for (int j = 0; j < m_hydpos.size(); j++)
		{
			COLOR streamcol, pathcol;
			int colindex = m_hydpos[j].colorindex;
			streamcol = COLOR(C_STREAM[colindex].R, C_STREAM[colindex].G, C_STREAM[colindex].B);
			pathcol = COLOR(C_ROUTE[colindex].R, C_ROUTE[colindex].G, C_ROUTE[colindex].B);



			if (m_hydpos[j].activate == true) // 표시를 활성화한 소화전일때만
			{
				vector<ptr> &stream = m_hydpos[j].stream;


				/// path 처리 부분 
				if (displaycode & DISPLAYPATH)
				{
					vector<ptr> path = m_hydpos[j].range;
					for (int t = 0; t < path.size(); t++)
					{
						ptr pt = path[t];
						int x = pt.x; int y = pt.y;
						pattern[x][y].count++;
						pattern[x][y].cumulB += pathcol.B;
						pattern[x][y].cumulG += pathcol.G;
						pattern[x][y].cumulR += pathcol.R;
					}
				}
				/// stream 처리
				if (displaycode & DISPLAYSTREAM)
				{
					vector<ptr> stream = m_hydpos[j].stream;
					for (vector<ptr>::iterator it = stream.begin(); it != stream.end(); it++)
					{
						ptr pt = *it;
						int x = pt.x; int y = pt.y;
						pattern[x][y].count++;
						pattern[x][y].cumulB += streamcol.B;
						pattern[x][y].cumulG += streamcol.G;
						pattern[x][y].cumulR += streamcol.R;
					}
				}

			}
		}

		for (int j = 0; j < pattern.size(); j++)
		{
			for (int k = 0; k < pattern[j].size(); k++)
			{
				if (pattern[j][k].count != 0)
				{
					unsigned char finalR = pattern[j][k].cumulR / pattern[j][k].count;
					unsigned char finalG = pattern[j][k].cumulG / pattern[j][k].count;
					unsigned char finalB = pattern[j][k].cumulB / pattern[j][k].count;

					if (displaycode & DISPLAYSINGLE && pattern[j][k].count != 1) // single인데 count가 1이 아니면
					{
						continue;
					}
					else if (displaycode & DISPLAYOVERLAP && pattern[j][k].count < 2) // overlap인데 count가 2보다 작으면
					{
						continue;
					}
					// displaymode가 0이면 그냥 수행
					if (pattern[j][k].count == 1)
						cvRectangle(result, CvPoint((j*GRIDSIZE), k*GRIDSIZE), CvPoint((j*GRIDSIZE) + GRIDSIZE - 1, k*GRIDSIZE + GRIDSIZE - 1), CvScalar(finalB, finalG, finalR), CV_FILLED);
					else if (pattern[j][k].count == 2)
						cvRectangle(result, CvPoint((j*GRIDSIZE), k*GRIDSIZE), CvPoint((j*GRIDSIZE) + GRIDSIZE - 1, k*GRIDSIZE + GRIDSIZE - 1), CvScalar(0, 255, 255), CV_FILLED);
					else if (pattern[j][k].count >= 3)
						cvRectangle(result, CvPoint((j*GRIDSIZE), k*GRIDSIZE), CvPoint((j*GRIDSIZE) + GRIDSIZE - 1, k*GRIDSIZE + GRIDSIZE - 1), CvScalar(255, 255, 0), CV_FILLED);
				}
			}
		}
	}



	for (int i = 0; i < result->width / GRIDSIZE; i++)
	{
		for (int j = 0; j < result->height / GRIDSIZE; j++)
		{
			if (m_program_mode == MODE_DRAWING)
			{
				if (displaycode & DISPLAYBACKGROUNDDRAWING)
				{
					for (int a = 0; a < NUMOFLAYER; a++)
					{
						if (bGrid[i][j].isobstacle[a] == true)
						{
							COLOR coll = C_OBSTACLE[a];
							coll = COLOR((255 * 3 + coll.R) / 4, (255 * 3 + coll.G) / 4, (255 * 3 + coll.B) / 4);
							CvScalar col = CvScalar(coll.B, coll.G, coll.R);
							cvRectangle(result, CvPoint(i*GRIDSIZE, j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), col, CV_FILLED);
						}
					}
					if (bGrid[i][j].iswall == true)
					{
						COLOR coll = C_WALL;
						coll = COLOR((255 * 3 + coll.R) / 4, (255 * 3 + coll.G) / 4, (255 * 3 + coll.B) / 4);
						CvScalar col = CvScalar(coll.B, coll.G, coll.R);
						cvRectangle(result, CvPoint(i*GRIDSIZE, j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), col, CV_FILLED);
					}
					if (bGrid[i][j].isstair == true)
					{
						COLOR coll = C_STAIR;
						coll = COLOR((255 * 3 + coll.R) / 4, (255 * 3 + coll.G) / 4, (255 * 3 + coll.B) / 4);
						CvScalar col = CvScalar(coll.B, coll.G, coll.R);
						cvRectangle(result, CvPoint(i*GRIDSIZE, j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), col, CV_FILLED);
					}
					if (bGrid[i][j].ispath == true)
					{
						COLOR coll = C_PATH;
						coll = COLOR((255 * 3 + coll.R) / 4, (255 * 3 + coll.G) / 4, (255 * 3 + coll.B) / 4);
						CvScalar col = CvScalar(coll.B, coll.G, coll.R);
						cvRectangle(result, CvPoint(i*GRIDSIZE, j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), col, CV_FILLED);
					}
					if (bGrid[i][j].isequipment == true)
					{
						COLOR coll = C_EQUIPMENT;
						coll = COLOR((255 * 3 + coll.R) / 4, (255 * 3 + coll.G) / 4, (255 * 3 + coll.B) / 4);
						CvScalar col = CvScalar(coll.B, coll.G, coll.R);
						cvRectangle(result, CvPoint(i*GRIDSIZE, j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), col, CV_FILLED);
					}
				}
			}


			for (int a = 0; a < NUMOFLAYER; a++)
			{
				if (Grid[i][j].isobstacle[a] == true)
				{
					CvScalar col;
					if (a == m_currentlayer)
					{
						col = CvScalar(C_OBSTACLE[a].B, C_OBSTACLE[a].G, C_OBSTACLE[a].R);
							cvRectangle(result, CvPoint(i*GRIDSIZE, j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), col, CV_FILLED);
					}
					else
					{
						col = CvScalar((255 * 3 + C_OBSTACLE[a].B) / 4, (255 * 3 + C_OBSTACLE[a].G) / 4, (255 * 3 + C_OBSTACLE[a].R) / 4);
							cvRectangle(result, CvPoint(i*GRIDSIZE, j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), col, CV_FILLED);
					}
				}
			}






			if (Grid[i][j].isstair == true)
			{
				int SIZE = GRIDSIZE;
				if (Grid[i][j].stairindex == m_highlighted_stair)
				{
					SIZE = GRIDSIZE * 2;
					int OUTLINESIZE = SIZE + 2;
					cvRectangle(result, CvPoint((i*GRIDSIZE) - 1, j*GRIDSIZE - 1), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1 + 1, j*GRIDSIZE + GRIDSIZE - 1 + 1), CvScalar(0, 0, 0), CV_FILLED);
					cvRectangle(result, CvPoint((i*GRIDSIZE), j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), CvScalar(C_STAIR.B, C_STAIR.G, C_STAIR.R), CV_FILLED);

				}
				else
					cvRectangle(result, CvPoint((i*GRIDSIZE - SIZE / 2), j*GRIDSIZE - SIZE / 2), CvPoint((i*GRIDSIZE) + SIZE / 2, j*GRIDSIZE + SIZE / 2), CvScalar(C_STAIR.B, C_STAIR.G, C_STAIR.R), CV_FILLED);
			}
				if (Grid[i][j].ispath == true)
					cvRectangle(result, CvPoint((i*GRIDSIZE), j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), CvScalar(C_PATH.B, C_PATH.G, C_PATH.R), CV_FILLED);
			if (Grid[i][j].isequipment == true)
				cvRectangle(result, CvPoint((i*GRIDSIZE), j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), CvScalar(C_EQUIPMENT.B, C_EQUIPMENT.G, C_EQUIPMENT.R), CV_FILLED);
			if (Grid[i][j].iswall == true)
				cvRectangle(result, CvPoint((i*GRIDSIZE), j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), CvScalar(C_WALL.B, C_WALL.G, C_WALL.R), CV_FILLED);

			if (displaycode & DISPLAYMISSINGLINE)
			{
				if (Grid[i][j].lossed == true)
				{
					int size = GRIDSIZE * 2;
					cvCircle(result, CvPoint(i*GRIDSIZE + GRIDSIZE / 2, j*GRIDSIZE + GRIDSIZE / 2), size * 2, CvScalar(0, 0, 0), GRIDSIZE);

				}
			}

			/// 축척선 표시
			cvLine(result, CvPoint(m_scaleline.x1, m_scaleline.y), CvPoint(m_scaleline.x2, m_scaleline.y), CvScalar(C_SCALE.B, C_SCALE.G, C_SCALE.R), 1);
		}
	}

	/// pattern [i][j][k].count가 중복 횟수





	for (int i = 0; i < possible_boundary.size(); i++)
	{
		for (int k = 0; k < possible_boundary[i].size(); k++)
		{
			pair<ptr, double> &target = possible_boundary[i][k];
			int x = target.first.x;
			int y = target.first.y;
			double weight = target.second;
			int tempcol = 255 * weight;
			unsigned char col = tempcol;
			CvScalar color = CvScalar(col, col, col);
			cvRectangle(result, CvPoint(x*GRIDSIZE, y*GRIDSIZE), CvPoint((x*GRIDSIZE) + GRIDSIZE - 1, y*GRIDSIZE + GRIDSIZE - 1), color, CV_FILLED);

		}
	}


	


	if (displaycode & DISPLAY_DETECTION_CHECK)
	{
		vector<node> target;
		if (displaycode & DISPLAY_DETECTION_STREAMABLE)
			target = m_streamable_point;
		else if (displaycode & DISPLAY_DETECTION_FLOODABLE)
			target = m_floodable_point;
		else if (displaycode & DISPLAY_DETECTION_FEASIBLE)
			target = m_feasible_point;
		else if (displaycode & DISPLAY_DETECTION_PATH)
			target = m_recommended_point;
		else if (displaycode & DISPLAY_DETECTION_DOOR)
			target = m_doors;

		if (displaycode & DISPLAY_DETECTION_PREFERENCE)
		{
			for (int i = 0; i < m_grid.size(); i++)
			{
				for (int j = 0; j < m_grid[i].size(); j++)
				{
					double cost = m_grid[i][j].costweight;
					if (cost == 1)
						continue;
					else
					{
						int tempcol = 255 * cost;
						unsigned char col = tempcol;
						CvScalar color = CvScalar(col, col, col);
						cvRectangle(result, CvPoint(i*GRIDSIZE, j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), color, CV_FILLED);

					}
				}
			}
			goto skip;
		}
		else
		{
			for (int i = 0; i < target.size(); i++)
			{
				if (displaycode & DISPLAY_DETECTION_DOOR)
				{
					cvRectangle(result, CvPoint(target[i].x*GRIDSIZE - GRIDSIZE, target[i].y*GRIDSIZE - GRIDSIZE), CvPoint((target[i].x*GRIDSIZE) + GRIDSIZE - 1 + 2 * GRIDSIZE, target[i].y*GRIDSIZE + GRIDSIZE - 1 + 2 * GRIDSIZE), CvScalar(255, 0, 0), CV_FILLED);
				}
				else
					cvRectangle(result, CvPoint(target[i].x*GRIDSIZE, target[i].y*GRIDSIZE), CvPoint((target[i].x*GRIDSIZE) + GRIDSIZE - 1, target[i].y*GRIDSIZE + GRIDSIZE - 1), CvScalar(127, 127, 127), CV_FILLED);
			}
			goto skip;
		}

	}
skip:

	/*

	for (int i = 0; i < Grid.size(); i++)
	{
	for (int j = 0; j < Grid[i].size(); j++)
	{
	unsigned char col = (255 * Grid[i][j].costweight); // 0~1, 1이면 검정, 0이면 흰색
	cvRectangle(result, CvPoint(i*GRIDSIZE , j*GRIDSIZE), CvPoint((i*GRIDSIZE) + GRIDSIZE - 1, j*GRIDSIZE + GRIDSIZE - 1), CvScalar(col, col, col), CV_FILLED);

	}
	}
	*/



	/// hydrant 표시시 ///

	for (int j = 0; j < m_hydpos.size(); j++)
	{

		int x = m_hydpos[j].x * GRIDSIZE;
		int y = m_hydpos[j].y * GRIDSIZE;
		double dis = m_hydpos[j].length;

		CvScalar color = CvScalar(0, 0, 0);

		int type = m_hydpos[j].type;
		color = CvScalar(C_HYDRANT[type].B, C_HYDRANT[type].G, C_HYDRANT[type].R);


		int size = GRIDSIZE;
		if (m_hydpos[j].highlighted == true)
			size *= 2;

		if (m_hydpos[j].activate == true) // 표시를 활성화한 소화전일때만
		{
			cvCircle(result, CvPoint(x + GRIDSIZE / 2, y + GRIDSIZE / 2), size * 2, color, CV_FILLED);
			if (m_hydpos[j].highlighted == true)
				cvCircle(result, CvPoint(x + GRIDSIZE / 2, y + GRIDSIZE / 2), size * 2 + 2, CvScalar(0, 0, 0), GRIDSIZE);

			if (m_hydpos[j].highlighted == true && displaycode & DISPLAYGUIDELINE)
			{
				Get_guidebox(j, m_hydpos[j].length / DISTANCE_PER_GRID);
				vector<ptr> gdbox = m_hydpos[j].guidebox;
				for (vector<ptr>::iterator it = gdbox.begin(); it != gdbox.end(); it++)
				{
					cvRectangle(result, CvPoint((it->x*GRIDSIZE + GRIDSIZE - 1), it->y*GRIDSIZE + GRIDSIZE - 1), CvPoint((it->x*GRIDSIZE) + GRIDSIZE - 1, it->y*GRIDSIZE + GRIDSIZE /2), CvScalar(C_GUIDELINE.B, C_GUIDELINE.G, C_GUIDELINE.R), CV_FILLED);
				}
			}
		}
		else
			cvCircle(result, CvPoint(x + GRIDSIZE / 2, y + GRIDSIZE / 2), GRIDSIZE * 2, CvScalar(127,127,127), CV_FILLED);
			

	}


	/// 드로잉 모드

	
	/// hydrant text 표시
#define DELTAPOS 15
	if (displaycode & DISPLAYTEXT)
	{
		int num[NUMOFTYPE] = { 0 };
		for (int j = 0; j < m_hydpos.size(); j++)
		{
			int _type = m_hydpos[j].type;

			char s[20];

			switch (_type)
			{
			case TYPE_HYD_0: sprintf_s(s, "[Hydrant%03d]", num[_type]++); break;
			case TYPE_HYD_1: sprintf_s(s, "[Hyd_form%03d]", num[_type]++); break;
			case TYPE_HYD_2: sprintf_s(s, "[Hyd_powder%03d]", num[_type]++);  break;
			case TYPE_HOSEREEL: sprintf_s(s, "[Hosereel%03d]", num[_type]++); break;
			case TYPE_EXTINGUISHER: sprintf_s(s, "[Extinguisher%03d]", num[_type]++); break;
			case TYPE_MONITOR: sprintf_s(s, "[Monitor%03d]", num[_type]++); break;
			default: break;
			}
			if (m_hydpos[j].activate == true)
				cvPutText(result, s, CvPoint(m_hydpos[j].x * GRIDSIZE - DELTAPOS, m_hydpos[j].y * GRIDSIZE - DELTAPOS), &font, CvScalar(0, 255, 0));
			else
				cvPutText(result, s, CvPoint(m_hydpos[j].x * GRIDSIZE - DELTAPOS, m_hydpos[j].y * GRIDSIZE - DELTAPOS), &font, CvScalar(127, 127, 127));
		}
	}
	/// 축척선 텍스트 표시
	char s[100];
	sprintf_s(s, "%.2lf [m]", SCALELENGTH);
	cvPutText(result, s, CvPoint(m_scaleline.x1 + DELTAPOS, m_scaleline.y - DELTAPOS), &font, CvScalar(C_SCALE.B, C_SCALE.G, C_SCALE.R));

	/// 저장하는 부분
	char filepath2[255];
	sprintf_s(filepath2, "%s\\deck%d.png", filepath, ind);
	cvSaveImage(filepath2, result);
	cvReleaseImage(&result);
	return ind;
	//exportimg(result, filepath);

}

void output::exportimg(IplImage *img, const char *filepath)
{
	
}

void output::Get_guidebox( int hydnum, int size)
{
	//double radius = size / DISTANCE_PER_GRID;
	m_hydpos[hydnum].guidebox.clear();
	vector<ptr> gdbox;
	vector<ptr> vec = m_hydpos[hydnum].outline;

	IplImage* temp = cvCloneImage(m_ents);
	for (vector<ptr>::iterator it = vec.begin(); it != vec.end(); it++)
	{
		int x = it->x;
		int y = it->y;
		cvCircle(temp, CvPoint(x*GRIDSIZE + GRIDSIZE / 2, y*GRIDSIZE + GRIDSIZE / 2), size*GRIDSIZE, CvScalar(C_VIRTUAL.B, C_VIRTUAL.G, C_VIRTUAL.R), GRIDSIZE);
	}


	vector<ptr> o = m_hydpos[hydnum].outline;

	int minx = 65535, maxx = 0, miny = 65535, maxy = 0;

	//vector<ptr> enlarged;
	// bounding box를 얻음, 박스 사이즈는 size
	for (vector<ptr>::iterator it = o.begin(); it != o.end(); it++)
	{
		if (it->x < minx)
			minx = it->x;
		if (it->x > maxx)
			maxx = it->x;
		if (it->y < miny)
			miny = it->y;
		if (it->y > maxy)
			maxy = it->y;
	}

	for (int i = (minx - size)*GRIDSIZE; i <= (maxx + size + 2)*GRIDSIZE; i += GRIDSIZE)
	{
		int first = 0;
		bool firstdetected = 0;
		bool previously_detected = 0;
		int last = 0;
		for (int j = (miny - size)*GRIDSIZE; j <= (maxy + size + 2)*GRIDSIZE; j += GRIDSIZE)
		{
			COLOR col = getcolor(temp, i, j);
			if (col == COLOR(C_VIRTUAL.R, C_VIRTUAL.G, C_VIRTUAL.B) && !(col == COLOR(2, 2, 2))) // 기준색을 찾으면
			{
				firstdetected = 1;
				if (previously_detected == false)
				{
					previously_detected = true;
					gdbox.push_back(ptr((i / GRIDSIZE), (j / GRIDSIZE)));
				}

				last = j;
			}
			else if (previously_detected == true)
			{
				previously_detected = false;
				gdbox.push_back(ptr((i / GRIDSIZE), (j / GRIDSIZE)));
			}
		}
	}

	for (int j = (miny - size)*GRIDSIZE; j <= (maxy + size + 2)*GRIDSIZE; j += GRIDSIZE)
	{
		int first = 0;
		bool firstdetected = 0;
		bool previously_detected = 0;
		int last = 0;
		for (int i = (minx - size)*GRIDSIZE; i <= (maxx + size + 2)*GRIDSIZE; i += GRIDSIZE)
		{
			COLOR col = getcolor(temp, i, j);
			if (col == COLOR(C_VIRTUAL.R, C_VIRTUAL.G, C_VIRTUAL.B) && !(col == COLOR(2, 2, 2)))
			{
				firstdetected = 1;
				if (previously_detected == false)
				{
					previously_detected = true;
					gdbox.push_back(ptr((i / GRIDSIZE), (j / GRIDSIZE)));
				}

				last = i;
			}
			else if (previously_detected == true)
			{
				previously_detected = false;
				gdbox.push_back(ptr((i / GRIDSIZE), (j / GRIDSIZE)));
			}
		}
	}


	cvReleaseImage(&temp);


	m_hydpos[hydnum].guidebox = gdbox;
}

