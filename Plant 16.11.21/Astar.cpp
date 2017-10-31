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


#define HOWLONGSLEEP 100 // ���� �ֱ�

/// map �������� ��ġ
grid ***map = NULL;// map ��������
 
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


vector<searchcoord> determine_searchrange()					/// ���� �̰� �̻���
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

		if (boxes[i].isOBB) // OBB�̸� out ��ǥ�� ���
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



void grid_init()					// �������� normal ���� Ȯ������ ����.
{
	static int total_x = 0;
	static int total_y = 0;
	static int total_z = 0;

	if (map) // �̹� �Ҵ�Ǿ� ������ ����
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

	/// �ܰ� �κ� �� ó��
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



	/// ���⼭���� �ʱ� ��ֹ� �ν�

	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CTESTOCCView* pView = (CTESTOCCView*)pChild->GetActiveView();


//	pView->Collect_BB(); // ���� �ٿ���ڽ� �����ͺ��� ��ƾ���.

	vector<searchcoord> searchrange = determine_searchrange();   // �׸��� Ž���� �ʿ��� ��ǥ���� ����. ������� ����
	for (int i = 0; i < searchrange.size(); i++)
	{

		int x, y, z;
		x = searchrange[i].x;
		y = searchrange[i].y;
		z = searchrange[i].z;
		if (pView->Collision_box_and_grid(x,y,z))    // x,y,z���� ���� �ߺ��� �߻�.
		{

			for (int j = -RANGEOFOBSTACLE; j <= RANGEOFOBSTACLE; j++)    // ������ ���� N������ŭ ��ֹ� ���
			{
				for (int k = -RANGEOFOBSTACLE; k <= RANGEOFOBSTACLE; k++)
				{
					for (int l = -RANGEOFOBSTACLE; l <= RANGEOFOBSTACLE; l++)
					{
					

						map[x + j][y + k][z + l].close();				// ������ ���� N������ŭ ��ֹ� ���
						map[x + j][y + k][z + l].obstacle = 1;
						// �ڽ� visualize



					}
				}
			}
			
		}
	}


	/// �������� ��Ȯ�� normal�� �˰� normal ���� Ȯ��   - ���⼭�� 2�׸������� + ��ֹ���� ������ Ŭ���� �� Ŀ��
	for (int i = 0; i < pipes.size(); i++)
	{
		double x, y, z;
		gp_Dir dir = pipes[i].normal;
		x = pipes[i].x + dir.X() * (WEIGHT_OF_OFFSET + RANGEOFOBSTACLE) * GRIDSIZE;
		y = pipes[i].y + dir.Y() * (WEIGHT_OF_OFFSET + RANGEOFOBSTACLE) * GRIDSIZE;
		z = pipes[i].z + dir.Z() * (WEIGHT_OF_OFFSET + RANGEOFOBSTACLE) * GRIDSIZE;
		point pt = coord_to_grid(x, y, z);
		if (map[pt.x][pt.y][pt.z].closed == 1) // �������̸�
		{
			pipes[i].normal.SetX(-dir.X());   // ���⺤�͸� �ݴ�� ����
			pipes[i].normal.SetY(-dir.Y());
			pipes[i].normal.SetZ(-dir.Z());
		}
	}


	/// ����׿� 
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
void gatherpipe()    /// ������ ������ ���� �޾ƿ��� �Լ�
{
	pipes.clear();


	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CTESTOCCView* pView = (CTESTOCCView*)pChild->GetActiveView();
	CTESTOCCDoc *pDoc = (CTESTOCCDoc *)pChild->GetActiveDocument();
	const Handle(AIS_InteractiveContext) aCon = pDoc->myAISContext;

	pDoc->Selection(SELECT_PART);
	aCon->SetAutoActivateSelection(Standard_False);
	aCon->Select(-2000, -2000, 2000, 2000, pView->myView);  // -2000���� 2000��������
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
			string = (wchar_t*)name.ToExtString();   // ������ CString���� ��ȯ
			
			if (string.Left(4).CompareNoCase(_T("pipe")) == 0)   // ������ ���ϰ��� 0�̴�. pipe��� ���ڰ� �� ����.
			{
				printf("pipe detect\n");
				filenum.push_back(i);
				index.push_back(j);    // �������� ���� ��� ���Ϲ�ȣ�� part index�� ����.

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

				/// normal vector �Ǹ��ϴ� �κ�
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
						
						
						normal = circ.Axis().Direction();  // ��� ���� ����.
						break;
					}
				}
				 
				printf("\nnormal is -> x:%lf , y:%lf,   z:%lf\n", normal.X(), normal.Y(), normal.Z());
				///
				
				
				
				point_d pt = pView->GetAbsCoord(selected_shape);
				pipes.push_back(Pipe_Info(aLabel, filenum[i], index[i], pt.x, pt.y, pt.z, normal));
			//	printf("coord is -> x:%lf , y:%lf,   z:%lf\n", pt.x, pt.y, pt.z);
				
				
				
				TopoDS_Shape sh_box = pView->CreateOBB(selected_shape).shape; // OBB ����
				static AIS_Shape *selected = NULL;
				selected = new AIS_Shape(sh_box);


				selected->SetTransparency(0.8); // ���� ����
				selected->SetColor(Quantity_Color(0, 0, 1, Quantity_TOC_RGB)); // ���� ����
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

	/// �̹� vec�� sort�Ǿ��� ����
	map[cur_x][cur_y][cur_z].close(); // ���� ��ġ�� �ݾƹ���.

	int level = vec.front().level;		// ���� H���� ���� �������� level�� ����
	map[vec.front().x][vec.front().y][vec.front().z].go_here = true;    // �Ϸ� ����� ���

	vec.erase(vec.begin());				// ������ ���� open list���� ����
	while (path.size() != level)		// path_data�� ��Ƴ��� (level�� ������������)
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



#define WEIGHT_OF_COST 1   // �� �׸��� ������ ������ �����ϴ� �ڽ�Ʈ

double getH(int start_x, int start_y, int start_z, int end_x, int end_y, int end_z) // �� ���� �Ÿ��� ��� �Լ�
{
	return WEIGHT_OF_COST * sqrt(pow(end_x - start_x, 2) + pow(end_y - start_y, 2) + pow(end_z - start_z, 2));   //��Ŭ����� �Ÿ�
//	return abs(start_x - end_x) + abs(start_y - end_y) + abs(start_z - end_z);   // ����ź �Ÿ�
}






bool function(int ax, int ay, int az, int dest_x, int dest_y, int dest_z, DIRECTION dir, double cur_G, double &min_cost, DIRECTION &min_dir, vector<grid> &vec, vector<path_data> &path, int level)
{
	int x = ax;
	int y = ay;
	int z = az;

	/// ���⿡ ���� ��ǥ��ȭ ����
	double DeltaG = 0;

	switch (dir)		// ���⿡ �� ��ǥ speed ����, �׸��� x,y���� ��ȭ
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
	map[x][y][z].H_value = getH(x, y, z, dest_x, dest_y, dest_z);	/// �� �������� ���������� �������� ���µ� ��� cost�� ����	
	double F_value = next_cost + map[x][y][z].H_value;								/// F���� G�� + H��

	/*
	vector<path_data>::iterator it;
	it = find(path.begin(), path.end(), grid(x, y, z)); //���� ��Ʈ���� ���⸦ �ٽ� �߰��ϸ�
	if (it != path.end()) // ã����
	{
		if (map[x][y][z].G_value > next_cost)  // �׸��� �� ���� G������ �̵��� �� ������
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
	
	if (map[x][y][z].closed == 1) // ��������. ��� ���ʿ�
	{
		return true;
	}

	



	else  // ���� ���� �ƴ� ��
	{

		///
		 
		if (map[x][y][z].isopen() == true)											// open�� ���̶�� G��(cost)�� ���ŵ� �� ���� �Ǵ��Ѵ�.
		{
			if (map[x][y][z].G_value + 0.001 < next_cost)								/// �������� �� ���� cost�� �ʿ�� �ϸ�
			{
				// ����� �ƹ��͵� ����
				return true;													// ���� �� ���
			}
			else /// �� �ܿ�
			{
				vector<grid>::iterator it;
				it = find(vec.begin(), vec.end(), grid(x, y, z));     // open info ���� ���� ��ġ�� �ش��ϴ� ������ ã�� �����Ѵ�.
				if (it != vec.end())
					vec.erase(it);

				map[x][y][z].setG(next_cost);										// �������� ���� cost�� �ʿ�� �ϸ� �� ���� G_value�� ����
				map[x][y][z].setF(F_value);
				map[x][y][z].setXYZ(x, y, z);
				map[x][y][z].setlevel(level);
				vec.push_back(map[x][y][z]);							// �Ķ���͸� �����ϰ� ���Ӱ� open list�� �߰���.

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
		else // open���°� �ƴ� ���̸�
		{
			map[x][y][z].open = 1;
			map[x][y][z].setG(next_cost);
			map[x][y][z].setF(F_value);
			map[x][y][z].setXYZ(x, y, z);
			map[x][y][z].setlevel(level);

			vec.push_back(map[x][y][z]);			// open list�� �ִ´�.

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
	map[xpos][ypos][zpos].close();				   // �������� ����.
	path.push_back(path_data(xpos, ypos, zpos, 0, NONE)); // �������� ��ǥ�� �ڽ�Ʈ�� list�� push
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


vector<path_data> sharpening(vector<path_data> path) // ����ȭ �˰���
{


	
	/// ���� ����
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

		if (((double)((x1 + x3) / 2.0) == x2) + ((double)((y1 + y3) / 2.0) == y2) + ((double)((z1 + z3) / 2.0) == z2) >= 2) // �������̸�
		{
			map[(it + 1)->x][(it + 1)->y][(it + 1)->z].closed = 0;  //������ ���� ���·� �����.
			path.erase(it + 1); // �߰��κ��� �����.
			
			continue;
		}
		else
		{
			it++;   // �������� �ƴϸ� ���� iteration����
		}
	}
	
	/// ���� ���� �Ϸ�

	/// �ڵ� ��������
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CChildFrame *pChild = (CChildFrame *)pFrame->GetActiveFrame();
	CTESTOCCView* pView = (CTESTOCCView*)pChild->GetActiveView();
	///

	/// ����ȭ ����

	it = path.begin();

	int index = 0;
	while (1)
	{
		bool flag = 0;

		if (index + 3 >= path.size())			// ���� ������ �簢���� �����ų� ���ų�
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

			/// 3������ ��� �������ų�, 1�����θ� �����ΰ��
			if (x1 != x4) deltax = 1;
			if (y1 != y4) deltay = 1;
			if (z1 != z4) deltaz = 1;

			int type = deltax + deltay + deltaz;

			if (type <= 1)  // 2�� �簢���� ������ �ȵǸ� �����ݺ�����
			{
				continue;			// ���� iteration����
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
						continue;				// ��������
					}

					/// �������� ���� ������ ��� ��ǥ�� ��������.

					for (int j = i - 1; j > index; j--)
					{
						map[path[j].x][path[j].y][path[j].z].closed = 0;  //������ ���� ���·� �����.
						vector<path_data>::iterator it = path.begin();
						it += j;		// path[j]�� ����
						path.erase(it);
					}

					vector<path_data>::iterator it = path.begin();
					it += index + 1;  // path[index+1]�� ����

					path.insert(it, path_data(lower_grid.x, lower_grid.y, lower_grid.z, 0, NONE));
					break;
				}
				else
				{
					for (int j = i - 1; j > index; j--)
					{
						map[path[j].x][path[j].y][path[j].z].closed = 0;  //������ ���� ���·� �����.
						vector<path_data>::iterator it = path.begin();
						it += j;		// path[j]�� ����
						path.erase(it);
					}

					vector<path_data>::iterator it = path.begin();
					it += index + 1;  // path[index+1]�� ����

					path.insert(it, path_data(upper_grid.x, upper_grid.y, upper_grid.z, 0, NONE));
					break;
				}
			}

			else if (type == 3) // 6���� ����
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
					if (col_route[k] == 0) // k��° ��Ʈ�� �浹�� ������ (�� ���� ����)
					{
						printf("\n\ntest\n\n");
						for (int j = i - 1; j > index; j--)  // �߰��� �ִ� ������ �� ����
						{
							map[path[j].x][path[j].y][path[j].z].closed = 0;  //������ ���� ���·� �����.
							vector<path_data>::iterator it = path.begin();
							it += j;		// path[j]�� ����
							path.erase(it);
						}

						vector<path_data>::iterator it = path.begin();
						it += index;  // path[index+1]�� ����

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
		/// ��, 2������ ������ ��츸 �ٷ��.
		

		/// �� �ܿ��� �簢���� �׸���.  (closed check, �浹 ����߿� �浹����� ���)
	}



	
	/// ����ȭ�� �Ϸ�Ǹ� �� ��ε��� close �� obstacle ó�� �Ѵ�.

	for (int i = 0; i < path.size() - 1; i++)
	{
//		printf("test\n");
		if (i + 1 == path.size()) // �ε��� �ʰ� ����
			break;
		/// path[i]�� path[i+1]���� ��� ������ closed, obstacle ó����
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


	/// ��ֹ� Ȯ�� (������ ������ŭ)
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






vector<path_data> pathfinder(point pinit, point pfinal)// ����Ÿ���� ��θ� ���� structure, ���ڴ� �����, ������, �������� ���� sturucture
{
	/// �ʱ� ���� ���� ///////////////////
	int xpos_init = pinit.x;
	int ypos_init = pinit.y;
	int zpos_init = pinit.z;
	int xpos_final = pfinal.x;
	int ypos_final = pfinal.y;
	int zpos_final = pfinal.z;

	int current_x = xpos_init;
	int current_y = ypos_init;
	int current_z = zpos_init;


	map[xpos_final][ypos_final][zpos_final].go_here = 1; /// �������� �߰��ϸ� ������ ����.

	/// �ʱ� �۾� ////////////////
	vector<path_data> total_path; // path_data�� ���� ��ũ�帮��Ʈ
	vector<path_data> path_info;  // path�� ������ list�� ����
	Init(path_info, xpos_init, ypos_init, zpos_init);  // �������� push�ϰ�, �������� close��
	vector<grid> open_info;
	///


	if (map[xpos_final][ypos_final][zpos_final].closed == 1)
	{
		return path_info;
	}

	/// ����Ʈ�� ���� //////////////////////
	double min_F = 0;
	double min_deltaG = 0;
	double current_cost = 0;
	DIRECTION min_dir = X_POS;
	///

	
	while (1)
	{
		
		min_F = pow(2, 31);          // ������ ����
		min_deltaG = pow(2, 31);

		bool is_fullyclosed =
			function(current_x, current_y, current_z, xpos_final, ypos_final, zpos_final, X_POS, current_cost, min_F, min_dir, open_info, path_info,path_info.size())
			*function(current_x, current_y, current_z, xpos_final, ypos_final, zpos_final, X_NEG, current_cost, min_F, min_dir, open_info, path_info,path_info.size())
			*function(current_x, current_y, current_z, xpos_final, ypos_final, zpos_final, Y_POS, current_cost, min_F, min_dir, open_info, path_info,path_info.size())
			*function(current_x, current_y, current_z, xpos_final, ypos_final, zpos_final, Y_NEG, current_cost, min_F, min_dir, open_info, path_info,path_info.size())
			*function(current_x, current_y, current_z, xpos_final, ypos_final, zpos_final, Z_POS, current_cost, min_F, min_dir, open_info, path_info,path_info.size())
			*function(current_x, current_y, current_z, xpos_final, ypos_final, zpos_final, Z_NEG, current_cost, min_F, min_dir, open_info, path_info,path_info.size());

		if (is_fullyclosed) // ���ٸ����̸�
		{
			printf("check3 : size : %d\n", open_info.size());
			/// open list�� F_value ������� ������
			if (!open_info.empty())			// �ϳ��� ���� ���� ������
			{
				sort(open_info.begin(), open_info.end());    // ���� ���� sort�Ѵ�.  sort�� H_value�� �������� �Ѵ�.
				vector<grid>::iterator it;
				it = find(open_info.begin(), open_info.end(), grid(current_x, current_y, current_z)); // openlist�߿� ���� ��ġ�� ã�� ����������.
				if (it != open_info.end())
				{
					open_info.erase(it);
				}
			}
			else
			{
				path_info.clear();
				return path_info;
				// ���� ã������ ���°��.
			}

			go_backward(current_x, current_y, current_z, current_cost, path_info, xpos_init, ypos_init, zpos_init, xpos_final, ypos_final, zpos_final, open_info); // �ǵ��ư���.

			if (path_info.empty())
			{
				return path_info;
				// ���� ã������ ����.
			}

			continue;
		}


		/// ���õ� ��ǥ�� ������Ͽ��� ���� ������Ͽ� �߰���
		vector<grid>::iterator it;
		it = find(open_info.begin(), open_info.end(), grid(current_x, current_y, current_z));
		if (it != open_info.end())
			open_info.erase(it);
		map[current_x][current_y][current_z].close();
		///


		current_cost += WEIGHT_OF_COST; // �� ���� ��ġ �߿�.  ���߿� ������ �κ�


		

		/// ȭ��ǥ �׸��� �κ�
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
		

		/// ����׿� visualize //
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
			
			vector<path_data>::iterator it, it2;    // �� �ܰ迡�� ��� �����͸� total_path�� ���������� �������.
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



