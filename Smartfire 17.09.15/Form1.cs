using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using HydrantModule;
using System.Drawing.Drawing2D;  // Matrix사용을 위한
using Excel = Microsoft.Office.Interop.Excel;
using excelpopup;
//using Microsoft.Office.Interop.Excel;
namespace smartfire
{
    enum ProgMode {VISUALIZE, DRAWING, OPTIMIZATION }; // 프로그램 모드 관련 열거자
    enum MouseMode {DEFAULT, ADD_HYD, MOVE_HYD, DRAWING_START, DRAWING_CONT, DRAW_SCALELINE, DRAW_SCALELINE_CONT, DRAW_MONITOR_POS, DRAW_MONITOR_DST, DRAW_MONITOR_ANGLE, TRIM}
    enum Drawing_Type_Object { OBSTACLE, WALL, PATH, EQUIPMENT };
    enum Drawing_Type_2P {LINE, RECTANGLE, CIRCLE, DOUBLELINE};

    public partial class Form1 : Form
    {
        public HydModule module = new HydModule(); // 모듈 선언

        private MouseMode mode = MouseMode.DEFAULT;  // 0은 평상시, 1은 소화전 추가모드, 2소화전 이동모드
                               // 3은 그리기 모드, 4는 이어그리기 모드, 5는 그리드모드+소화전추가, 6은 소화전 선택, 7-8은 축척선 그리기, 9-11은 모니터 추가

        bool KEYPRESSED_SHIFT = false;

        // 디스플레이 옵션
        private int DISPLAYSTREAM =          0x1;
        private int DISPLAYPATH =			 0x2;
        private int DISPLAYTEXT	=		     0x4;
        private int DISPLAYGUIDELINE =	     0x8;
        private int DISPLAYMISSINGLINE =	 0x10;
        private int DISPLAYSINGLE =	         0x20;
        private int DISPLAYOVERLAP =         0x40;
        private int DISPLAYBACKGROUNDDRAWING = 0x80;
        private int DISPLAY_SIMPLECHECK = 0x100;
        private int DISPLAY_DETECTION_FLOODABLE = 0x200;
        private int DISPLAY_DETECTION_STREAMABLE = 0x400;
        private int DISPLAY_DETECTION_PATH = 0x800;
        private int DISPLAY_DETECTION_FEASIBLE = 0x1000;
        private int DISPLAY_DETECTION_DOOR = 0x2000;
        private int DISPLAY_DETECTION_PREFERENCE = 0x4000;
        private int DISPLAY_DETECTION_CHECK = 0x8000;

        // 디스플레이 옵션

        private ProgMode prgmode = ProgMode.VISUALIZE;
        private Drawing_Type_2P drawingtype_2p = Drawing_Type_2P.LINE;
        private Drawing_Type_Object drawingtype_object = Drawing_Type_Object.WALL;

        // 파일 로드 관련
        private string filename;
        private string filename_nopath;
        private float mag_x;    // PictureBox의 너비 / 실제 너비
        private float mag_y;    // PictureBox의 높이 / 실제 높이

        private bool Initialized = false; // "도면 생성" 전까지 다른 작업들을 억제
        private string DefaultPath = "C:\\";  // 이미지 파일이 저장되고 불러들여지는 경로

        private int GRIDSIZE = 3;               // 결정된 그리드 사이즈
        


        // 마우스 동작 관련
        private int img_width = 0, img_height = 0;
        private bool focused = false; // 마우스가 들어갈때 true가 됨, 한번만 발생 1회용
        private double Zoommag = 0.7; // 휠 이동시 확대/축소 비율
        private int MAX_Zoom_Level = 8; // 최대 줌 레벨
        private int Zoomlevel = 0;    // 현재 줌 레벨
        private Rectangle Croparea = new Rectangle(); // 잘라서 줌으로 보여줄 사각형 영역
        private Image displaying;     // 표시중인 이미지(realsize)
        private Image cur_img;        // 표시중인 이미지(잘린 사이즈)
        private Size Picbox_size;            // PictureBox의 Picbox_size
 
        private int start_x = 0, start_y = 0; // 현재 보여주는 영역의 시작점좌표
     //   private int curs_x, curs_y;   // 현재 보여주는 영역의 마우스 위치 좌표
     //   private int curabs_x, curabs_y; // 현재 마우스위치 절대좌표

        private Point curpos_abs;
        private Point curpos_mouse;
       

        private Point p1, p1_grid, p1_abs, p1_rel;  // 소화전 추가시 추가좌표와 절대좌표
        private Point p2, p2_grid, p2_abs, p2_rel;  // 소화전 추가시 추가좌표와 절대좌표
        private Point p3, p3_grid, p3_abs, p3_rel;  // 소화전 추가시 추가좌표와 절대좌표
        private Point initp, initp_grid, initp_abs, initp_rel;  // 그리기 작업시 초기 찍은 좌표

        private Point curpos_rel; 
        private const double SEARCHRANGE = 2.0; // 마우스 클릭시 선택범위


        private double Default_hyd_length = 15;
        private double Default_stream_length = 15;


        private int layernum = 0;


        private double parallel_width = 3.0;     // 이중선 폭
        private bool parallel_direction = false; // 이중선 방향
        private bool parallel_cont = false; // 이중선을 계속해서 그리고 있는가
        private Point prev_parallel_dst = new Point();
        private double angle_of_rect = 0;

        // datagrid관련
        private int Hyd_Focused_Row = -1; // 현재 하이라이트된 행;


        bool LButton_Pushed = false;

        // 가시화 관련
        bool showguide = false; // 가이드라인 표시 여부

        private string ori_str;
        // 소방 추가
        int Convert_DrawingType2p_to_Int(Drawing_Type_2P target)
        {
            switch (target)
            {
                case Drawing_Type_2P.LINE:
                    return 0;
                case Drawing_Type_2P.RECTANGLE:
                    return 1;
                case Drawing_Type_2P.CIRCLE:
                    return 2;
                case Drawing_Type_2P.DOUBLELINE:
                    return 3;
                default: return -1;
            }
        }
        int Convert_DrawingType_Object_to_Int(Drawing_Type_Object target)
        {
            switch(target)
            {
                case Drawing_Type_Object.WALL:
                    return 0;
                case Drawing_Type_Object.OBSTACLE:
                    return 1;
                case Drawing_Type_Object.PATH:
                    return 2;
                case Drawing_Type_Object.EQUIPMENT:
                    return 3;
                default: return -1;
            }
        }


        #region 생성자
        public Form1()
        {
            InitializeComponent();
            check_hose.Checked = true;
            radio_all.Checked = true;
            radiobut_wall.Checked = true;
            WindowState = FormWindowState.Maximized;
            mode = MouseMode.DEFAULT;
   //         check_text.Checked = true;
            check_displaydrawing.Checked = true;
            Groupbox_drawingtool.Enabled = false;
            
            groupBox_Optimization.Enabled = false;
            comboBox_layer.Text = "0";
            Cbox_progmode.Text = "0 : 가시화";
            comboBox_Detection_Check.Text = "Default";

        }
        #endregion

        #region Import 함수
        private void button1_Click(object sender, EventArgs e)
        {
            
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Filter = "PngFile (*.png)|*.png";
            if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                filename = dlg.FileName;
                filename_nopath = dlg.SafeFileName;
                Initialized = false;
            }
        }
        // import 버튼
        #endregion

       
        #region 도면 생성 버튼
        private void Initialize_Click(object sender, EventArgs e)
        {
            if (Initialized == false)
            {

                byte[] inputpath = Encoding.ASCII.GetBytes(filename);
                // png 로드 부분
                unsafe
                {
                    fixed (byte* path = inputpath)
                    {

                        sbyte* impath = (sbyte*)path;
                        module.loadpng(impath);
                        // module.Init(true);
                    }
                }
                int mode2 = 0;
                switch (prgmode)
                {
                    case ProgMode.VISUALIZE: mode2 = 0; Groupbox_drawingtool.Enabled = false; groupBox_Optimization.Enabled = false; break;
                    case ProgMode.DRAWING: mode2 = 1; Groupbox_drawingtool.Enabled = true; groupBox_Optimization.Enabled = false; break;
                    case ProgMode.OPTIMIZATION: mode2 = 2; Groupbox_drawingtool.Enabled = false; groupBox_Optimization.Enabled = true;
                        // give_scroe에 사용될 파라미터 입력
                        module.Set_Scoring_Parameters(Convert.ToDouble(textBox_Door_Preference.Text), Convert.ToDouble(textBox_Door_Damping.Text), Convert.ToDouble(textBox_Path_Preference.Text), Convert.ToDouble(textBox_Path_Damping.Text));
                        module.Set_Infeasible_Range(Convert.ToDouble(textBox_infeasible_range.Text));
                        break;
                    default: break;
                }

                if (DisplayLot.Image != null)
                    DisplayLot.Image.Dispose();
                if (cur_img != null)
                    cur_img.Dispose();
                if (displaying != null)
                    displaying.Dispose();
                Croparea = new Rectangle();
                start_x = 0; start_y = 0; // 현재 보여주는 영역의 시작점좌표
                Zoomlevel = 0;    // 현재 줌 레벨




                mode = MouseMode.DEFAULT;


                // 여기서부턴 displaylot 다룸
                DisplayLot.Load(filename); // 원본 이미지

                // 디스플레이 비율 얻기
                float mx, my;
                mx = DisplayLot.Width / (float)DisplayLot.Image.Width;
                my = DisplayLot.Height / (float)DisplayLot.Image.Height;
                mag_x = mx;
                mag_y = my;


                Image img = DisplayLot.Image.Clone() as Image; // 이미지 복사본 얻기

                Picbox_size = new Size(DisplayLot.Width, DisplayLot.Height);
                cur_img = img.Clone() as Image;
                displaying = img.Clone() as Image;

                Croparea.Width = img.Width; // 확대에 의해 자를 부분 얻기
                Croparea.Height = img.Height;
                img_width = img.Width;
                img_height = img.Height;
                Initialized = true;  // 이니셜라이즈 완료

                this.DisplayLot.MouseWheel += new MouseEventHandler(this.DisplayLot_MouseWheel); // 마우스 휠 사용 가능
                module.Set_Gridsize(Convert.ToInt32(textbox_gridsize.Text)); // 그리드 사이즈 설정
                module.Set_Scalelength(Convert.ToDouble(ScaleBox.Text)); // 축척 설정

                module.Init(mode2);  // 이니셜라이즈
                GRIDSIZE = module.Get_gridsize(); // 그리드 사이즈 얻음


                cur_img = resizeImage(cur_img, Picbox_size); // 표시할 이미지를 사이즈에 맞춤
                ReleaseChart();
                ReleaseStairChart();
                //      DisplayLot.Invalidate(); 이거 필요할까?
                img.Dispose();
                
            }
            else // 이미 initialized 된 상태면
            {
                int mode2 = 0;
                switch (prgmode)
                {
                    case ProgMode.VISUALIZE: mode2 = 0; Groupbox_drawingtool.Enabled = false; groupBox_Optimization.Enabled = false; break;
                    case ProgMode.DRAWING: mode2 = 1; Groupbox_drawingtool.Enabled = true; groupBox_Optimization.Enabled = false; break;
                    case ProgMode.OPTIMIZATION: mode2 = 2; Groupbox_drawingtool.Enabled = false; groupBox_Optimization.Enabled = true; break;
                    default: break;
                }

                Croparea = new Rectangle();
                Croparea.Width = img_width; // 확대에 의해 자를 부분 얻기
                Croparea.Height = img_height;
                start_x = 0; start_y = 0; // 현재 보여주는 영역의 시작점좌표
                Zoomlevel = 0;    // 현재 줌 레벨
                module.Set_Gridsize(Convert.ToInt32(textbox_gridsize.Text)); // 그리드 사이즈 설정
                module.Set_Scalelength(Convert.ToDouble(ScaleBox.Text)); // 축척 설정
                module.Init(mode2);  // 이니셜라이즈
                GRIDSIZE = module.Get_gridsize(); // 그리드 사이즈 얻
                mode = MouseMode.DEFAULT;
                ZoomScroll(new Point(0, 0), false);
            }
            UpdateImage();
        }
        #endregion



        #region 이미지 사이즈 변경 (내부처리)
        public static Image resizeImage(Image imgToResize, Size Picbox_size)
        {
            Image res = new Bitmap(imgToResize, Picbox_size);
            imgToResize.Dispose();
            return res;
        }
        #endregion

        #region mouseenter & mouseleave
        private void DisplayLot_MouseEnter(object sender, EventArgs e)
        {
            if (DisplayLot.Focused == false)
            {
                DisplayLot.Focus();
                focused = true;
            }
        }

        private void DisplayLot_MouseLeave(object sender, EventArgs e)
        {
            //focused = false;
        }
        #endregion


        void display_step_by_step()
        {
            while (true)
            {
                if (module.Update_Single_Hydrant() != -1)
                {
                    UpdateImage();
                    Invalidate();
                    Update();
                }
                else
                    break;
            }
            UpdateImage();
            Invalidate();
            Update();
        }

       


        #region 하이라이트 해제
        private void Unhighlight_All()
        {
            module.Unhighlight_Hyd();
            module.Unhighlight_stair();
            for (int i = 0; i < Chart.Rows.Count; i++)
            {
                Chart.Rows[i].Selected = false;
            }
            for (int i = 0; i < stairchart.Rows.Count; i++)
            {
                stairchart.Rows[i].Selected = false;
            }
        }
        #endregion

        #region 마우스 클릭 함수
        private void DisplayLot_MouseClick(object sender, MouseEventArgs e)
        {
            if (Initialized == false) // 이니셜 라이즈 안한 상태면 아무것도 안함
                return;

             switch(mode)
             {
                 case MouseMode.DEFAULT:
                 case MouseMode.ADD_HYD:  
                 case MouseMode.MOVE_HYD:
                 case MouseMode.DRAW_SCALELINE:
                 case MouseMode.DRAWING_START: 
                 case MouseMode.DRAW_MONITOR_POS:
                     p1_abs = getAbsCoord(e.X, e.Y); // 절대좌표를 얻음
                     p1_grid = abstogrid(p1_abs);
                     p1_rel = getRelateCoord(p1_abs);
                     break;
                 case MouseMode.DRAW_SCALELINE_CONT: 
                 case MouseMode.DRAWING_CONT:
                 case MouseMode.DRAW_MONITOR_DST:
                     p2_abs = getAbsCoord(e.X, e.Y);
                     p2_grid = abstogrid(p2_abs);
                     break;
                 case MouseMode.DRAW_MONITOR_ANGLE:
                     p3_abs = getAbsCoord(e.X, e.Y);
                     p3_grid = abstogrid(p3_abs);
                     break;
                 default:
                     break;
             }

            switch(mode)
            {
                case MouseMode.DEFAULT:
                    Unhighlight_All();

                    int range = (int)(SEARCHRANGE / module.Get_Distance_Per_Grid());   // serach range가 몇미터만큼 인식할것인지에 대한 값

                    int hydindex = -1;
                    int strindex = -1;
                    for (int i = -range; i <= range; i++)
                    {
                        for (int j = -range; j <= range; j++)
                        {
                            hydindex = module.istherehydrant(p1_grid.X + i, p1_grid.Y + j);
                            strindex = module.istherestair(p1_grid.X + i, p1_grid.Y + j);
                            if (hydindex != -1) // 발견
                            {
                                module.Set_Hyd_Parameters(hydindex, -1, -1, -1, -1, -1, -1, -1,  -1, 1, false); // 하이라이트만 시킴
                                for (int k = 0; k < Chart.Rows.Count; k++)
                                {
                                    string str = Chart.Rows[k].Cells[7].Value.ToString();  // 숨겨진 행인 x,y값을 조사
                                    string str2 = Chart.Rows[k].Cells[8].Value.ToString();
                                    int x = Convert.ToInt32(str);
                                    int y = Convert.ToInt32(str2);
                                    if (x == module.Get_Hyd_xpos(hydindex) && y == module.Get_Hyd_ypos(hydindex))
                                    {
                                        Hyd_Focused_Row = k;   // 하이라이트 시킬 행을 결정
                                        Chart.Rows[k].Selected = true;
                                        break;
                                    }
                                }
                            }
                            else if (strindex != -1)    // Hydrant는 아니고 Stair가 선택될 경우
                            {
                                module.Set_Highlighted_Stair(strindex);
                                for (int k = 0; k < stairchart.Rows.Count; k++)
                                {
                                    string str = stairchart.Rows[k].Cells[0].Value.ToString(); // stair 고유번호 얻고 비교
                                    int num = Convert.ToInt32(str);
                                    //stairchart.Rows[k].Selected = false;
                                    if (num == strindex)
                                    {
                                        stairchart.Rows[k].Selected = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    break;
                case MouseMode.ADD_HYD:  
                    module.Add_Hyd(p1_grid.X, p1_grid.Y, Default_hyd_length, Default_stream_length, 0, 0, 0, 1, false);
                    ReleaseChart();
                    mode = MouseMode.DEFAULT;
                    break;
                case MouseMode.MOVE_HYD:
                    module.Set_Hyd_Parameters(getindex(Hyd_Focused_Row), p1_grid.X, p1_grid.Y, -1, -1, -1, -1, -1, -1, -1, true);
                    ReleaseChart();
                    mode = MouseMode.DEFAULT;
                    break;
                case MouseMode.DRAW_SCALELINE: mode = MouseMode.DRAW_SCALELINE_CONT; break; // 상대좌표만 얻고 다음단계로 이행
                case MouseMode.DRAWING_START: 
                        mode = MouseMode.DRAWING_CONT; 
                        initp = new Point(e.X, e.Y);
                        initp_abs = getAbsCoord(e.X, e.Y);
                        initp_grid = abstogrid(initp_abs);
                        initp_rel = getRelateCoord(initp_abs);
                        break;
                case MouseMode.DRAW_MONITOR_POS: mode = MouseMode.DRAW_MONITOR_DST; break;
                case MouseMode.DRAW_SCALELINE_CONT: 
                    module.Set_ScaleLine(p1_abs.X, p2_abs.X, p1_abs.Y, Convert.ToDouble(ScaleBox.Text));
                    mode = MouseMode.DEFAULT;
                    break;
                case MouseMode.DRAWING_CONT:
                    Point src = new Point(), dst = new Point();
                    Point src2 = new Point(), dst2 = new Point();
                    if(KEYPRESSED_SHIFT)
                    {
                        switch (drawingtype_2p)
                        {
                            case Drawing_Type_2P.LINE:
                            case Drawing_Type_2P.DOUBLELINE:
                                if (Math.Sqrt(Math.Pow(initp_abs.X - curpos_abs.X, 2) + Math.Pow(initp_abs.Y - curpos_abs.Y, 2)) < GRIDSIZE * 5) // 매우 근접해 있으면 마그넷시킴.
                                {
                                    src = p1_grid; dst = initp_grid;
                                    if (drawingtype_2p == Drawing_Type_2P.DOUBLELINE)
                                    {
                                        Point[] para = Get_Parallel_Line(p1_abs, initp_abs);
                                        prev_parallel_dst = para[1];
                                        src2 = abstogrid(para[0]); dst2 = abstogrid(para[1]);
                                    }
                                    mode = MouseMode.DRAWING_START;
                                }
                                else
                                {
                                    if (Math.Abs(p1_abs.X - curpos_abs.X) > Math.Abs(p1_abs.Y - curpos_abs.Y)) // X쪽으로 일치시켜야함
                                    {
                                        src = p1_grid; dst = new Point(p2_grid.X, p1_grid.Y);
                                        if (drawingtype_2p == Drawing_Type_2P.DOUBLELINE)
                                        {
                                            Point[] para = Get_Parallel_Line(p1_abs, new Point(p2_abs.X, p1_abs.Y));
                                            prev_parallel_dst = para[1];
                                            src2 = abstogrid(para[0]); dst2 = abstogrid(para[1]);
                                        }
                                        p1_abs = new Point(p2_abs.X, p1_abs.Y);
                                        p1_grid = abstogrid(p1_abs);
                                        p1_rel = getRelateCoord(p1_abs);
                                        
                                    }
                                    else // Y가 더 클경우 Y쪽으로 일치
                                    {
                                        src = p1_grid; dst = new Point(p1_grid.X, p2_grid.Y);

                                        if (drawingtype_2p == Drawing_Type_2P.DOUBLELINE)
                                        {
                                            Point[] para = Get_Parallel_Line(p1_abs, new Point(p1_abs.X, p2_abs.Y));
                                            prev_parallel_dst = para[1];
                                            src2 = abstogrid(para[0]); dst2 = abstogrid(para[1]);
                                        }
                                        p1_abs = new Point(p1_abs.X, p2_abs.Y);
                                        p1_grid = abstogrid(p1_abs);
                                        p1_rel = getRelateCoord(p1_abs);
                                        
                                    }
                                }
                                // 그리기
                               
                                break;
                            case Drawing_Type_2P.RECTANGLE: // 정사각 및 정원
                                int gap = Math.Max(Math.Abs(p1_abs.X - curpos_abs.X), Math.Abs(p1_abs.Y - curpos_abs.Y));
                                int xdir = (curpos_abs.X - p1_abs.X); if (xdir != 0) xdir /= Math.Abs(xdir);
                                int ydir = (curpos_abs.Y - p1_abs.Y); if (ydir != 0) ydir /= Math.Abs(ydir);
                                src = abstogrid(p1_abs);
                                Point temp = new Point(p1_abs.X + gap * xdir, p1_abs.Y + gap * ydir);
                                dst = abstogrid(temp);
                                
                                break;

                            case Drawing_Type_2P.CIRCLE:
                                break;
                            default:
                                break;
                        }
                    }
                    else // SHIFT 안눌리면
                    {
                        src = p1_grid; dst = p2_grid;
                       // module.Draw_2_Point(p1_grid.X, p1_grid.Y, p2_grid.X, p2_grid.Y, Convert_DrawingType2p_to_Int(drawingtype_2p), Convert_DrawingType_Object_to_Int(drawingtype_object));
                        if (drawingtype_2p == Drawing_Type_2P.DOUBLELINE)
                        {
                            Point[] para = Get_Parallel_Line(p1_abs, p2_abs);
                            prev_parallel_dst = para[1];
                            src2 = abstogrid(para[0]); dst2 = abstogrid(para[1]);
                        }
                        
                        p1_abs = new Point(p2_abs.X, p2_abs.Y);
                        p1_grid = abstogrid(p1_abs);
                        p1_rel = getRelateCoord(p1_abs);
                        
                    }
                    switch (drawingtype_2p)
                    {
                        case Drawing_Type_2P.LINE:
                            module.Draw_2_Point(src.X, src.Y, dst.X, dst.Y, Convert_DrawingType2p_to_Int(drawingtype_2p), Convert_DrawingType_Object_to_Int(drawingtype_object), Convert.ToInt32(comboBox_layer.Text));
                            break;
                        case Drawing_Type_2P.DOUBLELINE:
                            module.Draw_2_Point(src.X, src.Y, dst.X, dst.Y, Convert_DrawingType2p_to_Int(drawingtype_2p), Convert_DrawingType_Object_to_Int(drawingtype_object), Convert.ToInt32(comboBox_layer.Text));
                            module.Draw_2_Point(src2.X, src2.Y, dst2.X, dst2.Y, Convert_DrawingType2p_to_Int(drawingtype_2p), Convert_DrawingType_Object_to_Int(drawingtype_object), Convert.ToInt32(comboBox_layer.Text));
                            parallel_cont = true;
                            break;
                        case Drawing_Type_2P.RECTANGLE:
                            Point[] res = get_oriented_rect(src, dst);
                            module.Draw_2_Point(res[0].X, res[0].Y, res[1].X, res[1].Y, Convert_DrawingType2p_to_Int(drawingtype_2p), Convert_DrawingType_Object_to_Int(drawingtype_object), Convert.ToInt32(comboBox_layer.Text));
                            module.Draw_2_Point(res[1].X, res[1].Y, res[3].X, res[3].Y, Convert_DrawingType2p_to_Int(drawingtype_2p), Convert_DrawingType_Object_to_Int(drawingtype_object), Convert.ToInt32(comboBox_layer.Text));
                            module.Draw_2_Point(res[3].X, res[3].Y, res[2].X, res[2].Y, Convert_DrawingType2p_to_Int(drawingtype_2p), Convert_DrawingType_Object_to_Int(drawingtype_object), Convert.ToInt32(comboBox_layer.Text));
                            module.Draw_2_Point(res[2].X, res[2].Y, res[0].X, res[0].Y, Convert_DrawingType2p_to_Int(drawingtype_2p), Convert_DrawingType_Object_to_Int(drawingtype_object), Convert.ToInt32(comboBox_layer.Text));
                            mode = MouseMode.DRAWING_START;
                            break;
                        case Drawing_Type_2P.CIRCLE: mode = MouseMode.DRAWING_START;  break;
                        default: break;
                    }
                    break;
                case MouseMode.DRAW_MONITOR_DST: mode = MouseMode.DRAW_MONITOR_ANGLE; break;
                case MouseMode.DRAW_MONITOR_ANGLE:
                    double streamlength = Math.Sqrt(Math.Pow(p2_grid.X - p1_grid.X, 2) + Math.Pow(p2_grid.Y - p1_grid.Y, 2)) * module.Get_Distance_Per_Grid(); // grid to length
                    module.Add_Monitor(p1_grid.X, p2_grid.X, p3_grid.X, p1_grid.Y, p2_grid.Y, p3_grid.Y, streamlength, 0);
                    mode = MouseMode.DEFAULT;
                    ReleaseChart();
                    break;
                default: break;
            }
            UpdateImage();
        }
        #endregion


        private Point[] get_oriented_rect(Point p1, Point p4)
        {
            Point[] res = new Point[4];
            Point p1n = new Point(), p4n = new Point();
            Point p1_rot = new Point(), p4_rot = new Point();
            Point p2_rot = new Point(), p3_rot = new Point();
            Point p2 = new Point(), p3 = new Point();

            double theta = angle_of_rect * Math.PI / 180; // deg to rad
            if (-0.01 < theta && theta < 0.01)
                theta = 0;

            Point center = new Point((p1.X + p4.X) / 2, (p1.Y + p4.Y) / 2); // 중앙점
            p1n.X = p1.X - center.X; p4n.X = p4.X - center.X; p1n.Y = p1.Y - center.Y; p4n.Y = p4.Y - center.Y; // 좌표 표준화
            // rotation 매트릭스 적용
            p1_rot.X = (int)(p1n.X * Math.Cos(theta) + p1n.Y * Math.Sin(theta));
            p1_rot.Y = (int)(-p1n.X * Math.Sin(theta) + p1n.Y * Math.Cos(theta));
            p4_rot.X = (int)(p4n.X * Math.Cos(theta) + p4n.Y * Math.Sin(theta));
            p4_rot.Y = (int)(-p4n.X * Math.Sin(theta) + p4n.Y * Math.Cos(theta));
            p2_rot = new Point(p1_rot.X, p4_rot.Y);
            p3_rot = new Point(p4_rot.X, p1_rot.Y);
            // inverse rotation 적용
            p2.X = (int)(p2_rot.X * Math.Cos(theta) - p2_rot.Y * Math.Sin(theta)) + center.X;
            p2.Y = (int)(p2_rot.X * Math.Sin(theta) + p2_rot.Y * Math.Cos(theta)) + center.Y;
            p3.X = (int)(p3_rot.X * Math.Cos(theta) - p3_rot.Y * Math.Sin(theta)) + center.X;
            p3.Y = (int)(p3_rot.X * Math.Sin(theta) + p3_rot.Y * Math.Cos(theta)) + center.Y;
            res[0] = p1; res[1] = p2; res[2] = p3; res[3] = p4;

            return res;
        }

        #region 마우스 휠 함수
        private void DisplayLot_MouseWheel(object sender, MouseEventArgs e)
        {
            if (focused && e.Delta != 0)
            {
                ZoomScroll(e.Location, e.Delta > 0);
            }
            Console.WriteLine("Loc : {0:D}, {1:D}", e.Location.X, e.Location.Y);
        }
        #endregion

        #region 마우스 이동
        private void DisplayLot_MouseMove(object sender, MouseEventArgs e)
        {
            if (mode != MouseMode.DEFAULT)
            {
                curpos_mouse = new Point(e.X, e.Y);
                curpos_abs = getAbsCoord(e.X, e.Y);
                curpos_rel = getRelateCoord(curpos_abs);       
                if (mode == MouseMode.TRIM)
                {
                    if (LButton_Pushed)
                    {
                        Point curpos_grid = abstogrid(curpos_abs);
                        if (module.Trim_Line(curpos_grid.X, curpos_grid.Y) == 1) // trim이 적용된 상태면
                        {
                            UpdateImage();
                            Invalidate();
                            Update();
                        }
                    }
                }
            }
            
        }
        #endregion

        #region 줌인과 줌아웃
        private void ZoomScroll(Point loc, bool zoomIn)
        {
            int sx, sy, width, height;
            Point p = getAbsCoord(loc.X, loc.Y);
            sx = p.X;
            sy = p.Y;

            if (zoomIn)
                Zoomlevel++;
            else
                Zoomlevel--;

            if (Zoomlevel >= MAX_Zoom_Level)
            {
                Zoomlevel = 7;
                return;
            }
            else if (Zoomlevel < 0)
            {
                Zoomlevel = 0;
                return;
            }
            width = (int)(displaying.Width * Math.Pow(Zoommag, Zoomlevel));
            height = (int)(displaying.Height * Math.Pow(Zoommag, Zoomlevel));
            sx -= width / 2;
            sy -= height / 2;

            if (sx < 0)
                sx = 0;
            else if (sx + width > displaying.Width)
                sx = displaying.Width - width;
            if (sy < 0)
                sy = 0;
            else if (sy + height > displaying.Height)
                sy = displaying.Height - height;
            start_x = sx;
            start_y = sy;

            Croparea.X = sx; Croparea.Y = sy; Croparea.Width = width; Croparea.Height = height;

            cur_img = cropImage(displaying, Croparea);
            cur_img = resizeImage(cur_img, Picbox_size);
            

            p1_rel = getRelateCoord(p1_abs); 
            curpos_rel = getRelateCoord(curpos_abs);
            initp_rel = getRelateCoord(initp_abs);
            DisplayLot.Invalidate();
        }
        #endregion


        #region 이미지 ROI 설정 (절단, 내부함수)
        private static Image cropImage(Image img, Rectangle cropArea)
        {
            //Bitmap bmpImage = new Bitmap(img);// (Bitmap)(img); //new Bitmap(img);
            Image clone = ((Bitmap)(img)).Clone(cropArea, img.PixelFormat) as Image;
            //bmpImage.Dispose();
            return clone;
        }
        #endregion

        #region 마우스 좌표로부터 절대좌표 얻기 (getabscoord)
        private Point getAbsCoord(int mouse_x, int mouse_y)
        {
            int x,y;
            x = (int)(start_x + Math.Pow(Zoommag, Zoomlevel) * mouse_x / mag_x);
            y = (int)(start_y + Math.Pow(Zoommag, Zoomlevel) * mouse_y / mag_y);
            Point p = new Point();
            p.X = x;
            p.Y = y;
            return p;
        }
        private Point getRelateCoord(Point abscoord)
        {
            Point p = new Point();
            p.X = (int)((abscoord.X - start_x) * mag_x / Math.Pow(Zoommag, Zoomlevel));
            p.Y = (int)((abscoord.Y - start_y) * mag_y / Math.Pow(Zoommag, Zoomlevel));
            return p;
        }
        private Point abstogrid(Point abscoord)    // 픽셀 좌표로부터 그리드 좌표 얻기
        {
            Point p = new Point();
            p.X = (int)Math.Round((double)(abscoord.X / GRIDSIZE));
            p.Y = (int)Math.Round((double)(abscoord.Y / GRIDSIZE));
            return p;
        }


#endregion


        #region Datagrid 갱신 (소방장비)
        private void ReleaseChart()
        {
           Chart.Rows.Clear();
           int t0,t1,t2,t3,t4,t5;
           t0 = 0; t1 = 0; t2 = 0; t3 = 0; t4 = 0; t5 = 0;
           for(int i = 0; i < module.Get_numofitems(); i++)
           {
               bool read_only = false;
               string name = "";
               int num = 0, sourcenum, type, colorindex; double hoselength, streamlength;
               bool activated = module.Get_Hyd_activated(i);
               hoselength = module.Get_Hyd_length(i);
               streamlength = module.Get_Hyd_streamlength(i);
               type = module.Get_Hyd_type(i);
               sourcenum = module.Get_Hyd_sourcenum(i);
               colorindex = module.Get_Hyd_colorindex(i);
               string colname = "";
               switch(type)
               {
                   case 0: num = t0; t0++; name = "소화전"; break;
                   case 1: num = t1; t1++; name = "소화전(폼)"; break;
                   case 2: num = t2; t2++; name = "소화전(파우더)"; break;
                   case 3: num = t3; t3++; name = "호스릴"; break;
                   case 4: num = t4; t4++; name = "소화기"; break;
                   case 5: num = t5; t5++; name = "모니터"; read_only = true; break;
                   default: break;
               }
               switch(colorindex)
               {
                   case 0: colname = "Color0"; break;
                   case 1: colname = "Color1"; break;
                   case 2: colname = "Color2"; break;
                   case 3: colname = "Color3"; break;
                   case 4: colname = "Color4"; break;
                   case 5: colname = "Color5"; break;
                   case 6: colname = "Color6"; break;
                   case 7: colname = "Color7"; break;
                   case 8: colname = "Color8"; break;
                   case 9: colname = "Color9"; break;
                   default: break;
               }

               int x = 0, y = 0;
               x = module.Get_Hyd_xpos(i);
               y = module.Get_Hyd_ypos(i);

               string[] str = new string[] {activated.ToString(), name, num.ToString(), hoselength.ToString(), streamlength.ToString(), sourcenum.ToString(), colname, x.ToString(), y.ToString() };
               Chart.Rows.Add(str);
               if(read_only)
               {
                   int maxindex = Chart.Rows.Count - 1;
                   Chart.Rows[maxindex].Cells[1].ReadOnly = true; // 종류 변경 불가
                   Chart.Rows[maxindex].Cells[2].ReadOnly = true; // 호스길이 변경 불가
                   Chart.Rows[maxindex].Cells[3].ReadOnly = true; // sourcenum 변경 불가
                   Chart.Rows[maxindex].Cells[5].ReadOnly = true; // sourcenum 변경 불가
               }
           }
        }
        #endregion
        #region datagrid 갱신 (계단)
        private void ReleaseStairChart()
        {
            stairchart.Rows.Clear();
            for (int i = 0; i < module.Get_numofstair(); i++)
            {
                double penalty = module.Get_stair_penalty(i);
                string[] str = new string[] { i.ToString(), penalty.ToString()};
                stairchart.Rows.Add(str);//str);
            }
        }
        #endregion





        private Point[] Get_Parallel_Line(Point p1, Point p2)
        {
            Point[] res = new Point[2];
            double parallel_width_grid = parallel_width / module.Get_Distance_Per_Grid();

            double angle = Math.Atan2(p2.Y - p1.Y, p2.X - p1.X); // rad to deg, 초기각도

            if (-0.1 < angle && angle < 0.1)
                angle = 0;

            if (parallel_direction == true)  //  + 90도
                angle += Math.PI / 2;
            else
                angle -= Math.PI / 2; // false면 -90도
            res[0] = new Point(p1.X + (int)(parallel_width_grid * Math.Cos(angle)), p1.Y + (int)(parallel_width_grid * Math.Sin(angle)));
            if(parallel_cont == true)
            {
                res[0] = prev_parallel_dst;
            }
            res[1] = new Point(p2.X + (int)(parallel_width_grid * Math.Cos(angle)), p2.Y + (int)(parallel_width_grid * Math.Sin(angle)));

            return res;
        }

        #region PictureBox 화면갱신
        private void DisplayLot_Paint(object sender, PaintEventArgs e)
        {
            if (displaying == null) // 표시할게 없으면 return
                return;

            // 이 부분 조정 필요
            DisplayLot.SizeMode = PictureBoxSizeMode.StretchImage;
            DisplayLot.Image = cur_img;
            //     DisplayLot.Invalidate();
            //     DisplayLot.Update();
            // 이 부분 조정 필요


            Graphics g = e.Graphics;

            int rectsize = 0; // 보여줄 그림 요소의 크기
            Rectangle rect, rect2;
            Color col;
            SolidBrush br;
            Pen pen;
            switch (mode)
            {
                case MouseMode.ADD_HYD:
                case MouseMode.MOVE_HYD:
                case MouseMode.DRAWING_START:
                case MouseMode.DRAW_MONITOR_POS:
                    rectsize = 2 * (int)(mag_x * module.Get_gridsize() / Math.Pow(Zoommag, Zoomlevel)); // 보여줄 사각형의 크기
                    rect = new Rectangle(curpos_mouse.X - rectsize / 2, curpos_mouse.Y - rectsize / 2, rectsize, rectsize);
                    g.DrawRectangle(Pens.Red, rect);
                    break;
                case MouseMode.DRAW_SCALELINE_CONT:
                case MouseMode.DRAWING_CONT:
                case MouseMode.DRAW_MONITOR_DST: // p1_rel과 curpos_rel은 다른 과정에서 정의됨
                    rectsize = (int)(mag_x * module.Get_gridsize() / Math.Pow(Zoommag, Zoomlevel));//    p1_rel = getRelateCoord(p1_abs);
                    rect = new Rectangle(p1_rel.X - rectsize / 2, p1_rel.Y - rectsize / 2, rectsize, rectsize);
                    rect2 = new Rectangle(curpos_rel.X - rectsize / 2, curpos_rel.Y - rectsize / 2, rectsize, rectsize);
                    col = System.Drawing.Color.FromArgb(0, 0, 255); // 파란색
                    if (mode == MouseMode.DRAWING_CONT) // 그리기 모드일 경우
                    {
                        switch (drawingtype_object)
                        {
                            case Drawing_Type_Object.WALL:
                                col = System.Drawing.Color.FromArgb(module.Get_Color_Wall_R(), module.Get_Color_Wall_G(), module.Get_Color_Wall_B());
                                break;
                            case Drawing_Type_Object.OBSTACLE:
                                col = System.Drawing.Color.FromArgb(module.Get_Color_Obstacle_R(layernum), module.Get_Color_Obstacle_G(layernum), module.Get_Color_Obstacle_B(layernum));
                                break;
                            case Drawing_Type_Object.PATH:
                                col = System.Drawing.Color.FromArgb(module.Get_Color_Path_R(), module.Get_Color_Path_G(), module.Get_Color_Path_B());
                                break;
                            case Drawing_Type_Object.EQUIPMENT:
                                col = System.Drawing.Color.FromArgb(module.Get_Color_Equipment_R(), module.Get_Color_Equipment_G(), module.Get_Color_Equipment_B());
                                break;
                            default: break;
                        }
                    }
                    br = new SolidBrush(col);
                    pen = new Pen(br);
                    pen.Width = rectsize;
                    g.FillRectangle(br, rect);

                    Point src = new Point(), dst = new Point();
                    Point src2 = new Point(), dst2 = new Point();

                    if (mode == MouseMode.DRAWING_CONT || mode == MouseMode.DRAW_MONITOR_DST)
                    {
                        if (KEYPRESSED_SHIFT)
                        {
                            switch (drawingtype_2p)
                            {
                                case Drawing_Type_2P.LINE:
                                case Drawing_Type_2P.DOUBLELINE:
                                    if (Math.Sqrt(Math.Pow(initp_abs.X - curpos_abs.X, 2) + Math.Pow(initp_abs.Y - curpos_abs.Y, 2)) < GRIDSIZE * 5) // 매우 근접해 있으면 마그넷시킴.
                                    {
                                        Pen pen2 = Pens.Red;
                                        rectsize = rectsize * 4;
                                        Rectangle rect3 = new Rectangle(initp_rel.X - rectsize / 2, initp_rel.Y - rectsize / 2, rectsize, rectsize);
                                        g.DrawEllipse(pen2, rect3);

                                        src = p1_rel; dst = initp_rel;
                                        if (drawingtype_2p == Drawing_Type_2P.DOUBLELINE)
                                        {
                                            Point[] para = Get_Parallel_Line(p1_abs, initp_abs);
                                            src2 = getRelateCoord(para[0]); dst2 = getRelateCoord(para[1]);
                                        }
                                    }
                                    else
                                    {
                                        if (Math.Abs(p1_abs.X - curpos_abs.X) > Math.Abs(p1_abs.Y - curpos_abs.Y)) // X쪽으로 일치시켜야함
                                        {
                                            src = p1_rel; dst = new Point(curpos_rel.X, p1_rel.Y);
                                            if (drawingtype_2p == Drawing_Type_2P.DOUBLELINE)
                                            {
                                                Point[] para = Get_Parallel_Line(p1_abs, new Point(curpos_abs.X, p1_abs.Y));
                                                src2 = getRelateCoord(para[0]); dst2 = getRelateCoord(para[1]);
                                            }
                                        }
                                        else // Y가 더 클경우 Y쪽으로 일치
                                        {
                                            src = p1_rel; dst = new Point(p1_rel.X, curpos_rel.Y);
                                            if (drawingtype_2p == Drawing_Type_2P.DOUBLELINE)
                                            {
                                                Point[] para = Get_Parallel_Line(p1_abs, new Point(p1_abs.X, curpos_abs.Y));
                                                src2 = getRelateCoord(para[0]); dst2 = getRelateCoord(para[1]);
                                            }
                                        }
                                    }
                                    break;
                                case Drawing_Type_2P.RECTANGLE: // 정사각 및 정원
                                case Drawing_Type_2P.CIRCLE:
                                    int gap = Math.Max(Math.Abs(p1_abs.X - curpos_abs.X), Math.Abs(p1_abs.Y - curpos_abs.Y));
                                    int xdir = (curpos_abs.X - p1_abs.X); if(xdir != 0) xdir /= Math.Abs(xdir);
                                    int ydir = (curpos_abs.Y - p1_abs.Y); if(ydir != 0) ydir /= Math.Abs(ydir);
                                    src = p1_rel;
                                    Point temp = new Point(p1_abs.X + gap * xdir, p1_abs.Y + gap * ydir);
                                    dst = getRelateCoord(temp);
                                    break;
                                default: break;
                            }
                        }
                        else
                        {
                            src = p1_rel; dst = curpos_rel;
                            if (drawingtype_2p == Drawing_Type_2P.DOUBLELINE)
                            {
                                Point[] para = Get_Parallel_Line(p1_abs, curpos_abs);
                                src2 = getRelateCoord(para[0]); dst2 = getRelateCoord(para[1]);
                            }
                        }
                        if (mode == MouseMode.DRAWING_CONT)
                        {
                            switch (drawingtype_2p)
                            {
                                case Drawing_Type_2P.LINE: g.DrawLine(pen, src, dst); break;
                                case Drawing_Type_2P.DOUBLELINE: g.DrawLine(pen, src, dst); g.DrawLine(pen, src2, dst2); break;
                                case Drawing_Type_2P.RECTANGLE:
                                case Drawing_Type_2P.CIRCLE:
                                    Rectangle dRect = new Rectangle(Math.Min(src.X, dst.X), Math.Min(src.Y, dst.Y), Math.Abs(src.X - dst.X), Math.Abs(src.Y - dst.Y));
                                    if (drawingtype_2p == Drawing_Type_2P.RECTANGLE)
                                    {
                                        Point[] res = get_oriented_rect(src, dst);
                                        g.DrawLine(pen, res[0], res[1]);
                                        g.DrawLine(pen, res[1], res[3]);
                                        g.DrawLine(pen, res[3], res[2]);
                                        g.DrawLine(pen, res[2], res[0]);
                                    }
                                    else if (drawingtype_2p == Drawing_Type_2P.CIRCLE)
                                        g.DrawEllipse(pen, dRect);
                                    break;
                                default: break;
                            }
                        }
                        else if (mode == MouseMode.DRAW_MONITOR_DST)
                        {
                            g.DrawLine(pen, src, dst); break;
                        }
                        g.FillRectangle(br, rect2);
                    }
                    else if (mode == MouseMode.DRAW_SCALELINE_CONT)
                    {
                        g.DrawLine(pen, p1_rel, new Point(curpos_rel.X, p1_rel.Y));
                    }
                    break;
                case MouseMode.DRAW_MONITOR_ANGLE:
                    p1_rel = getRelateCoord(p1_abs);
                    p2_rel = getRelateCoord(p2_abs);
                    curpos_rel = getRelateCoord(curpos_abs);
                    rectsize = (int)(mag_x * module.Get_gridsize() / Math.Pow(Zoommag, Zoomlevel));
                    rect = new Rectangle(p1_rel.X - rectsize / 2, p1_rel.Y - rectsize / 2, rectsize, rectsize);
                    rect2 = new Rectangle(p2_rel.X - rectsize / 2, p2_rel.Y - rectsize / 2, rectsize, rectsize);
                    int distance = (int)Math.Sqrt(Math.Pow(p2_rel.X - p1_rel.X, 2) + Math.Pow(p2_rel.Y - p1_rel.Y, 2));
                    Rectangle rect_for_arc = new Rectangle(p1_rel.X - distance, p1_rel.Y - distance, distance * 2, distance * 2);
                    col = System.Drawing.Color.FromArgb(0, 0, 255);
                    br = new SolidBrush(col);
                    pen = new Pen(br);
                    g.FillRectangle(br, rect);
                    g.FillRectangle(br, rect2);
                    g.DrawLine(pen, p1_rel, p2_rel);
                    double angle1 = Math.Atan2(p2_abs.Y - p1_abs.Y, p2_abs.X - p1_abs.X) * 180 / 3.1415; // -pi ~ pi의 rad값 반
                    double angle2 = Math.Atan2(curpos_abs.Y - p1_abs.Y, curpos_abs.X - p1_abs.X) * 180 / 3.1415; // -pi ~ pi의 rad값 반
                    double sweep = angle2 - angle1;
                    if (sweep < 0)
                        sweep += 360;
                    g.DrawArc(pen, rect_for_arc, (float)angle1, (float)sweep);
                    break;
                case MouseMode.TRIM:
                    rectsize = (int)(mag_x * module.Get_gridsize() / Math.Pow(Zoommag, Zoomlevel));
                    rect = new Rectangle(curpos_rel.X - rectsize, curpos_rel.Y - rectsize, rectsize * 2, rectsize * 2);
                    col = System.Drawing.Color.FromArgb(0, 0, 0);
                    br = new SolidBrush(col);
                    pen = new Pen(br);
                    g.FillRectangle(br, rect);
                    break;
                default: break;
            }
        }
        #endregion


        #region UpdateImage
        private void UpdateImage()
        {
            if (!Initialized)
                return;
            
            byte[] path = Encoding.ASCII.GetBytes(DefaultPath);
            unsafe
            {
                if(displaying != null)
                    displaying.Dispose();
                if (cur_img != null)
                    cur_img.Dispose();
                if (DisplayLot.Image != null)
                   DisplayLot.Image.Dispose();
            //    if (DisplayLot != null)
            //        DisplayLot.Dispose();
                //디스플레이 마스크 체크
                int displaycode = 0;
                if (check_stream.Checked)
                    displaycode += DISPLAYSTREAM;
                if (check_hose.Checked)
                    displaycode += DISPLAYPATH;
                if (check_text.Checked)
                    displaycode += DISPLAYTEXT;
                if (radio_overlap.Checked)
                    displaycode += DISPLAYOVERLAP;
                if (radio_single.Checked)
                    displaycode += DISPLAYSINGLE;
                if (guideline.Checked)
                    displaycode += DISPLAYGUIDELINE;
                if (check_missingline.Checked)
                    displaycode += DISPLAYMISSINGLINE;
                if (check_displaydrawing.Checked) // 디스플레이 모드 상태
                    displaycode += DISPLAYBACKGROUNDDRAWING;
                if (check_simplecheck.Checked)
                    displaycode += DISPLAY_SIMPLECHECK;

                if (groupBox_Optimization.Enabled == true)
                {
                    if(comboBox_Detection_Check.SelectedIndex != 0)
                    {
                        displaycode += DISPLAY_DETECTION_CHECK;
                        switch(comboBox_Detection_Check.SelectedIndex)
                        {
                            case 1: displaycode += DISPLAY_DETECTION_STREAMABLE; break;
                            case 2: displaycode += DISPLAY_DETECTION_FLOODABLE; break;
                            case 3: displaycode += DISPLAY_DETECTION_FEASIBLE; break;
                            case 4: displaycode += DISPLAY_DETECTION_PATH; break;
                            case 5: displaycode += DISPLAY_DETECTION_DOOR; break;
                            case 6: displaycode += DISPLAY_DETECTION_PREFERENCE; break;

                            default: break;
                        }
                    }

                }


                int ind = 0;
                fixed (byte* path1 = path) // 만약에 오류생길경우 fixed처리
                {
                    sbyte* sim_path = (sbyte*)path1;
                    ind = module.saveimg(displaycode, sim_path);
                }
                string str = DefaultPath;
                str += "deck";
                str += Convert.ToString(ind);
                str += ".png";
               // Image img = Image.FromFile(str);
               // displaying = img.Clone() as Image;
                displaying = Image.FromFile(str);
                cur_img = cropImage(displaying, Croparea);
                //displaying.Dispose();
                cur_img = resizeImage(cur_img, Picbox_size);
                DisplayLot.Image = cur_img;

                
              //img.Dispose();
                DisplayLot.Invalidate();
            }
        }
        #endregion

        #region getindex
        private int getindex(int RowIndex, bool ori = false)
        {
            int index = 0;
            string str = Chart.Rows[RowIndex].Cells[1].Value.ToString();
            if (ori == true)
                str = ori_str;
            
            string str2 = Chart.Rows[RowIndex].Cells[2].Value.ToString();
            int num = Convert.ToInt32(str2);
            if (str.Equals("소화전")) { index = num; }
            else if (str.Equals("소화전(폼)")) { index = module.Get_numofhydrant() + num; }
            else if (str.Equals("소화전(파우더)")) { index = module.Get_numofhydrant() + module.Get_numofhydrant_form() + num; }
            else if (str.Equals("호스릴")) { index = module.Get_numofhydrant() + module.Get_numofhydrant_form() + module.Get_numofhydrant_powder() + num; }
            else if (str.Equals("소화기")) { index = module.Get_numofhydrant() + module.Get_numofhydrant_form() + module.Get_numofhydrant_powder() + module.Get_numofhosereel() + num; }
            else if (str.Equals("모니터")) { index = module.Get_numofhydrant() + module.Get_numofhydrant_form() + module.Get_numofhydrant_powder() + module.Get_numofhosereel() + module.Get_numofextinguisher() + num; }
          
            return index;
        }
        #endregion

        #region 셀 수정
        private void Chart_CellValueChanged(object sender, DataGridViewCellEventArgs e)
        {
            bool Update_Immediately = check_immediately_update.Checked;
            if (!Initialized)
                return;
            if (e.RowIndex == -1)
                return;
            if (e.ColumnIndex == 0)
                return;
            int index = getindex(e.RowIndex);    

            if (e.ColumnIndex == 1)
            {
                index = getindex(e.RowIndex,true);   
                string str = Chart.Rows[e.RowIndex].Cells[1].EditedFormattedValue.ToString();
                int type = 0;
                if (str.Equals("호스릴"))
                    type = 3;
                else if (str.Equals("소화기"))
                    type = 4;
                else if (str.Equals("소화전(폼)"))
                    type = 1;
                else if (str.Equals("소화전(파우더)"))
                    type = 2;
                else if (str.Equals("소화전"))
                    type = 0;
                module.Set_Hyd_Parameters(index, -1, -1, -1, -1, -1, -1,  type, -1, -1, Update_Immediately);
                ReleaseChart();
            }
            else if (e.ColumnIndex == 3) // 호스 길이 변경
            {
                string str = Chart.Rows[e.RowIndex].Cells[3].EditedFormattedValue.ToString();
                double newlength = Convert.ToDouble(str);
                module.Set_Hyd_Parameters(index, -1, -1, newlength, -1, -1, -1, -1, -1, -1, Update_Immediately);
            }
            else if (e.ColumnIndex == 4) // 스트림 길이 변경
            {
                string str = Chart.Rows[e.RowIndex].Cells[4].EditedFormattedValue.ToString();
                double newlength = Convert.ToDouble(str);
                module.Set_Hyd_Parameters(index, -1, -1, -1, newlength, -1, -1, -1, -1, -1, Update_Immediately);
            }
            else if (e.ColumnIndex == 5) // sourcenum
            {
                string str = Chart.Rows[e.RowIndex].Cells[5].EditedFormattedValue.ToString();
                int sourcenum = Convert.ToInt32(str);
                module.Set_Hyd_Parameters(index, -1, -1, -1, -1, sourcenum, -1, -1, -1, -1, Update_Immediately);
            }
            else if (e.ColumnIndex == 6) // colorindex
            {
                string str = Chart.Rows[e.RowIndex].Cells[6].EditedFormattedValue.ToString();
                int colorindex = 0;
                if (str.Equals("Color0"))
                    colorindex = 0;
                else if (str.Equals("Color1"))
                    colorindex = 1;
                else if (str.Equals("Color2"))
                    colorindex = 2;
                else if (str.Equals("Color3"))
                    colorindex = 3;
                else if (str.Equals("Color4"))
                    colorindex = 4;
                else if (str.Equals("Color5"))
                    colorindex = 5;
                else if (str.Equals("Color6"))
                    colorindex = 6;
                else if (str.Equals("Color7"))
                    colorindex = 7;
                else if (str.Equals("Color8"))
                    colorindex = 8;
                else if (str.Equals("Color9"))
                    colorindex = 9;
                module.Set_Hyd_Parameters(index, -1, -1, -1, -1, -1, colorindex, -1, -1, -1, Update_Immediately);
            }
            UpdateImage();
        }

        private void stairchart_CellValueChanged(object sender, DataGridViewCellEventArgs e)
        {
            if (!Initialized)
                return;
            if (e.RowIndex == -1 || e.ColumnIndex == -1)
                return;
            string str = stairchart.Rows[e.RowIndex].Cells[1].EditedFormattedValue.ToString(); // 패널티 값 읽기
            double penalty = Convert.ToDouble(str);
            str = stairchart.Rows[e.RowIndex].Cells[0].EditedFormattedValue.ToString();
            int num = Convert.ToInt32(str);
            module.Set_Stair_Penalty(num, penalty);
            UpdateImage();
        }
        #endregion
        #region 셀 체크박스 클릭
        private void Chart_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {
            if (e.RowIndex == -1)
                return;
            if (e.ColumnIndex != 0)
                return;
           
            string str = Chart.Rows[e.RowIndex].Cells[0].EditedFormattedValue.ToString();
            bool visualize = Convert.ToBoolean(str);
            int index = getindex(e.RowIndex);
            module.Set_Hyd_Parameters(index,  -1, -1, -1, -1, -1, -1, -1, Convert.ToInt32(visualize), -1, false);

            UpdateImage();
        }
        #endregion
        #region 셀 클릭
        private void Chart_CellClick(object sender, DataGridViewCellEventArgs e)
        {
            
            if (e.RowIndex == -1 && e.ColumnIndex == -1)
            {
                ReleaseChart();
                return;
            }
            if (e.ColumnIndex == 1)
            {
                ori_str = Chart.Rows[e.RowIndex].Cells[1].EditedFormattedValue.ToString();
                return;
            }
               
            else if (e.RowIndex == -1 && e.ColumnIndex != -1)
                return;
            Unhighlight_All();
            int index = getindex(e.RowIndex);
            module.Set_Hyd_Parameters(index,  -1, -1, -1, -1,-1, -1, -1, -1, 1, false); // 하이라이트 시킴
            Chart.Rows[e.RowIndex].Selected = true;
            Hyd_Focused_Row = e.RowIndex;
            UpdateImage();
            
        }
        private void stairchart_CellClick(object sender, DataGridViewCellEventArgs e)
        {
            
            if (e.RowIndex == -1 || e.ColumnIndex == -1)
            {
                return;
            }
            Unhighlight_All();
            string str = stairchart.Rows[e.RowIndex].Cells[0].EditedFormattedValue.ToString();
            int num = Convert.ToInt32(str);
            module.Set_Highlighted_Stair(num);
            stairchart.Rows[e.RowIndex].Selected = true;
            UpdateImage();
        }
        #endregion

        #region Delete키 입력
        private void Chart_KeyDown(object sender, KeyEventArgs e)
        {
            if (Hyd_Focused_Row == -1)
                return;
        }
        #endregion


        
        
        private void Chart_CurrentCellChanged(object sender, EventArgs e)
        {
        }


        #region Dialog내에서 키동작 (눌림 혹은 뗌)
        private void Form1_KeyDown(object sender, KeyEventArgs e)
        {
            switch(e.KeyCode)
            {
                case Keys.Escape:
                    module.Unhighlight_Hyd();
                    if (Hyd_Focused_Row != -1)
                    {
                        Chart.Rows[Hyd_Focused_Row].Selected = false;
                        Hyd_Focused_Row = -1;
                    }
                    if (mode != MouseMode.DEFAULT)
                        mode = MouseMode.DEFAULT;
                    
                    break;
                case Keys.ShiftKey:
                    KEYPRESSED_SHIFT = true;//!KEYPRESSED_SHIFT;
                    break;
                case Keys.Z:
                    parallel_direction = !parallel_direction;
                    break;
                case Keys.Left:
                    angle_of_rect += 5; if(angle_of_rect >= 90) angle_of_rect -= 90;
                    break;
                case Keys.Right:
                    angle_of_rect -= 5; if(angle_of_rect <= -90) angle_of_rect += 90;
                    break;
                case Keys.Up:
                    parallel_width += 0.5;
                    break;
                case Keys.Down:
                    parallel_width -= 0.5;
                    break;

                case Keys.Delete:
                    if (Hyd_Focused_Row != -1)
                    {
                        string str2 = Chart.Rows[Hyd_Focused_Row].Cells[2].Value.ToString();
                        string refstr = Chart.Rows[Hyd_Focused_Row].Cells[1].Value.ToString();
                        int refnum = Convert.ToInt32(str2);
                        for (int i = 0; i < Chart.RowCount; i++)
                        {
                            string str = Chart.Rows[i].Cells[1].Value.ToString();
                            if (str.Equals(refstr))
                            {
                                str2 = Chart.Rows[i].Cells[2].Value.ToString();
                                int num = Convert.ToInt32(str2);
                                if (num > refnum)
                                    Chart.Rows[i].Cells[2].Value = num - 1;
                            }
                        }
                        int index = getindex(Hyd_Focused_Row);
                        Chart.Rows.Remove(Chart.Rows[Hyd_Focused_Row]);
                        module.Delete_Hyd(index);
                        Hyd_Focused_Row = -1;

                        UpdateImage();
                    }
                    break;
                default:break;
            }
        }

        private void Form1_KeyUp(object sender, KeyEventArgs e)
        {
            
            switch (e.KeyCode)
            {
                case Keys.ShiftKey:  // 쉬프트 키뗌
                    KEYPRESSED_SHIFT = false;
                    break;
                default: break;
            }
            
        }


        #endregion




        #region 소화전 추가 및 이동 버튼
        private void Add_Hydrant_Click(object sender, EventArgs e)
        {
            mode = MouseMode.ADD_HYD;
        }
        private void Move_hydrant_Click(object sender, EventArgs e)
        {
            if (Hyd_Focused_Row != -1)
                mode = MouseMode.MOVE_HYD;
            else
                Console.WriteLine("선택된 소방장비가 없음");
        }
        #endregion

        #region 체크박스 동작들
        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if (check_stream.Checked == true && check_hose.Checked == true)
                check_stream.Checked = false;
            if (Initialized)
                UpdateImage();
        }
        private void check_stream_CheckedChanged(object sender, EventArgs e)
        {
            if (check_stream.Checked == true && check_hose.Checked == true)
                check_hose.Checked = false;
            if (Initialized)
                UpdateImage();
        }
        private void radio_all_CheckedChanged(object sender, EventArgs e)
        {
            if (check_hose.Checked == true && check_stream.Checked == true)
            {
                check_stream.Checked = false;
            }
            if (Initialized)
                UpdateImage();
        }
        private void radio_single_CheckedChanged(object sender, EventArgs e)
        {
            if (check_hose.Checked == true && check_stream.Checked == true)
            {
                check_stream.Checked = false;
            }
            if (Initialized)
                UpdateImage();
        }
        private void radio_overlap_CheckedChanged(object sender, EventArgs e)
        {
            if (check_hose.Checked == true && check_stream.Checked == true)
            {
                check_stream.Checked = false;
            }
            if (Initialized)
                UpdateImage();
        }
        private void guideline_CheckedChanged(object sender, EventArgs e)
        {
            showguide = guideline.Checked;
            UpdateImage();
        }
        #endregion

        #region Report 생성
        private void MakeReport_Click(object sender, EventArgs e)
        {
            Excel.Application Excelapp = new Excel.Application();
            Excel.Workbook wb = Excelapp.Workbooks.Add(true);
            Excel._Worksheet workSheet = wb.Worksheets.get_Item(1) as Excel._Worksheet;


           // dlg.Close();

            excelpopup.popup dlg = new excelpopup.popup();
            dlg.Show();
            dlg.ReportView.Rows.Clear();
            dlg.Invalidate();
            dlg.ReportView.ColumnCount = module.Get_numofitems() + 2;
            dlg.ReportView.RowCount = module.Get_numofitems() * 2 + 10;
            


            workSheet.Cells[2, 1] = "path 중첩(m^2)";
            dlg.ReportView.Rows[1].Cells[0].Value = "path 중첩(m^2)";
            workSheet.Cells[3, 1] = "Item";
            dlg.ReportView.Rows[2].Cells[0].Value = "Item";
            
            int r = 3;
            int c = 2;
            int[] type = new int[3] { 0, 0, 0 };
            int f = 0;
            for (int k = 0; k < module.Get_numofitems(); k++)
            {
                if (!module.Get_Hyd_activated(k))
                    continue;
                f++;

                switch(module.Get_Hyd_type(k))
                {
                    case 0: workSheet.Cells[r, c] = String.Format("소화전{0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화전{0:D2}", module.Get_Hyd_index(k)); break;
                    case 1: workSheet.Cells[r, c] = String.Format("소화전(폼){0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화전(폼){0:D2}", module.Get_Hyd_index(k)); break;
                    case 2: workSheet.Cells[r, c] = String.Format("소화전(파우더){0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화전(파우더){0:D2}", module.Get_Hyd_index(k)); ; break;
                    case 3: workSheet.Cells[r, c] = String.Format("호스릴{0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("호스릴{0:D2}", module.Get_Hyd_index(k)); break;
                    case 4: workSheet.Cells[r, c] = String.Format("소화기{0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화기{0:D2}", module.Get_Hyd_index(k)); break;
                    default: c--;  break;
                }
                c++;
            }


            c = 1;
            r = 3;
            f = 0;
            for (int k = 0; k < module.Get_numofitems(); k++)
            {
                if (!module.Get_Hyd_activated(k))
                    continue;
                f++;
                r++;
                c = 1;

                switch (module.Get_Hyd_type(k))
                {
                    case 0: workSheet.Cells[r, c] = String.Format("소화전{0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화전{0:D2}", module.Get_Hyd_index(k)); break;
                    case 1: workSheet.Cells[r, c] = String.Format("소화전(폼){0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화전(폼){0:D2}", module.Get_Hyd_index(k)); break;
                    case 2: workSheet.Cells[r, c] = String.Format("소화전(파우더){0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화전(파우더){0:D2}", module.Get_Hyd_index(k)); ; break;
                    case 3: workSheet.Cells[r, c] = String.Format("호스릴{0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("호스릴{0:D2}", module.Get_Hyd_index(k)); break;
                    case 4: workSheet.Cells[r, c] = String.Format("소화기{0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화기{0:D2}", module.Get_Hyd_index(k)); break;
                    default: break;
                }


                int j = 0;
                for (int n = 0; n < module.Get_numofitems(); n++)
                {
                    if (!module.Get_Hyd_activated(n))
                        continue;
                    j++;
                    c++;
                    if (j < f + 1)
                    {
                        workSheet.Cells[r, c] = String.Format("-------");
                        dlg.ReportView.Rows[r-1].Cells[c-1].Value = String.Format("-------");
                    }
                    else
                    {
                        double area = module.Get_route_overlapped_area(k, n);
                        if (area != -1)
                        {
                            workSheet.Cells[r, c] = String.Format("{0:0.###}", area);
                            dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("{0:0.###}", area);
                        }
                        else
                        {
                            workSheet.Cells[r, c] = String.Format("--------");
                            dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("--------");
                        }
                        
                    }
                }
            }

            r += 2;
            c = 1;

            workSheet.Cells[r, c] = "stream 중첩(m^2)";
            dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = "stream 중첩(m^2)";
            r++;
            workSheet.Cells[r, c] = "Item";
            dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = "Item";
            c = 2;


           
            for (int k = 0; k < module.Get_numofitems(); k++)
            {
                if (!module.Get_Hyd_activated(k))
                    continue;
                switch (module.Get_Hyd_type(k))
                {
                    case 0: workSheet.Cells[r, c] = String.Format("소화전{0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화전{0:D2}", module.Get_Hyd_index(k)); break;
                    case 1: workSheet.Cells[r, c] = String.Format("소화전(폼){0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화전(폼){0:D2}", module.Get_Hyd_index(k)); break;
                    case 2: workSheet.Cells[r, c] = String.Format("소화전(파우더){0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화전(파우더){0:D2}", module.Get_Hyd_index(k)); ; break;
                    case 3: workSheet.Cells[r, c] = String.Format("호스릴{0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("호스릴{0:D2}", module.Get_Hyd_index(k)); break;
                    case 4: workSheet.Cells[r, c] = String.Format("소화기{0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화기{0:D2}", module.Get_Hyd_index(k)); break;
                    default: c--;  break;
                }
                c++;
            }
          
            c = 1;
            f = 0;
            for (int k = 0; k < module.Get_numofitems(); k++)
            {
                if (!module.Get_Hyd_activated(k))
                    continue;
                f++;
                r++;
                c = 1;
                switch (module.Get_Hyd_type(k))
                {
                    case 0: workSheet.Cells[r, c] = String.Format("소화전{0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화전{0:D2}", module.Get_Hyd_index(k)); break;
                    case 1: workSheet.Cells[r, c] = String.Format("소화전(폼){0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화전(폼){0:D2}", module.Get_Hyd_index(k)); break;
                    case 2: workSheet.Cells[r, c] = String.Format("소화전(파우더){0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화전(파우더){0:D2}", module.Get_Hyd_index(k)); ; break;
                    case 3: workSheet.Cells[r, c] = String.Format("호스릴{0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("호스릴{0:D2}", module.Get_Hyd_index(k)); break;
                    case 4: workSheet.Cells[r, c] = String.Format("소화기{0:D2}", module.Get_Hyd_index(k)); dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("소화기{0:D2}", module.Get_Hyd_index(k)); break;
                    default: break;
                }
                int j = 0;
                for (int n = 0; n < module.Get_numofitems(); n++)
                {
                    if (!module.Get_Hyd_activated(n))
                        continue;
                    j++;
                    c++;
                    if (j < f + 1)
                    {
                        workSheet.Cells[r, c] = String.Format("-------");
                        dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("-------");
                    }

                    else
                    {
                        double area = module.Get_stream_overlapped_area(k, n);
                        if (area != -1)
                        {
                            workSheet.Cells[r, c] = String.Format("{0:0.###}", area);
                            dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("{0:0.###}", area);
                        }
                        else
                        {
                            workSheet.Cells[r, c] = String.Format("--------");
                            dlg.ReportView.Rows[r - 1].Cells[c - 1].Value = String.Format("--------");
                        }
                    }
                }
            }
        
            // 액셀 릴리즈
            for (int i = 0; i < dlg.ReportView.Columns.Count; i++)
            {
                dlg.ReportView.AutoResizeColumn(i, DataGridViewAutoSizeColumnMode.AllCells);
            }
            ExcelDispose(Excelapp, wb, workSheet);
           
        }
            
            
        #region 액셀관련함수
        public static void ExcelDispose(Excel.Application excelApp, Excel.Workbook wb, Excel._Worksheet workSheet)
        {
            // 파일 삭제 부분

            System.IO.FileInfo fileDel = new System.IO.FileInfo(Application.StartupPath + @"\report.xls");
            if (fileDel.Exists) //삭제할 파일이 있는지
            {
                fileDel.Delete(); //없어도 에러안남
            }

            wb.SaveAs(Application.StartupPath + @"\report.xls", Excel.XlFileFormat.xlWorkbookNormal, Type.Missing, Type.Missing, Type.Missing, Type.Missing,
                Microsoft.Office.Interop.Excel.XlSaveAsAccessMode.xlExclusive, Type.Missing, Type.Missing, Type.Missing, Type.Missing, Type.Missing);

            wb.Close(Type.Missing, Type.Missing, Type.Missing);
            excelApp.Quit();
            releaseObject(excelApp);
            releaseObject(workSheet);
            releaseObject(wb);
        }

       
        private static void releaseObject(object obj)
        {
            try
            {
                System.Runtime.InteropServices.Marshal.ReleaseComObject(obj);
                obj = null;
            }
            catch (Exception e)
            {
                obj = null;
            }
            finally
            {
                GC.Collect();
            }
        }
        #endregion
        #endregion



        private void but_scalingline_Click(object sender, EventArgs e)
        {
            mode = MouseMode.DRAW_SCALELINE;
        }

        private void but_Undo_Click(object sender, EventArgs e)
        {
            module.Undo();
            UpdateImage();
            ReleaseChart();
        }

        private void but_Reset_Click(object sender, EventArgs e)
        {
            module.reset_drawing();
            Hyd_Focused_Row = -1;
            UpdateImage();
            ReleaseChart();
            
        }

      

        private void but_re_calculate_Click(object sender, EventArgs e)
        {
            module.Recalculate_All();
            display_step_by_step();
            UpdateImage();
        }

        private void comboBox_layer_SelectedIndexChanged(object sender, EventArgs e)
        {
            int layer = Convert.ToInt32(comboBox_layer.SelectedIndex);
            module.Set_Layer(layer);
            UpdateImage();
        }

        private void but_add_firemonitor_Click(object sender, EventArgs e)
        {
            mode = MouseMode.DRAW_MONITOR_POS;
        }

        private void check_text_CheckedChanged(object sender, EventArgs e)
        {
             UpdateImage();
        }

        private void but_cal_checked_only_Click(object sender, EventArgs e)
        {
            if (Initialized)
            {
                display_step_by_step();
            }
        }

        private void check_missingline_CheckedChanged(object sender, EventArgs e)
        {
                UpdateImage();
        }

        private void but_calculate_missingline_Click(object sender, EventArgs e)
        {
            if (Initialized)
            {
                module.detect_closed_loop();
                check_missingline.Checked = true;
                UpdateImage();
            }
        }

        private void check_displaydrawing_CheckedChanged(object sender, EventArgs e)
        {
               UpdateImage();
        }

     


        private void stairchart_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {

        }

        private void Cbox_progmode_SelectedIndexChanged(object sender, EventArgs e)
        {
            switch(Cbox_progmode.SelectedIndex)
            {
                case 0: prgmode = ProgMode.VISUALIZE; break;
                case 1: prgmode = ProgMode.DRAWING; break;
                case 2: prgmode = ProgMode.OPTIMIZATION; break;
                default: break;
            }
        }

        private void check_simplecheck_CheckedChanged(object sender, EventArgs e)
        {
            UpdateImage();
        }



        private void but_drawline_Click(object sender, EventArgs e)
        {
            mode = MouseMode.DRAWING_START;
            drawingtype_2p = Drawing_Type_2P.LINE;
            parallel_cont = false;
        }
        private void but_drawrect_Click(object sender, EventArgs e)
        {
            mode = MouseMode.DRAWING_START;
            drawingtype_2p = Drawing_Type_2P.RECTANGLE;
            parallel_cont = false;
        }
        private void but_drawcircle_Click(object sender, EventArgs e)
        {
            mode = MouseMode.DRAWING_START;
            drawingtype_2p = Drawing_Type_2P.CIRCLE;
            parallel_cont = false;
        }
        private void but_drawdoubleline_Click(object sender, EventArgs e)
        {
            mode = MouseMode.DRAWING_START;
            drawingtype_2p = Drawing_Type_2P.DOUBLELINE;
            parallel_cont = false;
        }
        private void But_Trim_Click(object sender, EventArgs e)
        {
            mode = MouseMode.TRIM;
        }

        private void radiobut_wall_CheckedChanged(object sender, EventArgs e)
        {
            if (radiobut_wall.Checked)
            {
                drawingtype_object = Drawing_Type_Object.WALL;
            }
        }

        private void radiobut_obstacle_CheckedChanged(object sender, EventArgs e)
        {
            if (radiobut_obstacle.Checked)
            {
                drawingtype_object = Drawing_Type_Object.OBSTACLE;
            }
        }

        private void radiobut_path_CheckedChanged(object sender, EventArgs e)
        {
            if (radiobut_path.Checked)
            {
                drawingtype_object = Drawing_Type_Object.PATH;
            }
        }

        private void radiobut_equipment_CheckedChanged(object sender, EventArgs e)
        {
            if (radiobut_equipment.Checked)
            {
                drawingtype_object = Drawing_Type_Object.EQUIPMENT;
                
            }
        }


        private void DisplayLot_MouseDown(object sender, MouseEventArgs e)
        {
            LButton_Pushed = true;
        }

        private void DisplayLot_MouseUp(object sender, MouseEventArgs e)
        {
            LButton_Pushed = false;
        }

        private void but_Undo2_Click(object sender, EventArgs e)
        {
            module.Undo();
            UpdateImage();
            ReleaseChart();
        }

        private void textbox_gridsize_KeyPress(object sender, KeyPressEventArgs e)
        {
            int keyCode = (int)e.KeyChar;  // 46: Point  
            if ((keyCode < 48 || keyCode > 57) && keyCode != 8)  
            {  
                e.Handled = true;  
            }
            if (keyCode == 46)
            {
                if (string.IsNullOrEmpty(textbox_gridsize.Text))
                {
                    e.Handled = true;
                }
            }
        }

        private void textbox_gridsize_TextChanged(object sender, EventArgs e)
        {            
        }

        #region 키입력 제한

        private void ScaleBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            int keyCode = (int)e.KeyChar;  // 46: Point  
            if ((keyCode < 48 || keyCode > 57) && keyCode != 8 && keyCode != 46)  
            {  
                e.Handled = true;  
            }  
            if (keyCode == 46)  
            {  
                if (string.IsNullOrEmpty(ScaleBox.Text) || ScaleBox.Text.Contains('.') == true)  
                {  
                    e.Handled = true;  
                }  
            }  
        }

        private void textBox_DefaultLength_KeyPress(object sender, KeyPressEventArgs e)
        {
            int keyCode = (int)e.KeyChar;  // 46: Point  
            if ((keyCode < 48 || keyCode > 57) && keyCode != 8 && keyCode != 46)
            {
                e.Handled = true;
            }
            if (keyCode == 46)
            {
                if (string.IsNullOrEmpty(ScaleBox.Text) || textBox_DefaultLength.Text.Contains('.') == true)
                {
                    e.Handled = true;
                }
            }  
        }

        private void textBox_DefaultStreamLength_KeyPress(object sender, KeyPressEventArgs e)
        {
            int keyCode = (int)e.KeyChar;  // 46: Point  
            if ((keyCode < 48 || keyCode > 57) && keyCode != 8 && keyCode != 46)
            {
                e.Handled = true;
            }
            if (keyCode == 46)
            {
                if (string.IsNullOrEmpty(ScaleBox.Text) || textBox_DefaultStreamLength.Text.Contains('.') == true)
                {
                    e.Handled = true;
                }
            }  
        }
        private void textBox_goal_achivement_KeyPress(object sender, KeyPressEventArgs e)
        {
            int keyCode = (int)e.KeyChar;  // 46: Point  
            if ((keyCode < 48 || keyCode > 57) && keyCode != 8)
            {
                e.Handled = true;
            }
            if (keyCode == 46)
            {
                if (string.IsNullOrEmpty(textBox_goal_achivement.Text))
                {
                    e.Handled = true;
                }
            }
        }



        private void textBox_Path_Preference_KeyPress(object sender, KeyPressEventArgs e)
        {
            int keyCode = (int)e.KeyChar;  // 46: Point  
            if ((keyCode < 48 || keyCode > 57) && keyCode != 8)
            {
                e.Handled = true;
            }
            if (keyCode == 46)
            {
                if (string.IsNullOrEmpty(textBox_Path_Preference.Text))
                {
                    e.Handled = true;
                }
            }
        }

        private void textBox_Door_Preference_KeyPress(object sender, KeyPressEventArgs e)
        {
            int keyCode = (int)e.KeyChar;  // 46: Point  
            if ((keyCode < 48 || keyCode > 57) && keyCode != 8)
            {
                e.Handled = true;
            }
            if (keyCode == 46)
            {
                if (string.IsNullOrEmpty(textBox_Door_Preference.Text))
                {
                    e.Handled = true;
                }
            }
        }

        private void textBox_Path_Damping_KeyPress(object sender, KeyPressEventArgs e)
        {
            int keyCode = (int)e.KeyChar;  // 46: Point  
            if ((keyCode < 48 || keyCode > 57) && keyCode != 8 && keyCode != 46)
            {
                e.Handled = true;
            }
            if (keyCode == 46)
            {
                if (string.IsNullOrEmpty(ScaleBox.Text) || textBox_Path_Damping.Text.Contains('.') == true)
                {
                    e.Handled = true;
                }
            }
        }

        private void textBox_Door_Damping_KeyPress(object sender, KeyPressEventArgs e)
        {
            int keyCode = (int)e.KeyChar;  // 46: Point  
            if ((keyCode < 48 || keyCode > 57) && keyCode != 8 && keyCode != 46)
            {
                e.Handled = true;
            }
            if (keyCode == 46)
            {
                if (string.IsNullOrEmpty(ScaleBox.Text) || textBox_Door_Damping.Text.Contains('.') == true)
                {
                    e.Handled = true;
                }
            }
        }

        private void textBox_irregular_hyd_length_KeyPress(object sender, KeyPressEventArgs e)
        {
            int keyCode = (int)e.KeyChar;  // 46: Point  
            if ((keyCode < 48 || keyCode > 57) && keyCode != 8 && keyCode != 46)
            {
                e.Handled = true;
            }
            if (keyCode == 46)
            {
                if (string.IsNullOrEmpty(ScaleBox.Text) || textBox_irregular_hyd_length.Text.Contains('.') == true)
                {
                    e.Handled = true;
                }
            }
        }


#endregion
        // 여기서부터 최적화


        private void Init_Optimization_Click(object sender, EventArgs e)
        {
            module.Reset_Irregular_Set();
            for (int i = listBox_irregular_hyd_list.Items.Count - 1; i >= 0; i--) // 총 몇개인가?
            {
                double value = Convert.ToDouble(listBox_irregular_hyd_list.Items[i]);
                module.Add_Irregular_Hyd(value);
            }
            comboBox_Detection_Check.SelectedIndex = 0;

            module.Opt_Init_Option1_deterministic(Convert.ToDouble(textBox_DefaultLength.Text), Convert.ToDouble(textBox_DefaultStreamLength.Text), radioBut_Option1.Checked , Convert.ToDouble(textBox_goal_achivement.Text) / 100.0, checkBox_UseFixedPoint.Checked);
            UpdateImage();
            ReleaseChart();
        }


        // bool starttimer = false;
        private void Do_Option1_by_step_Click(object sender, EventArgs e)
        {

            module.Reset_Irregular_Set();
            for (int i = listBox_irregular_hyd_list.Items.Count - 1; i >= 0; i--) // 총 몇개인가?
            {
                double value = Convert.ToDouble(listBox_irregular_hyd_list.Items[i]);
                module.Add_Irregular_Hyd(value);
            }
            comboBox_Detection_Check.SelectedIndex = 0;

            module.Opt_Init_Option1_deterministic(Convert.ToDouble(textBox_DefaultLength.Text), Convert.ToDouble(textBox_DefaultStreamLength.Text), radioBut_Option1.Checked, Convert.ToDouble(textBox_goal_achivement.Text) / 100.0, checkBox_UseFixedPoint.Checked);
            UpdateImage();
            ReleaseChart();
            Invalidate();
            Update();
            //Console.Read();
            //Console.Read();
            while (true)
            {
                while (true)
                {
                    if (module.Do_deterministic_1_step(radioBut_Option1.Checked) == -1)
                        break;
                    UpdateImage();
                    Invalidate();
                    Update();
                }
                //Console.Read();
                //Console.Read();
                if (module.Do_deterministic_move_step(radioBut_Option1.Checked) == -1)
                    break;
                ReleaseChart();
                UpdateImage();
                Invalidate();
                Update();
            }
            ReleaseChart();
            UpdateImage();
            Invalidate();
            Update();
            //Console.Read();
            //Console.Read();
            
            if(radioBut_Option2.Checked == false)
            {

                int iteration = 0; // full iteration
                //     int NEW_GRIDSIZE = GRIDSIZE - 2;
                //    if (NEW_GRIDSIZE < 3)
                //         NEW_GRIDSIZE = 3;
                //     textbox_gridsize.Text = NEW_GRIDSIZE.ToString();
                //      module.Set_Gridsize(NEW_GRIDSIZE);
                module.Set_Iteration(iteration); // full_iteartion
                //      if (NEW_GRIDSIZE != GRIDSIZE)
                //      {
                //          module.Init(2); // optimization_init
                //         module.give_score();
                //     }
                //      GRIDSIZE = NEW_GRIDSIZE;

                //       ReleaseChart();
                //       UpdateImage();
                //      Invalidate();
                //      Update();

                while (true)
                {
                    while (module.Do_deterministic_1_step(true) != -1)
                    {
                        //module.Do_deterministic_move_step(true);
                        UpdateImage();
                        Invalidate();
                        Update();
                    }

                    // break;

                    if (module.Do_deterministic_move_step(true) == -1)
                        break;
                    ReleaseChart();
                    UpdateImage();
                    Invalidate();
                    Update();
                }
                ReleaseChart();
                UpdateImage();
                Invalidate();
                Update();

            }
            

            /*



            NEW_GRIDSIZE = 5;
            if (NEW_GRIDSIZE < 3)
                NEW_GRIDSIZE = 3;
            textbox_gridsize.Text = NEW_GRIDSIZE.ToString();
            module.Set_Gridsize(NEW_GRIDSIZE);
            module.Set_Iteration(iteration); // full_iteartion
            if (NEW_GRIDSIZE != GRIDSIZE)
            {
                module.Init(2); // optimization_init
                module.give_score();
            }
            GRIDSIZE = NEW_GRIDSIZE;

            ReleaseChart();
            UpdateImage();
            Invalidate();
            Update();

            while (true)
            {
                while (module.Do_deterministic_1_step(true) != -1)
                {
                    //module.Do_deterministic_move_step(true);
                    UpdateImage();
                    Invalidate();
                    Update();
                }

                break;
            }
            */
            while (true)
            {
                if (module.Do_deterministic_move_prefer_area(radioBut_Option1.Checked) == -1)// prefer area 찾아감
                    break;
                ReleaseChart();
                UpdateImage();
                Invalidate();
                Update();
                //Console.Read();
                //Console.Read();
            }
            ReleaseChart();
            UpdateImage();
            Invalidate();
            Update();


            if (radioBut_Option1.Checked == false)
            {
                module.Do_Plan2();
                module.Opt_Init_Option1_deterministic(Convert.ToDouble(textBox_DefaultLength.Text), Convert.ToDouble(textBox_DefaultStreamLength.Text), radioBut_Option1.Checked, Convert.ToDouble(textBox_goal_achivement.Text) / 100.0, checkBox_UseFixedPoint.Checked);
                UpdateImage();
                ReleaseChart();

                while (true)
                {
                    while (true)
                    {
                        if (module.Do_deterministic_1_step(radioBut_Option1.Checked) == -1)
                            break;
                        UpdateImage();
                        Invalidate();
                        Update();
                    }
                    if (module.Do_deterministic_move_step(radioBut_Option1.Checked) == -1)
                        break;
                    ReleaseChart();
                    UpdateImage();
                    Invalidate();
                    Update();
                }
                ReleaseChart();
                UpdateImage();
                Invalidate();
                Update();




                if (radioBut_Option2.Checked == false)
                {

                    //int iteration = 0; // full iteration
                    //module.Set_Iteration(iteration); // full_iteartion
                    while (true)
                    {
                        while (module.Do_deterministic_1_step(true) != -1)
                        {
                            //module.Do_deterministic_move_step(true);
                            UpdateImage();
                            Invalidate();
                            Update();
                        }

                        // break;

                        if (module.Do_deterministic_move_step(true) == -1)
                            break;
                        ReleaseChart();
                        UpdateImage();
                        Invalidate();
                        Update();
                    }
                    ReleaseChart();
                    UpdateImage();
                    Invalidate();
                    Update();

                }
                while (true)
                {
                    if (module.Do_deterministic_move_prefer_area(radioBut_Option1.Checked) == -1)// prefer area 찾아감
                        break;
                    ReleaseChart();
                    UpdateImage();
                    Invalidate();
                    Update();
                }
                ReleaseChart();
                UpdateImage();
                Invalidate();
                Update();
            }
        }
        private void button3_Click(object sender, EventArgs e)
        {
            while (true)
            {
                if (module.Do_deterministic_move_prefer_area(radioBut_Option1.Checked) == -1)// prefer area 찾아감
                    break;
                ReleaseChart();
                UpdateImage();
                Invalidate();
                Update();
            }
            ReleaseChart();
            UpdateImage();
            Invalidate();
            Update();
        }

        private void comboBox_Detection_Check_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdateImage();
        }

        private void textBox4_TextChanged(object sender, EventArgs e)
        {

        }

        private void but_Update_Scoring_Click(object sender, EventArgs e)
        {
            module.Set_Scoring_Parameters(Convert.ToDouble(textBox_Door_Preference.Text), Convert.ToDouble(textBox_Door_Damping.Text), Convert.ToDouble(textBox_Path_Preference.Text), Convert.ToDouble(textBox_Path_Damping.Text));
            module.Set_Infeasible_Range(Convert.ToDouble(textBox_infeasible_range.Text));
            module.Update_Feasible_Region();
            module.give_score();
  
            UpdateImage();
        }

        private void but_irregular_hyd_add_Click(object sender, EventArgs e)
        {
            listBox_irregular_hyd_list.Items.Add(textBox_irregular_hyd_length.Text);
        }

        private void but_irregular_hyd_clear_Click(object sender, EventArgs e)
        {
            listBox_irregular_hyd_list.Items.Clear();
        }

        private void radioBut_Option2_CheckedChanged(object sender, EventArgs e)
        {
            check_stream.Checked = true;
        }

        private void button_PSO_INIT_Click(object sender, EventArgs e)
        {
            module.Do_PSO_Option1_Init(Convert.ToDouble(textBox_goal_achivement.Text) / 100.0);
        }

        private void button_PSO_Click(object sender, EventArgs e)
        {
            while(true)
            {
                int result = module.Do_PSO_Option1_Step();
                UpdateImage();
                Invalidate();
                Update();
                if (result == 0)
                    continue;
                else if (result == -1)
                    continue;
                else if (result == -3)
                    break;
            }
           
        }

        private void but_savedata_Click(object sender, EventArgs e)
        {
            if (!Initialized)
                return;

            SaveFileDialog savefile = new SaveFileDialog();
           
           // savefile.InitialDirectory = @"C:\"; //기본 저장 경로
            savefile.Title = "데이터 저장"; //saveFileDialog 창 이름 설정
            savefile.Filter = "텍스트 문서(*.txt)|*.txt|모든 파일|*.*"; //파일 형식 부분
            savefile.DefaultExt = "txt";
            savefile.FileName = filename_nopath + "_GRID" + GRIDSIZE.ToString();
            savefile.OverwritePrompt = true;
        //   savefile.AddExtension = true;//확장명 추가 여부
            if (savefile.ShowDialog() == DialogResult.OK)    // 여기서 경로를 얻어냄
            {
        //        if (System.IO.File.Exists(savefile.FileName))
        //            System.IO.File.Delete(savefile.FileName);
                byte[] path = Encoding.ASCII.GetBytes(savefile.FileName);
                unsafe
                {
                    fixed (byte* path1 = path) // 만약에 오류생길경우 fixed처리
                    {
                        sbyte* sim_path = (sbyte*)path1;
                        module.saveinfo(sim_path);
                    }
                }
                
            }
        }

        private void but_opendata_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(filename))
                return;

            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Filter = "텍스트 문서(*.txt)|*.txt|모든 파일|*.*"; //파일 형식 부분
            dlg.DefaultExt = "txt";

            if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                
                byte[] path = Encoding.ASCII.GetBytes(dlg.FileName);
                unsafe
                {
                    fixed (byte* path1 = path) // 만약에 오류생길경우 fixed처리
                    {
                        sbyte* sim_path = (sbyte*)path1;
                        module.loadinfo(sim_path);

                        int prgmode = module.get_program_mode();
                        Cbox_progmode.SelectedIndex = prgmode;
                        textbox_gridsize.Text = module.Get_gridsize().ToString();
                        
                        Initialize_Click(sender, e);
                     
                        UpdateImage();
                        ReleaseChart();
                    }
                }
            }
        }
        
    }
}
