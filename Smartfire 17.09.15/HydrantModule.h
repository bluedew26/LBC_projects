// HydrantModule.h

#pragma once
#include "hydrant.h"

using namespace System;

namespace HydrantModule {

	public ref class HydModule
	{
	public:
		output *module;
		HydModule() { module = new output; }
		virtual ~HydModule() { delete module; }
		
		/// 파일 제어 관련 ///
		void loadpng(char* filename) {	module->loadpng(filename); 	}
		/// 파일 제어 관련 ///

		/// 초기화 ///
		void Init(int mode) { module->Init(mode);}
		/// 초기화 ///

		/// 파라미터 수정 관련 ///

		/// 색상 정의 ///
		void Set_Color_ROUTE(unsigned char R, unsigned char G, unsigned char B, int colorindex) { module->Set_Color_ROUTE(R, G, B, colorindex); } // route는 호스의 도달범위 색깔
		void Set_Color_STREAM(unsigned char R, unsigned char G, unsigned char B, int colorindex){ module->Set_Color_STREAM(R, G, B, colorindex); }
		void Set_Color_HYDRANT(unsigned char R, unsigned char G, unsigned char B, int type) { module->Set_Color_HYDRANT(R, G, B, type); }
		void Set_Color_WALL(unsigned char R, unsigned char G, unsigned char B) { module->Set_Color_WALL(R, G, B); }
		void Set_Color_OBSTACLE(unsigned char R, unsigned char G, unsigned char B, int layernum) { module->Set_Color_OBSTACLE(R, G, B, layernum); }
		void Set_Color_STAIR(unsigned char R, unsigned char G, unsigned char B) { module->Set_Color_STAIR(R, G, B); }
		void Set_Color_SCALE(unsigned char R, unsigned char G, unsigned char B) { module->Set_Color_SCALE(R, G, B); }
		void Set_Color_PATH(unsigned char R, unsigned char G, unsigned char B) { module->Set_Color_PATH(R, G, B); } // path는 "선호하는"길 정도로 기존의 path와 다름
		
		void Set_Layer(int layernum) { module->Set_Layer(layernum); }
		void Set_Gridsize(int size) { module->Set_Gridsize(size); }
		void Set_Scalelength(double scale) { module->Set_ScaleLength(scale);}
		void Set_Stair_Penalty( int num, double penalty) { module->Set_Stair_Penalty(num, penalty); }
		void Set_Slicerate(int rate) { module->Set_Slicerate(rate); }
		void Set_Highlighted_Stair(int num) { module->Set_Highlighted_Stair(num); }
		void Set_Distance_Per_Grid(double value) { module->Set_Distance_Per_Grid(value); }
		void Set_ScaleLine(int x1, int x2, int y, double length) { module->Set_ScaleLine(x1, x2, y, length); }
		
		int Get_Color_Wall_R() { return module->Get_Color_Wall_R(); }
		int Get_Color_Wall_B() { return module->Get_Color_Wall_B(); }
		int Get_Color_Wall_G() { return module->Get_Color_Wall_G(); }
		int Get_Color_Obstacle_R(int layernum) { return module->Get_Color_Obstacle_R(layernum); }
		int Get_Color_Obstacle_B(int layernum) { return module->Get_Color_Obstacle_B(layernum); }
		int Get_Color_Obstacle_G(int layernum) { return module->Get_Color_Obstacle_G(layernum); }
		int Get_Color_Path_R() { return module->Get_Color_Path_R(); }
		int Get_Color_Path_G() { return module->Get_Color_Path_G(); }
		int Get_Color_Path_B() { return module->Get_Color_Path_B(); }
		int Get_Color_Equipment_R() { return module->Get_Color_Equipment_R(); }
		int Get_Color_Equipment_G() { return module->Get_Color_Equipment_G(); }
		int Get_Color_Equipment_B() { return module->Get_Color_Equipment_B(); }

		int Get_numofitems()  { return module->Get_numofitems(); }
		int Get_numofhydrant(){ return module->Get_numofhydrant(); }
		int Get_numofhydrant_form() { return module->Get_numofhydrant_form(); }
		int Get_numofhydrant_powder(){ return module->Get_numofhydrant_powder(); }
		int Get_numofhosereel(){ return module->Get_numofhosereel(); }
		int Get_numofextinguisher(){ return module->Get_numofextinguisher(); }
		int Get_numofmonitor(){ return module->Get_numofmonitor(); }
		double Get_Distance_Per_Grid(){ return module->Get_Distance_Per_Grid(); }
		double Get_stair_penalty(int num)  { return module->Get_stair_penalty(num); }
		int Get_gridsize()  { return module->Get_gridsize(); }
		int Get_numofstair() { return module->Get_numofstair(); }
		double Get_scalelength(){ return module->Get_scalelength(); }
		int Get_slicerate()  { return module->Get_slicerate(); }

		void Set_Hyd_Parameters(int hydnum, int xpos, int ypos, double length, double streamlength, int sourcenum, int colorindex, int type, int activation, int highlighted, bool calculation)
		{
			module->Set_Hyd_Parameters(hydnum, xpos, ypos, length, streamlength, sourcenum, colorindex, type, activation, highlighted, calculation);
		}
		void Add_Hyd(int xpos, int ypos, double length, double streamlength, int sourcenum, int type, int colorindex, int activation, bool calculation)
		{
			module->Add_Hyd(xpos, ypos, length, streamlength, sourcenum, type, colorindex, activation, calculation);
		}
		void Add_Monitor(int x1, int x2, int x3, int y1, int y2, int y3, double streamlength, int colorindex)
		{
			module->Add_Monitor(x1, x2, x3, y1, y2, y3, streamlength, colorindex);
		}
		void Delete_Hyd(int hydnum) { module->Delete_Hyd(hydnum); }


		/// 소화전 파라미터 획득 관련 ///
		int Get_Hyd_xpos(int hydnum) { return module->Get_Hyd_xpos(hydnum); }
		int Get_Hyd_ypos(int hydnum) { return module->Get_Hyd_ypos(hydnum); }
		int Get_Hyd_sourcenum(int hydnum) { return module->Get_Hyd_sourcenum(hydnum); }
		int Get_Hyd_type(int hydnum) { return module->Get_Hyd_type(hydnum); }
		double Get_Hyd_length(int hydnum) { return module->Get_Hyd_length(hydnum); }
		double Get_Hyd_streamlength(int hydnum) { return module->Get_Hyd_streamlength(hydnum); }
		bool Get_Hyd_activated(int hydnum) { return module->Get_Hyd_activated(hydnum); }
		int Get_Hyd_index(int hydnum) { return module->Get_Hyd_index(hydnum); }
		int Get_Hyd_colorindex(int hydnum) { return module->Get_Hyd_colorindex(hydnum); }

		/// 화면 클릭시 대응
		int istherehydrant(int x, int y) { return module->istherehydrant(x, y); }
		int istherestair(int x, int y){ return module->istherestair(x, y); }


		void Unhighlight_stair() { module->Unhighlight_stair(); }
		void Unhighlight_Hyd(){ module->Unhighlight_Hyd(); }
		/// 소화전 파라미터 획득 관련 ///
		
		void Recalculate_All() { module->Recalculate_All(); } /// 재계산
		
		void detect_closed_loop(){ module->detect_closed_loop(); } /// 불연속구간 탐색

		/// Report 관련
		double Get_route_overlapped_area(int hydnum1, int hydnum2) { return module->Get_route_overlapped_area(hydnum1, hydnum2); }
		double Get_stream_overlapped_area(int hydnum1, int hydnum2) { return module->Get_stream_overlapped_area(hydnum1, hydnum2); }



		int Update_Single_Hydrant() { return module->Update_Single_Hydrant(); }

		/// 작업된 이미지 저장
		int saveimg(int displaycode, const char* filepath) { return module->saveimg(displaycode, filepath); }
		int get_program_mode() { return module->get_program_mode();}
		void saveinfo(const char *path) { module->saveinfo(path); }
		void loadinfo(const char *path) { module->loadinfo(path); }
		/// 작업된 이미지 저장

		/// drawing 관련
		void Undo() { module->Undo(); }
		void reset_drawing() { module->reset_drawing(); } /// 그리기 초기화
		void Draw_2_Point(int x1, int y1, int x2, int y2, int type_2p, int type_object, int layernum) { module->Draw_2_Point(x1, y1, x2, y2, type_2p, type_object, layernum); }
		int Trim_Line(int x, int y){ return module->Trim_Line(x, y); }

		/// 최적화 관련
		void Opt_Init_Option1_deterministic(double defaultlength, double defaultstreamlength, bool isoption1, double goal_occupancy, bool fixed) { module->Opt_Init_Option1_deterministic(defaultlength, defaultstreamlength, isoption1, goal_occupancy, fixed); }
		void PSO_by_step() { module->PSO_by_step(); }
		int Do_deterministic_1_step(bool isoption1) { return module->Do_deterministic_1_step(isoption1); }
		int Do_deterministic_move_step(bool isoption1) { return module->Do_deterministic_move_step(isoption1); }
		int Do_deterministic_move_prefer_area(bool isoption1) { return module->Do_deterministic_move_prefer_area(isoption1); }
		void Set_Iteration(int num){ module->Set_Iteration(num); }
		void give_score(){ module->give_score(); }
		void Set_Scoring_Parameters(double door_pref, double door_damp, double path_pref, double path_damp) { module->Set_Scoring_Parameters(door_pref, door_damp, path_pref, path_damp); }
		void Reset_Irregular_Set() { module->Reset_Irregular_Set(); }
		void Add_Irregular_Hyd(double value) { module->Add_Irregular_Hyd(value); }
		void Set_Infeasible_Range(double range) { module->Set_Infeasible_Range(range); }
		void Update_Feasible_Region() { module->Update_Feasible_Region(); }

		void Do_PSO_Option1_Init(double goal_occupancy){ module->Do_PSO_Option1_Init(goal_occupancy); }
		int Do_PSO_Option1_Step() { return module->Do_PSO_Option1_Step(); }
		void Do_Plan2(){ module->Do_Plan2(); }
	};
}
