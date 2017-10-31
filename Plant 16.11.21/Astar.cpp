#include "stdafx.h"

#include <iostream>
#include <algorithm>
#include <list>
#include <queue>

#include "config.h"
#include "grid.h"

#include "Mainfrm.h"
#include "ChildFrm.h"
#include "TESTOCCDoc.h"
#include "TESTOCCView.h"
#include "TESTOCC.h"
using namespace std;


#define HOWLONGSLEEP 100 // 갱신 주기

/// map 전역변수 위치
grid ***map = NULL;// map 전역변수
 
point coord_to_grid(double x, double y, double z)
{
	point result;

	double delta = GRIDSIZE;

	result.x = (x - MIN_X_COORD) / delta;
	result.y = (y - MIN_Y_COORD) / delta;
	result.z = (z - MIN_Z_COORD) / delta;

	return result;
}

point_d grid_to_coord(int x, int y, int z)
{
	double xstart = MIN_X_COORD;
	double ystart = MIN_Y_COORD;
	double zstart = MIN_Z_COORD;
	
	double delta = GRIDSIZE;

	point_d p;
	p.x = xstart + (x*delta);
	p.y = ystart + (y*delta);
	p.z = zstart + (z*delta);

	return p;
}


vector<searchcoord> determine_searchrange()					/// 역시 이게 이상해
{
	vector<searchcoord> coords;
	double xbias = MIN_X_COORD;
	double ybias = MIN_Y_COORD;
	double zbias = MIN_Z_COORD;
	double delta = GRIDSIZE;
	int xstart, ystart, zstart, xend, yend, zend;

	static int count = 0;
	
	for (int i = 0; i < boxes.size(); i++)
	{

		if (boxes[i].isOBB) // OBB이면 out 좌표를 사용
		{
			xstart = (boxes[i].out_xmin - xbias) / delta;
			xend = (boxes[i].out_xmax - xbias) / delta+1;
			ystart = (boxes[i].out_ymin - ybias) / delta;
			yend = (boxes[i].out_ymax - ybias) / delta+1;
			zstart = (boxes[i].out_zmin - zbias) / delta;
			zend = (boxes[i].out_zmax - zbias) / delta+1;
		}
		else
		{
			xstart = (boxes[i].Xmin - xbias) / delta;
			xend = (boxes[i].Xmax - xbias) / delta+1;
			ystart = (boxes[i].Ymin - ybias) / delta;
			yend = (boxes[i].Ymax - ybias) / delta+1;
			zstart = (boxes[i].Zmin - zbias) / delta;
			zend = (boxes[i].Zmax - zbias) / delta+1;
		}
///		printf("<searchrange> xst : %d, xend : %d, yst : %d, yend : %d, zst : %d, znd : %d\n", xstart, xend, ystart, yend, zstart, zend);

		int count = 0;
		for (int j = xstart; j <= xend; j++)
		{
			for (int k = ystart; k <= yend; k++)
			{
				for (int l = zstart; l <= zend; l++)
				{
					count++;

					coords.push_back(searchcoord(j, k, l));
				}
			}
		}
//		printf("\ncheckpoint3. count : %d, total : %d  \n", count, (xend - xstart + 1)*(yend - ystart + 1)*(zend - zstart) + 1);
	}
	

	return coords;
}



void grid_init()					// 파이프의 normal 영역 확보까지 수행.
{
	static int total_x = 0;
	static int total_y = 0;
	static int total_z = 0;

	if (map) // 이미 할당되어 있으면 해제
	{
		for (int x = 0; x < total_x; x++)
		{
			for (int y = 0; y < total_y; y++)
			{
				delete[] map[x][y];
			}
		}

		for (int x = 0; x < total_x; x++)
		{
			delete[] map[x];
		}
		delete[] map;
		map = NULL;
	}

	double delta = GRIDSIZE;
	total_x = (MAX_X_COORD - MIN_X_COORD) / delta;
	total_y = (MAX_Y_COORD - MIN_Y_COORD) / delta;
	total_z = (MAX_Z_COORD - MIN_Z_COORD) / delta;

	map = new grid **[total_x];
	for (int x = 0; x < total_x; x++)
	{
		map[x] = new grid *[total_y];
		for (int y = 0; y < total_y; y++)
		{
			map[x][y] = new grid[total_z];
		}
	}

	/// 외곽 부분 벽 처리
	for (int x = 0; x < total_x; x++)
	{
		for (int y = 0; y < total_y; y++)
		{
			map[x][y][0].closed = 1;
			map[x][y][total_z - 1].closed = 1;
		}
	}

	for (int x = 0; x < total_x; x++)
	{
		for (int z = 0; z < total_z; z++)
		{
			map[x][0][z].closed = 1;
			map[x][total_y-1][z].closed = 1;
		}
	}
	for (int z = 0; z < total_z; z++)
	{
		for (int y = 0; y < total_y; y++)
		{
			map[0][y][z].closed = 1;
			map[total_x-1][y][z].closed = 1;
		}
	}



	/// 여기서부터 초기 장애물 인식

	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CTESTOCCView* pView = (CTESTOCCView*)pChild->GetActiveView();


//	pView->Collect_BB(); // 먼저 바운딩박스 데이터부터 모아야함.

	vector<searchcoord> searchrange = determine_searchrange();   // 그리고 탐색이 필요한 좌표들을 모음. 여기까지 정상
	for (int i = 0; i < searchrange.size(); i++)
	{

		int x, y, z;
		x = searchrange[i].x;
		y = searchrange[i].y;
		z = searchrange[i].z;
		if (pView->Collision_box_and_grid(x,y,z))    // x,y,z에서 많은 중복이 발생.
		{

			for (int j = -RANGEOFOBSTACLE; j <= RANGEOFOBSTACLE; j++)    // 설정한 주위 N마스만큼 장애물 취급
			{
				for (int k = -RANGEOFOBSTACLE; k <= RANGEOFOBSTACLE; k++)
				{
					for (int l = -RANGEOFOBSTACLE; l <= RANGEOFOBSTACLE; l++)
					{
					

						map[x + j][y + k][z + l].close();				// 설정한 주위 N마스만큼 장애물 취급
						map[x + j][y + k][z + l].obstacle = 1;
						// 박스 visualize



					}
				}
			}
			
		}
	}


	/// 파이프의 정확한 normal을 알고 normal 영역 확보   - 여기서는 2그리드정도 + 장애물취급 영역이 클수록 더 커짐
	for (int i = 0; i < pipes.size(); i++)
	{
		double x, y, z;
		gp_Dir dir = pipes[i].normal;
		x = pipes[i].x + dir.X() * (WEIGHT_OF_OFFSET + RANGEOFOBSTACLE) * GRIDSIZE;
		y = pipes[i].y + dir.Y() * (WEIGHT_OF_OFFSET + RANGEOFOBSTACLE) * GRIDSIZE;
		z = pipes[i].z + dir.Z() * (WEIGHT_OF_OFFSET + RANGEOFOBSTACLE) * GRIDSIZE;
		point pt = coord_to_grid(x, y, z);
		if (map[pt.x][pt.y][pt.z].closed == 1) // 막힌길이면
		{
			pipes[i].normal.SetX(-dir.X());   // 방향벡터를 반대로 설정
			pipes[i].normal.SetY(-dir.Y());
			pipes[i].normal.SetZ(-dir.Z());
		}
	}


	/// 디버그용 
	/*
	for (int x = 0; x < total_x; x++)
	{
		for (int y = 0; y < total_y; y++)
		{
			for (int z = 0; z < total_z; z++)
			{
				if (map[x][y][z].obstacle == 1)
				{
					if (map[x - 1][y][z].obstacle * map[x + 1][y][z].obstacle * map[x][y - 1][z].obstacle * map[x][y + 1][z].obstacle * map[x][y][z + 1].obstacle* map[x][y][z - 1].obstacle == 0)
					{
						point_d p;
						p = grid_to_coord(x, y, z);
						pView->displaybox(p.x, p.y, p.z);

					}
				}
			}
			
		}
	}
	*/
}


gp_Dir FaceNormal(const TopoDS_Face &face)
{
	//	TopoDS::Face()
	// get bounds of face
	Standard_Real umin, umax, vmin, vmax;
	BRepTools::UVBounds(face, umin, umax, vmin, vmax);	// create surface
	Handle(Geom_Surface) surf = BRep_Tool::Surface(face);	// get surface properties
	GeomLProp_SLProps props(surf, umin, vmin, 1, 0.01);	// get surface normal
	gp_Dir norm = props.Normal();	// check orientation
	if (face.Orientation() == TopAbs_REVERSED) norm.Reverse();
	return norm;
}

/*
void gatherpipe()    /// 파이프 정보를 새로 받아오는 함수
{
	pipes.clear();


	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CTESTOCCView* pView = (CTESTOCCView*)pChild->GetActiveView();
	CTESTOCCDoc *pDoc = (CTESTOCCDoc *)pChild->GetActiveDocument();
	const Handle(AIS_InteractiveContext) aCon = pDoc->myAISContext;

	pDoc->Selection(SELECT_PART);
	aCon->SetAutoActivateSelection(Standard_False);
	aCon->Select(-2000, -2000, 2000, 2000, pView->myView);  // -2000에서 2000범위까지
	aCon->UnhilightSelected();

	Handle(TDataStd_Name) N;
	CString string;
	///

	vector<int> filenum;
	vector<int> index;

	for (int i = 0; i < pDoc->num_of_file; i++)
	{
		for (int j = 0; j < pDoc->numofpart[i]; j++)
		{
			pDoc->Parts[i][j].FindAttribute(TDataStd_Name::GetID(), N);
			TCollection_ExtendedString name = N->Get();
			string = (wchar_t*)name.ToExtString();   // 강제로 CString으로 변환
			
			if (string.Left(4).CompareNoCase(_T("pipe")) == 0)   // 같으면 리턴값은 0이다. pipe라는 문자가 들어가 있음.
			{
				printf("pipe detect\n");
				filenum.push_back(i);
				index.push_back(j);    // 파이프라 명명된 모든 파일번호와 part index를 얻음.

			}
		}
	}
	///

	for (aCon->InitSelected(); aCon->MoreSelected(); aCon->NextSelected())
	{
		for (int i = 0; i < filenum.size(); i++)
		{
			TDF_Label aLabel = pDoc->myAssembly[filenum[i]]->FindShape(aCon->SelectedShape());
			
			if (aLabel == pDoc->Parts[filenum[i]][index[i]])
			{

				TopoDS_Shape selected_shape;
				selected_shape = aCon->SelectedShape();

				/// normal vector 판명하는 부분
				TopoDS_Edge circleEdge;
				gp_Dir normal;
				for (TopExp_Explorer ex(selected_shape, TopAbs_EDGE); ex.More(); ex.Next())
				{
					TopoDS_Edge currentEdge = TopoDS::Edge(ex.Current());
					BRepAdaptor_Curve curve(currentEdge);
					if (curve.GetType() == GeomAbs_Circle)
					{
						gp_Circ circ = curve.Circle();
						gp_Cylinder cylin;
						
						
						normal = circ.Axis().Direction();  // 노멀 벡터 얻음.
						break;
					}
				}
				 
				printf("\nnormal is -> x:%lf , y:%lf,   z:%lf\n", normal.X(), normal.Y(), normal.Z());
				///
				
				
				
				point_d pt = pView->GetAbsCoord(selected_shape);
				pipes.push_back(Pipe_Info(aLabel, filenum[i], index[i], pt.x, pt.y, pt.z, normal));
			//	printf("coord is -> x:%lf , y:%lf,   z:%lf\n", pt.x, pt.y, pt.z);
				
				
				
				TopoDS_Shape sh_box = pView->CreateOBB(selected_shape).shape; // OBB 생성
				static AIS_Shape *selected = NULL;
				selected = new AIS_Shape(sh_box);


				selected->SetTransparency(0.8); // 투명도 설정
				selected->SetColor(Quantity_Color(0, 0, 1, Quantity_TOC_RGB)); // 색상 설정
				aCon->Display(selected);
				
				break;
			}
		}
		
	}

}
*/

DIRECTION pre_dir = NONE;


void go_backward(int &cur_x, int &cur_y, int &cur_z, double &cur_cost, vector<path_data> &path, int init_x, int init_y, int init_z, int final_x, int final_y, int final_z, vector<grid> &vec)
{
	if (path.empty())
		return;

	/// 이미 vec은 sort되었다 가정
	map[cur_x][cur_y][cur_z].close(); // 현재 위치는 닫아버림.

	int level = vec.front().level;		// 가장 H값이 작은 지점에서 level을 취함
	map[vec.front().x][vec.front().y][vec.front().z].go_here = true;    // 일로 가라고 명령

	vec.erase(vec.begin());				// 정해진 곳을 open list에서 지움
	while (path.size() != level)		// path_data를 깎아나감 (level과 같아질때까지)
	{
		if (path.size() < level)
			break;
		vector<path_data>::iterator it;
		it = path.end();
		it--;
		path.erase(it);

		if (path.empty())
		{
			return;
		}

		vector<grid>::iterator it2;
		it2 = find(vec.begin(), vec.end(), grid(path.back().x, path.back().y, path.back().z));
		if (it2 != vec.end())
			vec.erase(it2);
	}



	cur_x = path.back().x;
	cur_y = path.back().y;
	cur_z = path.back().z;
	cur_cost = path.back().cost;
	pre_dir = path.back().pre_dir;
}



#define WEIGHT_OF_COST 1   // 한 그리드 전진할 때마다 산정하는 코스트

double getH(int start_x, int start_y, int start_z, int end_x, int end_y, int end_z) // 두 점의 거리를 재는 함수
{
	return WEIGHT_OF_COST * sqrt(pow(end_x - start_x, 2) + pow(end_y - start_y, 2) + pow(end_z - start_z, 2));   //유클리디언 거리
//	return abs(start_x - end_x) + abs(start_y - end_y) + abs(start_z - end_z);   // 맨하탄 거리
}






bool function(int ax, int ay, int az, int dest_x, int dest_y, int dest_z, DIRECTION dir, double cur_G, double &min_cost, DIRECTION &min_dir, vector<grid> &vec, vector<path_data> &path, int level)
{
	int x = ax;
	int y = ay;
	int z = az;

	/// 방향에 따른 좌표변화 설정
	double DeltaG = 0;

	switch (dir)		// 방향에 따 목표 speed 산정, 그리고 x,y값의 변화
	{
	case X_POS:	x++;	break;
	case X_NEG:	x--;	break;
	case Y_POS:	y++;	break;
	case Y_NEG:	y--;	break;
	case Z_POS: z++; break;
	case Z_NEG: z--; break;
	default:	break;
	}
	
	if (pre_dir == NONE)
		DeltaG = WEIGHT_OF_COST;
	else if (pre_dir != dir)
		DeltaG = WEIGHT_OF_COST;
		//DeltaG = WEIGHT_OF_COST * 10;
	
	double next_cost = cur_G + DeltaG;
	map[x][y][z].H_value = getH(x, y, z, dest_x, dest_y, dest_z);	/// 이 지점부터 목적지까지 직선으로 가는데 드는 cost를 산출	
	double F_value = next_cost + map[x][y][z].H_value;								/// F값은 G값 + H값

	/*
	vector<path_data>::iterator it;
	it = find(path.begin(), path.end(), grid(x, y, z)); //기존 루트에서 여기를 다시 발견하면
	if (it != path.end()) // 찾으면
	{
		if (map[x][y][z].G_value > next_cost)  // 그리고 더 적은 G값으로 이동할 수 있으면
		{
			min_cost -= map[x][y][z].G_value - next_cost;
			path.back().cost = min_cost;
			map[x][y][z].G_value = next_cost;
			vector<path_data>::iterator it2;
			while (1)
			{
				it2 = path.begin();
				if (it2 == it)
					break;
				path.erase(it2);
			}
			vector<path_data> temp = path;
			while (1)
			{
				it2 = temp.end(); it2--;
				min_route.push_front(*it2);
				temp.erase(it2);
				if (temp.size() == 0)
					break;
			}
			return false;
		}
	}
	*/

	if (map[x][y][z].go_here == true)
	{

		map[x][y][z].go_here = false;
		min_cost = 0.000001;
		map[x][y][z].setlevel(level);
		min_dir = dir;
		return false;
	}
	
	if (map[x][y][z].closed == 1) // 닫혀있음. 계산 불필요
	{
		return true;
	}

	



	else  // 닫힌 곳이 아닐 때
	{

		///
		 
		if (map[x][y][z].isopen() == true)											// open된 곳이라면 G값(cost)가 갱신될 만 한지 판단한다.
		{
			if (map[x][y][z].G_value + 0.001 < next_cost)								/// 기존보다 더 많은 cost를 필요로 하면
			{
				// 현재는 아무것도 안함
				return true;													// 닫힌 곳 취급
			}
			else /// 그 외엔
			{
				vector<grid>::iterator it;
				it = find(vec.begin(), vec.end(), grid(x, y, z));     // open info 에서 현재 위치에 해당하는 지점을 찾아 삭제한다.
				if (it != vec.end())
					vec.erase(it);

				map[x][y][z].setG(next_cost);										// 기존보다 적은 cost를 필요로 하면 그 값을 G_value에 대입
				map[x][y][z].setF(F_value);
				map[x][y][z].setXYZ(x, y, z);
				map[x][y][z].setlevel(level);
				vec.push_back(map[x][y][z]);							// 파라미터를 기입하고 새롭게 open list에 추가함.

				if (min_cost == 0)
					min_cost = F_value;
				else
				{
					if (min_cost > F_value)
					{
						min_cost = F_value;
						min_dir = dir;
					}
				}
			}
		}
		else // open상태가 아닌 곳이면
		{
			map[x][y][z].open = 1;
			map[x][y][z].setG(next_cost);
			map[x][y][z].setF(F_value);
			map[x][y][z].setXYZ(x, y, z);
			map[x][y][z].setlevel(level);

			vec.push_back(map[x][y][z]);			// open list에 넣는다.

			if (min_cost == 0)
				min_cost = F_value;
			else
			{
				if (min_cost > F_value)
				{
					min_cost = F_value;
					min_dir = dir;
				}
			}
		}
	}
	return false;
}



//void getoutput(output *out, list<path_data> &total_path);

void Init(vector<path_data> &path, int xpos, int ypos, int zpos)
{
	map[xpos][ypos][zpos].close();				   // 시작점을 닫음.
	path.push_back(path_data(xpos, ypos, zpos, 0, NONE)); // 시작점의 좌표와 코스트를 list에 push
}

TopoDS_Shape getcylinder(point_d p1, point_d p2, double radius)
{
	double x1 = p1.x; 
	double y1 = p1.y;
	double z1 = p1.z;
	double x2 = p2.x;
	double y2 = p2.y;
	double z2 = p2.z;

	gp_Pnt P1(x1, y1, z1);
	gp_Pnt P2(x2, y2, z2);
	Handle(Geom_TrimmedCurve) Seg1 = GC_MakeSegment(P1, P2);
	TopoDS_Edge Edge1 = BRepBuilderAPI_MakeEdge(Seg1);
	gp_Dir dir = gp_Dir(x2 - x1, y2 - y1, z2 - z1);
	gp_Ax2 axis = gp_Ax2(P1, dir);
	double height = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
	TopoDS_Shape shape = BRepPrimAPI_MakeCylinder(axis, radius, height);
	return shape;
}


vector<path_data> sharpening(vector<path_data> path) // 직각화 알고리즘
{


	
	/// 직선 검출
	vector<path_data>::iterator it;
	it = path.begin();
	int x1, x2, x3, y1, y2, y3, z1, z2, z3,x4,y4,z4;
	int deltax = 0, deltay = 0, deltaz = 0;
	
	while (1)
	{
		if ((it + 1) == path.end() || (it + 2) == path.end())
			break;
		

		x1 = it->x;   y1 = it->y;   z1 = it->z;
		x2 = (it + 1)->x;   y2 = (it + 1)->y;   z2 = (it + 1)->z;
		x3 = (it + 2)->x;   y3 = (it + 2)->y;   z3 = (it + 2)->z;

		if (((double)((x1 + x3) / 2.0) == x2) + ((double)((y1 + y3) / 2.0) == y2) + ((double)((z1 + z3) / 2.0) == z2) >= 2) // 일직선이면
		{
			map[(it + 1)->x][(it + 1)->y][(it + 1)->z].closed = 0;  //닫히지 않은 상태로 만든다.
			path.erase(it + 1); // 중간부분을 지운다.
			
			continue;
		}
		else
		{
			it++;   // 일직선이 아니면 다음 iteration으로
		}
	}
	
	/// 직선 검출 완료

	/// 핸들 가져오기
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CTESTOCCView* pView = (CTESTOCCView*)pChild->GetActiveView();
	///

	/// 직각화 시작

	it = path.begin();

	int index = 0;
	while (1)
	{
		bool flag = 0;

		if (index + 3 >= path.size())			// 가장 끝점과 사각형을 만들어보거나 말거나
			break;
		for (int i = path.size() - 1; i >= index+3; i--)
		{
			deltax = 0;
			deltay = 0;
			deltaz = 0;

			
			x1 = path[index].x;
			y1 = path[index].y;
			z1 = path[index].z;
			x4 = path[i].x;
			y4 = path[i].y;
			z4 = path[i].z;

			/// 3축으로 모두 움직였거나, 1축으로만 움직인경우
			if (x1 != x4) deltax = 1;
			if (y1 != y4) deltay = 1;
			if (z1 != z4) deltaz = 1;

			int type = deltax + deltay + deltaz;

			if (type <= 1)  // 2축 사각형이 성립이 안되면 다음반복으로
			{
				continue;			// 다음 iteration으로
			}

			else if (type == 2)
			{
				int upper_collide = 0;
				int lower_collide = 0;
				point upper_grid;
				point lower_grid;
				if (deltax && deltay)
				{
					for (int i = x1; i != x4 + abs(x4 - x1) / (x4 - x1); i += abs(x4 - x1) / (x4 - x1))
					{
						upper_collide += map[i][y4][z4].obstacle;
						lower_collide += map[i][y1][z4].obstacle;
					}
					for (int j = y1; j != y4 + abs(y4 - y1) / (y4 - y1); j += abs(y4 - y1) / (y4 - y1))
					{
						upper_collide += map[x1][j][z4].obstacle;
						lower_collide += map[x4][j][z4].obstacle;
					}
					upper_grid = point(x1, y4, z4);
					lower_grid = point(x4, y1, z4);
				}
				else if (deltay && deltaz)
				{
					for (int i = z1; i != z4 + abs(z4 - z1) / (z4 - z1); i += abs(z4 - z1) / (z4 - z1))
					{
						upper_collide += map[x4][y4][i].obstacle;
						lower_collide += map[x4][y1][i].obstacle;
					}
					for (int j = y1; j != y4 + abs(y4 - y1) / (y4 - y1); j += abs(y4 - y1) / (y4 - y1))
					{
						upper_collide += map[x4][j][z1].obstacle;
						lower_collide += map[x4][j][z4].obstacle;
					}

					upper_grid = point(x4, y4, z1);
					lower_grid = point(x4, y1, z4);
				}
				else if (deltax && deltaz)
				{
					for (int i = z1; i != z4 + abs(z4 - z1) / (z4 - z1); i += abs(z4 - z1) / (z4 - z1))
					{
						upper_collide += map[x4][y4][i].obstacle;
						lower_collide += map[x1][y4][i].obstacle;
					}
					for (int j = x1; j != x4 + abs(x4 - x1) / (x4 - x1); j += abs(x4 - x1) / (x4 - x1))
					{
						upper_collide += map[j][y4][z1].obstacle;
						lower_collide += map[j][y4][z4].obstacle;
					}
					upper_grid = point(x4, y4, z1);
					lower_grid = point(x1, y4, z4);
				}

	//			printf("upper : %d, lower: %d, \n", upper_collide, lower_collide);

				if (upper_collide)
				{
					if (lower_collide)
					{
						continue;				// 보정안함
					}

					/// 시작점과 끝점 사이의 모든 좌표를 지워버림.

					for (int j = i - 1; j > index; j--)
					{
						map[path[j].x][path[j].y][path[j].z].closed = 0;  //닫히지 않은 상태로 만든다.
						vector<path_data>::iterator it = path.begin();
						it += j;		// path[j]를 취함
						path.erase(it);
					}

					vector<path_data>::iterator it = path.begin();
					it += index + 1;  // path[index+1]를 취함

					path.insert(it, path_data(lower_grid.x, lower_grid.y, lower_grid.z, 0, NONE));
					break;
				}
				else
				{
					for (int j = i - 1; j > index; j--)
					{
						map[path[j].x][path[j].y][path[j].z].closed = 0;  //닫히지 않은 상태로 만든다.
						vector<path_data>::iterator it = path.begin();
						it += j;		// path[j]를 취함
						path.erase(it);
					}

					vector<path_data>::iterator it = path.begin();
					it += index + 1;  // path[index+1]를 취함

					path.insert(it, path_data(upper_grid.x, upper_grid.y, upper_grid.z, 0, NONE));
					break;
				}
			}

			else if (type == 3) // 6방향 조사
			{
				int col_route[6] = { 0 };
				

				point opt_route[6];
				point opt_route2[6];
				//route 1
				for (int x = x1; x != x4 + abs(x4 - x1) / (x4 - x1); x += abs(x4 - x1) / (x4 - x1))
				{
					col_route[0] += map[x][y1][z1].obstacle; // x-y-z
					col_route[1] += map[x][y1][z1].obstacle; // x-z-y
					col_route[2] += map[x][y4][z1].obstacle; // y-x-z
					col_route[3] += map[x][y4][z4].obstacle; // y-z-x
					col_route[4] += map[x][y1][z4].obstacle; // z-x-y
					col_route[5] += map[x][y4][z4].obstacle; // z-y-x
				}
				for (int y = y1; y != y4 + abs(y4 - y1) / (y4 - y1); y+= abs(y4 - y1) / (y4 - y1))
				{
					col_route[0] += map[x4][y][z1].obstacle; // x-y-z
					col_route[1] += map[x4][y][z4].obstacle; // x-z-y
					col_route[2] += map[x1][y][z1].obstacle; // y-x-z
					col_route[3] += map[x1][y][z1].obstacle; // y-z-x
					col_route[4] += map[x4][y][z4].obstacle; // z-x-y
					col_route[5] += map[x1][y][z4].obstacle; // z-y-x
				}
				for (int z = z1; z!= z4 + abs(z4 - z1) / (z4 - z1); z+= abs(z4 - z1) / (z4 - z1))
				{
					col_route[0] += map[x4][y4][z].obstacle; // x-y-z
					col_route[1] += map[x4][y1][z].obstacle; // x-z-y
					col_route[2] += map[x4][y4][z].obstacle; // y-x-z
					col_route[3] += map[x1][y4][z].obstacle; // y-z-x
					col_route[4] += map[x1][y1][z].obstacle; // z-x-y
					col_route[5] += map[x1][y1][z].obstacle; // z-y-x
				}

				opt_route[0] = point(x4, y1, z1);
				opt_route2[0] = point(x4, y4, z1);
				opt_route[1] = point(x4, y1, z1);
				opt_route2[1] = point(x4, y1, z4);
				opt_route[2] = point(x1, y4, z1);
				opt_route2[2] = point(x4, y4, z1);
				opt_route[3] = point(x1, y4, z1);
				opt_route2[3] = point(x1, y4, z4);
				opt_route[4] = point(x1, y1, z4);
				opt_route2[4] = point(x4, y1, z4);
				opt_route[5] = point(x1, y1, z4);
				opt_route2[5] = point(x1, y4, z4);

				



				for (int k = 0; k < 6; k++)
				{
					if (col_route[k] == 0) // k번째 루트에 충돌이 없으면 (이 길을 선택)
					{
						printf("\n\ntest\n\n");
						for (int j = i - 1; j > index; j--)  // 중간에 있는 값들을 다 없앰
						{
							map[path[j].x][path[j].y][path[j].z].closed = 0;  //닫히지 않은 상태로 만든다.
							vector<path_data>::iterator it = path.begin();
							it += j;		// path[j]를 취함
							path.erase(it);
						}

						vector<path_data>::iterator it = path.begin();
						it += index;  // path[index+1]를 취함

						path.insert(it+1, path_data(opt_route[k].x, opt_route[k].y, opt_route[k].z, 0, NONE));

						//it = path.begin();
						
						path.insert(it+2, path_data(opt_route2[k].x, opt_route2[k].y, opt_route2[k].z, 0, NONE));

						
						flag = 1;
						break;
					}
				}
				if (flag)
					break;
			}
		}
		index++;
		/// 즉, 2축으로 움직인 경우만 다룬다.
		

		/// 그 외에는 사각형을 그린다.  (closed check, 충돌 방법중에 충돌방법을 사용)
	}



	
	/// 직각화가 완료되면 각 경로들을 close 및 obstacle 처리 한다.

	for (int i = 0; i < path.size() - 1; i++)
	{
//		printf("test\n");
		if (i + 1 == path.size()) // 인덱스 초과 방지
			break;
		/// path[i]와 path[i+1]간의 모든 점들을 closed, obstacle 처리함
		int x1 = min(path[i].x, path[i + 1].x);
		int x2 = max(path[i].x, path[i + 1].x);
		int y1 = max(path[i].y, path[i + 1].y);
		int y2 = max(path[i].y, path[i + 1].y);
		int z1 = max(path[i].z, path[i + 1].z);
		int z2 = max(path[i].z, path[i + 1].z);
		for (int j = x1; j <= x2; j++)
		{
			for (int k = y1; k <= y2; k++)
			{
				for (int m = z1; m <= z2; m++)
				{
					map[j][k][m].closed = 1;
					map[j][k][m].obstacle = 1;
				}
			}
		}
	}


	/// 장애물 확장 (설정한 범위만큼)
	for (int i = 0; i < path.size()-1; i++)
	{
		int ax1, ay1, az1, ax2, ay2, az2;
		ax1 = path[i].x;   ay1 = path[i].y;   az1 = path[i].z;
		ax2 = path[i + 1].x; ay2 = path[i + 1].y; az2 = path[i + 1].z;
		x1 = min(ax1, ax2);
		x2 = max(ax1, ax2);
		y1 = min(ay1, ay2);
		y2 = max(ay1, ay2);
		z1 = min(az1, az2);
		z2 = max(az1, az2);

		for (int x = x1; x <= x2; x++)
		{
			for (int y = y1; y <= y2; y++)
			{
				for (int z = z1; z <= z2; z++)
				{
					for (int c = -RANGEOFPIPEOBSTACLE; c <= RANGEOFPIPEOBSTACLE; c++)
					{
						for (int d = -RANGEOFPIPEOBSTACLE; d <= RANGEOFPIPEOBSTACLE; d++)
						{
							for (int e = -RANGEOFPIPEOBSTACLE; e <= RANGEOFPIPEOBSTACLE; e++)
							{
								map[x + c][y + d][z + e].closed = 1;
								map[x + c][y + d][z + e].obstacle = 1;
							}
						}
					}
				}
			}
		}
	}
	



	return path;
}






vector<path_data> pathfinder(point pinit, point pfinal)// 리턴타입은 경로를 담은 structure, 인자는 출발점, 도착점, 경유지에 대한 sturucture
{
	/// 초기 설정 구간 ///////////////////
	int xpos_init = pinit.x;
	int ypos_init = pinit.y;
	int zpos_init = pinit.z;
	int xpos_final = pfinal.x;
	int ypos_final = pfinal.y;
	int zpos_final = pfinal.z;

	int current_x = xpos_init;
	int current_y = ypos_init;
	int current_z = zpos_init;


	map[xpos_final][ypos_final][zpos_final].go_here = 1; /// 목적지를 발견하면 무조건 간다.

	/// 초기 작업 ////////////////
	vector<path_data> total_path; // path_data를 담은 링크드리스트
	vector<path_data> path_info;  // path를 저장할 list를 선언
	Init(path_info, xpos_init, ypos_init, zpos_init);  // 시작점을 push하고, 시작점을 close함
	vector<grid> open_info;
	///


	if (map[xpos_final][ypos_final][zpos_final].closed == 1)
	{
		return path_info;
	}

	/// 디폴트값 설정 //////////////////////
	double min_F = 0;
	double min_deltaG = 0;
	double current_cost = 0;
	DIRECTION min_dir = X_POS;
	///

	
	while (1)
	{
		
		min_F = pow(2, 31);          // 높은값 설정
		min_deltaG = pow(2, 31);

		bool is_fullyclosed =
			function(current_x, current_y, current_z, xpos_final, ypos_final, zpos_final, X_POS, current_cost, min_F, min_dir, open_info, path_info,path_info.size())
			*function(current_x, current_y, current_z, xpos_final, ypos_final, zpos_final, X_NEG, current_cost, min_F, min_dir, open_info, path_info,path_info.size())
			*function(current_x, current_y, current_z, xpos_final, ypos_final, zpos_final, Y_POS, current_cost, min_F, min_dir, open_info, path_info,path_info.size())
			*function(current_x, current_y, current_z, xpos_final, ypos_final, zpos_final, Y_NEG, current_cost, min_F, min_dir, open_info, path_info,path_info.size())
			*function(current_x, current_y, current_z, xpos_final, ypos_final, zpos_final, Z_POS, current_cost, min_F, min_dir, open_info, path_info,path_info.size())
			*function(current_x, current_y, current_z, xpos_final, ypos_final, zpos_final, Z_NEG, current_cost, min_F, min_dir, open_info, path_info,path_info.size());

		if (is_fullyclosed) // 막다른길이면
		{
			printf("check3 : size : %d\n", open_info.size());
			/// open list를 F_value 순서대로 정렬함
			if (!open_info.empty())			// 하나라도 열린 곳이 있으면
			{
				sort(open_info.begin(), open_info.end());    // 열린 곳은 sort한다.  sort는 H_value를 기준으로 한다.
				vector<grid>::iterator it;
				it = find(open_info.begin(), open_info.end(), grid(current_x, current_y, current_z)); // openlist중에 현재 위치를 찾아 지워버린다.
				if (it != open_info.end())
				{
					open_info.erase(it);
				}
			}
			else
			{
				path_info.clear();
				return path_info;
				// 길을 찾을수가 없는경우.
			}

			go_backward(current_x, current_y, current_z, current_cost, path_info, xpos_init, ypos_init, zpos_init, xpos_final, ypos_final, zpos_final, open_info); // 되돌아간다.

			if (path_info.empty())
			{
				return path_info;
				// 길을 찾을수가 없음.
			}

			continue;
		}


		/// 선택된 좌표를 열린목록에서 빼고 닫힌목록에 추가함
		vector<grid>::iterator it;
		it = find(open_info.begin(), open_info.end(), grid(current_x, current_y, current_z));
		if (it != open_info.end())
			open_info.erase(it);
		map[current_x][current_y][current_z].close();
		///


		current_cost += WEIGHT_OF_COST; // 이 둘의 배치 중요.  나중에 수정할 부분


		

		/// 화살표 그리는 부분
		switch (min_dir)
		{
		case X_POS:
			current_x++;
			break;
		case X_NEG:
			current_x--;
			break;
		case Y_POS:
			current_y++;
			break;
		case Y_NEG:
			current_y--;
			break;
		case Z_POS:
			current_z++;
			break;
		case Z_NEG:
			current_z--;
			break;
		default:
			break;
		}
		///
		pre_dir = min_dir;
		path_info.push_back(path_data(current_x, current_y, current_z, current_cost, pre_dir));
		

		/// 디버그용 visualize //
		/*
		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
		CTESTOCCView* pView = (CTESTOCCView*)pChild->GetActiveView();
		point_d pt;
		pt = grid_to_coord(current_x, current_y, current_z);
		pView->displaybox(pt.x, pt.y, pt.z);
		*/
		///
		

		if (current_x == xpos_final && current_y == ypos_final && current_z == zpos_final)
		{
			
			vector<path_data>::iterator it, it2;    // 이 단계에서 경로 데이터를 total_path에 순차적으로 집어넣음.
			while (1)
			{
				it = path_info.begin();
				if (it == path_info.end())
					break;
				it2 = find(total_path.begin(), total_path.end(), path_data(it->x, it->y, it->z, 0, NONE));
				if (it2 == total_path.end())
					total_path.push_back(*it);
				path_info.erase(it);
			}
			open_info.clear();
			break;
		}
	//	printf("Astar check2\n");
	}
	return total_path;
}



