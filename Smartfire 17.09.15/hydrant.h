#pragma once
#include "stdafx.h"
#include <vector>
#include <map>
#include <functional>
#include "config.h"
#include "functions.h"

using namespace std;


extern double THRESHOLD_OF_OBSTACLE;
extern double THRESHOLD_OF_FLOOD_AREA;
extern double THRESHOLD_OF_PATH_AREA;
extern double THRESHOLD_OF_WALL_AREA;
extern double RESTRICTED_DISTANCE;
extern double THRESHOLD_OF_DOOR;


#define MAX_STAIRS 30
#define MAX_ACTION_STACK 200
#define NUMOFTYPE 6  // �� 5���� �ҹ���� Ÿ���� ���� 0~2�� ��ȭ��, 3�� ȣ����, 4�� ��ȭ��, 5�� �����
#define NUM_OF_SOURCE 9
#define NUM_OF_COLOR 9
#define MODE_VISUALIZE		0
#define MODE_DRAWING		1
#define MODE_OPTIMIZATION	2
class output
{
private:

	CvFont font;
	/// ���� ���� ////
	char m_file[500];
	int m_currentlayer;
	///


	/// ��� ���� ///
	int m_numofstair;
	int m_highlighted_stair;
	int m_program_mode;
	///

	/// �Ķ����
	vector<HYDRANT> m_hydpos;
	vector<vector<Node>> m_grid;
	vector<vector<Node>> m_background_grid;
	IplImage *m_original_image;
	IplImage *m_ents;
	ScaleLine m_scaleline;


	vector<set<node>> m_obstacle_outlines[NUMOFLAYER]; // obstacle ���յ�
	vector<set<node>> m_wall_outlines;				   // wall ���յ�

	vector<node> m_streamable_point;
	vector<node> m_floodable_point;
	vector<node> m_feasible_point;
	vector<node> m_recommended_point;
	vector<node> m_doors;

	///
	/// optimization ����
	FILE *log;
	int m_nHyd;
	int m_nPop;
	int cur_iter;
	int max_iteration;
	int max_local_iteration;
	int cur_iteration;
	int cur_local_iteration;
	double inertia;
	double damping;
	double C1;
	double C2;
	double minvel;
	double maxvel;
	double required_occupy_rate;
	double Default_Length;
	double Default_Stream_Length;

	double door_preference;
	double door_damping;
	double path_preference;
	double path_damping;
	double Infeasible_range;

	int cur_index;
	bool fixmode;
	bool local_updated[100];

	pair<double, double> single_n_double_coverages;
	pair<double, double> prev_single_n_double_coverages;
public:
	void Opt_Init_Option1_deterministic(double defaultlength, double defaultstreamlength, bool isoption1, double goal_occupancy, bool fixed);
	void Opt_Init_Option1(int mode, double defaultlength, double goal_occupancy) {}
	void give_score();
	
	void PSO_by_step();
	int Do_deterministic_1_step(bool isoption1);
	int Do_deterministic_move_step(bool isoption1);
	int Do_deterministic_move_prefer_area(bool isoption1);
	int Do_deterministic_move_prefer_area_option2();
	void Do_PSO_Option1_Init(double goal_occupancy);
	void Do_PSO_Option2_Init();
	int Do_PSO_Option1_Step();
	bool there_is_overlapped(vector<optptr> &container, int index, bool isoption1);
	pair<double, double> cal_single_and_double_overlapped(vector<optptr> &src);
	pair<int, int> cal_occupied_and_overlapped(vector<optptr> &container, int index, bool isoption1);
	double cal_achieved_percent_with_new_index(vector<optptr> &container, HYDRANT &target, int index, bool isoption1);
	pair<double,double> cal_total_overlapped_and_achieved(vector<optptr> &container, bool isoption1);
	double cal_cost(vector<optptr> &src, bool isoption1);
	vector<vector<pair<ptr, double>>> possible_boundary;

	void Update_Particle(vector<optptr> &src, bool isoption2); // Ư�� population�� ��ƼŬ���� ������Ʈ �ϰ� ������ �� �Լ� ȣ��
	void Set_Hyd_Parameters_for_Opt(int x, int y, double length, double streamlength, HYDRANT &target);
	void Set_Iteration(int num);
	
	void Set_Scoring_Parameters(double door_pref, double door_damp, double path_pref, double path_damp);

	void Update_Feasible_Region();
	void Set_Infeasible_Range(double range) 
	{ 
		Infeasible_range = range; 
	}
	void Do_Plan2(); // option2 ����

	void Reset_Irregular_Set() { length_array.clear(); }
	void Add_Irregular_Hyd(double value) 
	{ 
		length_array.push_back(value);
	}
	vector<optptr> single_groups;
	vector<optptr> single_groups_spared;

	vector<HYDRANT> m_globalhydpos;

	vector<vector<optptr>> cur_groups;
	vector<double> cur_costs;
	vector<vector<optptr>> best_groups;
	vector<double> best_costs;
	vector<optptr> global_best;
	double global_cost;
	vector<double> length_array;
	
	
	/// ����� ����
	deque<ActionStack> m_ActStack;
	bool m_drawingmode;
	vector<drawinginfo> m_drawlinenodes;              // ����ڰ� ���Ƿ� �׸� ���� �����ϴ� ��
	//deque<vector<drawinginfo>> m_drawdeque;      // ����ϰ� �׸� ������ �ӽ÷� �����ϴ°� (undo��)
	///


	/// �÷���
	COLOR C_STREAM[NUM_OF_SOURCE];
	COLOR C_ROUTE[NUM_OF_SOURCE], C_HYDRANT[NUMOFTYPE], C_WALL, C_OBSTACLE[NUMOFLAYER], C_STAIR, C_VIRTUAL;
	COLOR C_PATH;
	COLOR C_UNACTIVATED_HYDRANT;
	COLOR C_HIGHLIGHTED_HYDRANT;
	COLOR C_GUIDELINE;
	COLOR C_SCALE;
	COLOR C_EQUIPMENT;
	COLOR C_MONITOR_SYMBOL, C_MONITOR_STREAM;
	///

	/// ���
	int GRIDSIZE; // �׸��������
	int PRE_GRIDSIZE;
	int TEMP_GRIDSIZE;
	double DISTANCE_PER_PIXEL; // �ȼ��� ����
	double DISTANCE_PER_GRID;  // 1�׸���� ���� ����
	double STAIR_PENALTY[MAX_STAIRS];
	int SLICERATE; // ��Ž���� ���ҷ���
	double DEFAULT_STREAM_LENGTH;    // ���ٱ��� ����
	double DEFAULT_PATH_LENGTH;
	double SCALELENGTH;
	///

	/// ��� �ν� ���� ///
	bool ishydrant(IplImage *img, int x, int y, int type);
	bool iswall(IplImage *img, int x, int y);
	bool isobstacle(IplImage *img, int x, int y, int layernum);
	bool isstair(IplImage *img, int x, int y);
	bool isscale(IplImage *img, int x, int y);
	bool iswhite(IplImage *img, int x, int y);
	bool isequipment(IplImage *img, int x, int y);
	bool ispath(IplImage *img, int x, int y);
	/// ��� �ν� ���� ///
public:
	


	void Give_score();





	/// �ʱ�ȭ �� �ν� �Լ� ////
	void Init(int mode);
	void detect_scaleline(); // ��ô���� �а�, DISTANCE_PER_PIXEL�� ����
	vector<HYDRANT> detect_initial_hydrants(); // �ʱ� ��ġ ��ȭ���� �д� �Լ�
	void Initialize_grid();     // �׸��� ����
	void detect_cad_elements(); // ĳ�� ��� �ν�
	void detect_closed_loop();  // �� �ν�
	void swap_grid_to_background();
	void detect_optimization_elements(); // feasible, floodable, streamable ���� ����
	/// �ʱ�ȭ �� �ν� �Լ� ////

	/// ��� ���� �Լ� ///
	void flood(HYDRANT &pos, int numofiter = 0);
	void stream(HYDRANT &hyd, bool accurate_mode = false);
	void Calculate_Stream_by_Monitor(HYDRANT &hyd);

	vector<ptr> padding(vector<ptr> vecs);
	vector<ptr> three_ptr_smoothing(vector<ptr> vec, bool reversed);
	vector<ptr> four_ptr_smoothing(vector<ptr> vec, bool reversed);
	double totaldistance(vector<ptr> path, double total_stairpenalty);

	ptr collision_detection_for_stream(ptr p1, ptr p2, IplImage *img); // ���ٱ� ���� �� �� ����
	bool collision_detection_for_path(ptr p1, ptr p2, bool fpoint); // ������ ���� collision detection
	void sorthyds(){sort(m_hydpos.begin(), m_hydpos.end());}
	/// �˰��� ���� �Լ� ///
public:
	output(); // ������

	/// ���� ���� ���� ///
	void loadpng(char *filename);
	/// ���� ���� ���� ///


	/// �ҿ����� ����
	void Detect_Uncontinuous_Point();


	/// �۾��� �̹��� ����

	int saveimg(int displaycode, const char* filepath);
	void exportimg(IplImage *img, const char *filepath);
	void Get_guidebox(int hydnum, int size);
	/// �۾��� �̹��� ����

	/// advanced ���� üũ ���� ///
	
	

	/// ����� ������
	int Update_Single_Hydrant();
	
	void Recalculate_All();
	void Add_Hyd(int xpos, int ypos, double length, double streamlength, int sourcenum, int type, int colorindex, int activation = 1, bool calculation = false);
	void Add_Monitor(int x1, int x2, int x3, int y1, int y2, int y3, double streamlength, int colorindex);
	void Delete_Hyd(int hydnum);
	void Highlight_Hyd(int hydnum);
	void Unhighlight_Hyd() { for (int j = 0; j < m_hydpos.size(); j++){ m_hydpos[j].highlighted = false; } }
	void Unhighlight_stair()  { m_highlighted_stair = -1; }
	int istherehydrant(int x, int y);
	int istherestair(int x, int y);
	void calculate_hydrant_area(int hydnum, bool update);
	void Set_Hyd_Parameters(int hydnum, int xpos, int ypos, double length, double streamlength, int sourcenum, int colorindex, int type, int activation, int highlighted, bool calculation = false);

	
	int get_program_mode() { return m_program_mode; }

	bool m_user_loaded;
	void saveinfo(const char *path); // ������ ����
	void loadinfo(const char *path);

	/// Get Set ///
	int Get_Hyd_index(int hydnum);
	double Get_route_overlapped_area(int hydnum1, int hydnum2);
	double Get_stream_overlapped_area(int hydnum1, int hydnum2);

	void Set_Layer(int layernum) { m_currentlayer = layernum; }

	void Set_Color_ROUTE(unsigned char R, unsigned char G, unsigned char B, int colorindex)  { int n = colorindex;  C_ROUTE[n].R = R; C_ROUTE[n].G = G; C_ROUTE[n].B = B; }
	void Set_Color_STREAM(unsigned char R, unsigned char G, unsigned char B, int colorindex) { int n = colorindex;  C_STREAM[n].R = R; C_STREAM[n].G = G; C_STREAM[n].B = B; }
	void Set_Color_HYDRANT(unsigned char R, unsigned char G, unsigned char B, int type)  { C_HYDRANT[type].R = R; C_HYDRANT[type].G = G; C_HYDRANT[type].B = B; }
	void Set_Color_WALL(unsigned char R, unsigned char G, unsigned char B) { C_WALL.R = R; C_WALL.G = G; C_WALL.B = B; }
	void Set_Color_OBSTACLE(unsigned char R, unsigned char G, unsigned char B, int layernum) { C_OBSTACLE[layernum].R = R; C_OBSTACLE[layernum].G = G; C_OBSTACLE[layernum].B = B; }
	void Set_Color_STAIR(unsigned char R, unsigned char G, unsigned char B) { C_STAIR.R = R; C_STAIR.G = G; C_STAIR.B = B; }
	void Set_Color_SCALE(unsigned char R, unsigned char G, unsigned char B) { C_SCALE.R = R; C_SCALE.G = G; C_SCALE.B = B; }
	void Set_Color_PATH(unsigned char R, unsigned char G, unsigned char B) { C_PATH.R = R; C_PATH.G = G; C_PATH.B = B; }

	void Set_Gridsize(int size){ TEMP_GRIDSIZE = size; }
	void Set_ScaleLength(double scale) { SCALELENGTH = scale; }
	void Set_Stair_Penalty(int num, double penalty) { STAIR_PENALTY[num] = penalty; }
	void Set_Slicerate(int rate) { SLICERATE = rate; }
	void Set_Highlighted_Stair(int num)  { m_highlighted_stair = num; }
	void Set_Distance_Per_Grid(double value) { DISTANCE_PER_GRID = value; }
	void Set_ScaleLine(int x1, int x2, int y, double length);
	
	int Get_Color_Wall_R() { return C_WALL.R; }
	int Get_Color_Wall_B() { return C_WALL.B; }
	int Get_Color_Wall_G() { return C_WALL.G; }
	int Get_Color_Obstacle_R(int layernum) { return C_OBSTACLE[layernum].R; }
	int Get_Color_Obstacle_B(int layernum) { return C_OBSTACLE[layernum].B; }
	int Get_Color_Obstacle_G(int layernum) { return C_OBSTACLE[layernum].G; }
	int Get_Color_Path_R() { return C_PATH.R; }
	int Get_Color_Path_G() { return C_PATH.G; }
	int Get_Color_Path_B() { return C_PATH.B; }
	int Get_Color_Equipment_R() { return C_EQUIPMENT.R; }
	int Get_Color_Equipment_G() { return C_EQUIPMENT.G; }
	int Get_Color_Equipment_B() { return C_EQUIPMENT.B; }


	int Get_numofitems() { return m_hydpos.size(); }
	int Get_numofhydrant(){ int count = 0;	for (int i = 0; i < m_hydpos.size(); i++)	{ if (m_hydpos[i].type == 0) { count++; } }	return count; }
	int Get_numofhydrant_form(){ int count = 0;	for (int i = 0; i < m_hydpos.size(); i++)	{ if (m_hydpos[i].type == 1) { count++; } }	return count; }
	int Get_numofhydrant_powder(){ int count = 0;	for (int i = 0; i < m_hydpos.size(); i++)	{ if (m_hydpos[i].type == 2) { count++; } }	return count; }
	int Get_numofhosereel(){ int count = 0;	for (int i = 0; i < m_hydpos.size(); i++)	{ if (m_hydpos[i].type == 3) { count++; } }	return count; }
	int Get_numofmonitor() { int count = 0;	for (int i = 0; i < m_hydpos.size(); i++)	{ if (m_hydpos[i].type == 5) { count++; } }	return count; }
	int Get_numofextinguisher(){ int count = 0;	for (int i = 0; i < m_hydpos.size(); i++)	{ if (m_hydpos[i].type == 4) { count++; } }	return count; }
	int Get_numofstair() { return m_numofstair; }

	int Get_gridsize(){ return GRIDSIZE; }
	int Get_slicerate(){ return SLICERATE; }
	double Get_stair_penalty(int num){ return STAIR_PENALTY[num]; }
	double Get_Distance_Per_Grid(){ return DISTANCE_PER_GRID; }
	double Get_scalelength() { return SCALELENGTH; }

	int Get_Hyd_xpos(int hydnum){ return m_hydpos[hydnum].x; }
	int Get_Hyd_ypos(int hydnum){ return m_hydpos[hydnum].y; }
	int Get_Hyd_sourcenum(int hydnum){ return m_hydpos[hydnum].sourcenum; }
	int Get_Hyd_area(int hydnum){ return m_hydpos[hydnum].range.size(); }
	int Get_Stream_area(int hydnum){ return m_hydpos[hydnum].stream.size(); }
	int Get_Hyd_type(int hydnum) { return m_hydpos[hydnum].type; }
	double Get_Hyd_length(int hydnum){ return m_hydpos[hydnum].length; }
	double Get_Hyd_streamlength(int hydnum){ return m_hydpos[hydnum].streamlength; }
	bool Get_Hyd_highlighted(int hydnum) { return m_hydpos[hydnum].highlighted; }
	bool Get_Hyd_activated(int hydnum) { return m_hydpos[hydnum].activate; }

	int Get_Hyd_colorindex( int hydnum) { return m_hydpos[hydnum].colorindex; }

	
	/// ��ȭ�� �Ķ���� ȹ�� ���� ///


	
	

#define OBJECT_WALL 0
#define OBJECT_OBSTACLE 1
#define OBJECT_PATH 2
#define OBJECT_EQUIPMENT 3
#define TYPE_2P_LINE 0
#define TYPE_2P_RECTANGLE 1
#define TYPE_2P_CIRCLE 2
#define TYPE_2P_DOUBLELINE 3
#define MAX_STACK 100 // ���þȿ� ������ �� �ִ� �ִ� UNDOȽ��
	/// drawing ���� ///
	void Draw_2_Point(int x1, int y1, int x2, int y2, int type_2p, int type_object, int layernum = 0);
	void Update_Drawing();
	void reset_drawing();
	int Trim_Line(int x, int y); // ���콺 ��ġ�� �ڸ�
	void Undo();
};
