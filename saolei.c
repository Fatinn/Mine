#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include "resource.h"

#define WIDTH 20		//宽度，雷区格数
#define HEIGHT 15		//高度，雷区格数
#define BOXSIZE_X 25		//格宽
#define BOXSIZE_Y 25		//格高
#define BORDER 9		//界面空闲区域的宽度
#define SHADOW3 4		//外框阴影
#define SHADOW2 3		//内框阴影
#define SHADOW1 2		//格阴影
#define RESERVED_AREA 32		//显示状态区域的高度
#define BOMBNUM 30		// 雷的个数
#define ID_TIMER 1	//计时器编号

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;
void DrawBorder(HDC hdc, int x, int y, int Width, int Height, int BorderWidth, int WTop);
void ExpandSearch(int Board[HEIGHT][WIDTH],int i,int j,HWND hwnd);
void RedrawClientArea(HDC hdc,int Board[HEIGHT][WIDTH],HWND hwnd);

static TCHAR	szAppName[] = TEXT ("zuoYe3") ;

int WINAPI WinMain(	HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    PSTR szCmdLine,
                    int iCmdShow	)
{
    HWND			hwnd ;
    MSG				msg ;
    WNDCLASS	wndclass ;
    HMENU			hMenu;

    int iWinBorderX;
    int iWinBorderY;
    int iCaptionY;
    int iMenuY;
    int cx;		//窗口宽度
    int cy;		//窗口高度

    wndclass.style					= CS_HREDRAW | CS_VREDRAW ;
    wndclass.lpfnWndProc		= WndProc ;
    wndclass.cbClsExtra			= 0 ;
    wndclass.cbWndExtra			= 0 ;
    wndclass.hInstance				= hInstance ;
    wndclass.hIcon					= LoadIcon (NULL, IDI_APPLICATION) ;
    wndclass.hCursor				= LoadCursor (NULL, IDC_ARROW) ;
    wndclass.hbrBackground	= (HBRUSH) GetStockObject (WHITE_BRUSH) ;
    wndclass.lpszMenuName  = szAppName ;
    wndclass.lpszClassName	= szAppName ;

    if (!RegisterClass (&wndclass))
    {
        MessageBox (	NULL,
                        TEXT ("This program requires Windows NT!"),
                        szAppName,
                        MB_ICONERROR		) ;
        return 0 ;
    }

    iWinBorderX = GetSystemMetrics(SM_CXBORDER);		//窗口边框宽度
    iWinBorderY = GetSystemMetrics(SM_CYBORDER);		//窗口边框高度
    iCaptionY = GetSystemMetrics(SM_CYCAPTION);		//窗口标题条高度
    iMenuY = GetSystemMetrics(SM_CYMENU);		//窗口菜单条高度
    //iMenuY=0;

    cx = BOXSIZE_X * WIDTH + 2 * BORDER +  2 * SHADOW3 + 2 * SHADOW2 + 2 * iWinBorderX;
    cy = BOXSIZE_Y * HEIGHT + 3 * BORDER + 2 * SHADOW3+ 4 * SHADOW2 + RESERVED_AREA+2 * iWinBorderY + iCaptionY + iMenuY;

    hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU1));
    hwnd = CreateWindow(	szAppName,                  // window class name
                            TEXT ("Mine_21091101"), // window caption
                            WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,//这几个是什么风格？
                            // | WS_THICKFRAME,        // window style
                            CW_USEDEFAULT,              // initial x position
                            CW_USEDEFAULT,              // initial y position
                            cx,cy,
                            //CW_USEDEFAULT,              // initial x size
                            //CW_USEDEFAULT,              // initial y size
                            NULL,                       // parent window handle
                            hMenu,                       // window menu handle
                            hInstance,                  // program instance handle
                            NULL) ;                     // creation parameters

    ShowWindow (hwnd, iCmdShow) ;
    UpdateWindow (hwnd) ;

    while (GetMessage (&msg, NULL, 0, 0))
    {
        TranslateMessage (&msg) ;
        DispatchMessage (&msg) ;
    }
    return msg.wParam ;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //static HWND
    HDC         hdc ;
    PAINTSTRUCT ps ;
    static RECT Grid;
    RECT        rect;
    static RECT	 rect1;//计时框
    static RECT	 rect2;//标记计数框
    static BOOL  MouseDown=FALSE;
    static BOOL outFlag = TRUE;//鼠标指针出范围的标记
    HBRUSH      hBrush;
    int x,y,xStart,yStart,k;
    static int i,j;
    static int mi,mj;		//用于鼠标移动时控制
    POINT pt;
    int bi,bj,bombNum;
    TCHAR buffer[2];
    static int atime; //时间
    static int bombs = BOMBNUM;
    static int bombSign;//标记数目
    int FindNum;//已探测的数目
    //8邻域
    static int dir[8][2] = {{ -1, -1 },{ -1, 0 },{ -1, 1 },{ 0, -1 },{ 0, 1 },{ 1, -1 },{ 1, 0 },{ 1, 1 }};
    static int Board[HEIGHT][WIDTH]; //存放雷信息的二维数组
    /*
    说明:
    1.Board初值为0。
    2.Board值为-1，表示是雷。
    3.Board值为1~9，表示周围雷的个数，其中9表示周围没有雷。另外，1~9也表示已被探测过。
    4.Board值为-1-2=-3，表示已经标注且有雷，0-2=-2，表示已经标注但无雷。
    */

    //int iWinBorderX;
    //int iWinBorderY;
    //int iCaptionY;
    //int iMenuY;
    //int cx;
    //int cy;

    switch (message)
    {
    case WM_CREATE:
        //bombs = BOMBNUM;
        //FindNum = 0;
        atime = 0;
        bombSign = bombs;
        //初始化雷的位置
        for(j=0; j<HEIGHT; j++)
            for(i=0; i<WIDTH; i++)
                Board[j][i] = 0;
        srand((unsigned int)time(NULL));//设置随机数种子
        //随机生成雷的位置并设置为-1，雷总数是BOMBNUM
        for(i=0; i<bombs; i++)
        {
            j = rand()%HEIGHT;
            k = rand()%WIDTH;
            if(Board[j][k] == -1) i--;
            else Board[j][k] = -1;
        }
        return 0;

    case WM_MOUSEMOVE:		//鼠标移动
        //如果此时处于鼠标左键按下状态
        if (MouseDown)
        {
            //取坐标
            pt.x = LOWORD (lParam);
            pt.y = HIWORD (lParam);
            //整个雷区范围
            rect.left = SHADOW3+BORDER+SHADOW2;
            rect.top = RESERVED_AREA+SHADOW3+SHADOW2*3+2*BORDER;
            rect.right = rect.left+BOXSIZE_X * WIDTH ;
            rect.bottom = rect.top+BOXSIZE_Y * HEIGHT ;
            //坐标在雷区范围中
            hdc=GetDC(hwnd);
            if (PtInRect(&rect,pt))
            {
                mi = i;
                mj = j;//记录鼠标左键按下时 或 上次移动时的位置
                //某个雷区范围
                i=(pt.x-rect.left)/BOXSIZE_X;
                j=(pt.y-rect.top)/BOXSIZE_Y;
                rect.left+=i*BOXSIZE_X;
                rect.top+=j*BOXSIZE_Y;
                rect.right  = rect.left+BOXSIZE_X;
                rect.bottom =rect.top+BOXSIZE_Y;
                //坐标没有离开原来的那个雷区(还在原来雷区的范围内)
                if (Grid.left==rect.left && Grid.top==rect.top && Grid.right==rect.right && Grid.bottom==rect.bottom)
                    return 0;
                //坐标移到了别的雷区后
                //设置复原效果
                if(!outFlag)
                {
                    if(Board[mj][mi] <= 0)
                        DrawBorder(hdc, Grid.left, Grid.top, BOXSIZE_X-1, BOXSIZE_Y-1, SHADOW1, TRUE);
                }
                outFlag = FALSE;
                //设置按下效果
                Grid.left=rect.left;
                Grid.top=rect.top;
                Grid.right=rect.right;
                Grid.bottom=rect.bottom;
                if(Board[j][i] >= -1 )
                {
                    DrawBorder(hdc, Grid.left, Grid.top, BOXSIZE_X-1, BOXSIZE_Y-1, SHADOW1, FALSE);
                }
                mi = i;
                mj = j;
            }
            else
            {
                if(outFlag) return 0;//鼠标指针在雷区范围外，返回
                if(Board[mj][mi] <= 0)
                {
                    DrawBorder(hdc, Grid.left, Grid.top, BOXSIZE_X-1, BOXSIZE_Y-1, SHADOW1, TRUE);
                    //因为此时鼠标指针已在范围之外，故Grid中的值已无效
                    Grid.left = -1;
                    outFlag = TRUE;
                }

            }
            ReleaseDC(hwnd,hdc);
        }
        return 0;

    case WM_LBUTTONUP:		//鼠标左键抬起
        //如果之前鼠标左键处于有效按下状态
        if (MouseDown)
        {
            //释放拦截
            ReleaseCapture () ;

            pt.x = LOWORD (lParam);
            pt.y = HIWORD (lParam);
            rect.left = SHADOW3+BORDER+SHADOW2;
            rect.top = RESERVED_AREA+SHADOW3+SHADOW2*3+2*BORDER;
            i=(pt.x-rect.left)/BOXSIZE_X;
            j=(pt.y-rect.top)/BOXSIZE_Y;

            //如果指针在区域外
            if(outFlag)
            {
                MouseDown = FALSE;
                return 0;
            }
            //如果在标志处
            if(Board[j][i] < -1)
            {
                MouseDown = FALSE;
                return 0;
            }
            //如果是已探测区域
            if(Board[j][i] > 0)
            {
                MouseDown = FALSE;
                return 0;
            }

            //设置计时器
            SetTimer(hwnd,ID_TIMER,1000,NULL);

            hdc=GetDC(hwnd);
            //设置text的输出范围
            Grid.left+=2;
            Grid.top+=2;
            Grid.right= Grid.left+BOXSIZE_X-2;
            Grid.bottom =Grid.top+BOXSIZE_Y-2;
            //设置相关颜色
            //SetTextColor(hdc,RGB(0, 0, 255));
            SetBkColor(hdc,RGB(230, 230, 230));//设置Text背后颜色与背景颜色一致

            if(Board[j][i] == -1)
            {
                /* 表示失败的代码放这里 */
                //输出所有的雷
                for(j=0; j<HEIGHT; j++)
                    for(i=0; i<WIDTH; i++)
                    {
                        if(Board[j][i] == -1)
                        {
                            //设置text的输出范围
                            Grid.left = SHADOW3+BORDER+SHADOW2;
                            Grid.top = RESERVED_AREA+SHADOW3+SHADOW2*3+2*BORDER;
                            Grid.right = Grid.left+BOXSIZE_X * WIDTH ;
                            Grid.bottom = Grid.top+BOXSIZE_Y * HEIGHT ;
                            Grid.left+=i*BOXSIZE_X;
                            Grid.top+=j*BOXSIZE_Y;
                            Grid.right  = Grid.left+BOXSIZE_X;
                            Grid.bottom =Grid.top+BOXSIZE_Y;
                            Grid.left+=2;
                            Grid.top+=2;
                            Grid.right= Grid.left+BOXSIZE_X-2;
                            Grid.bottom =Grid.top+BOXSIZE_Y-2;
                            //颜色
                            SetTextColor(hdc,RGB(0, 0, 0));
                            //输出
                            DrawText(hdc,"雷",-1,&Grid,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
                        }
                    }
                /* 处理程序结束的代码 */
                KillTimer(hwnd,ID_TIMER);//停止时间
                if(MessageBox(hwnd,TEXT("- -# 你失败了!要重来吗?"),szAppName,MB_OKCANCEL) == IDCANCEL)
                    exit(0);
                else
                {
                    //atime = 0;
                    //bombs = BOMBNUM;
                    SendMessage(hwnd,WM_CREATE,0,0);
                    InvalidateRect(hwnd,NULL,FALSE);
                }
            }
            else
            {
                bombNum = 0;//复位
                for (k = 0; k < 8; k++)
                {
                    bi = i + dir[k][0];
                    if(bi < 0 || bi == WIDTH) continue;
                    bj = j + dir[k][1];
                    if(bj < 0 || bj == HEIGHT) continue;
                    if(Board[bj][bi] == -1 || Board[bj][bi] == -3) bombNum++;
                }
                if(bombNum == 0) //周围无雷，拓展搜索
                {
                    ExpandSearch(Board,i,j,hwnd);
                }
                else  //记录雷数
                {
                    Board[j][i] = bombNum;

                    wsprintf(buffer,TEXT("%d"),bombNum);
                    //输出text
                    switch(bombNum)
                    {
                    case 1:
                        SetTextColor(hdc,RGB(225, 55, 98));
                        break;
                    case 2:
                        SetTextColor(hdc,RGB(0, 255, 0));
                        break;
                    case 3:
                        SetTextColor(hdc,RGB(0, 0, 255));
                        break;
                    case 4:
                        SetTextColor(hdc,RGB(255, 255, 0));
                        break;
                    case 5:
                        SetTextColor(hdc,RGB(255, 0, 255));
                        break;
                    case 6:
                        SetTextColor(hdc,RGB(0, 255, 255));
                        break;
                    case 7:
                        SetTextColor(hdc,RGB(255, 255, 255));
                        break;
                    case 8:
                        SetTextColor(hdc,RGB(150, 30, 210));
                        break;
                    }

                    DrawText(hdc,buffer,-1,&Grid,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
                }
            }
            /* 胜利 */
            FindNum = 0;//复位
            for(j=0; j<HEIGHT; j++)
                for(i=0; i<WIDTH; i++)
                {
                    if(Board[j][i] > 0) FindNum++;
                }
            if(FindNum == (WIDTH * HEIGHT - bombs))
            {
                KillTimer(hwnd,ID_TIMER);
                MessageBox(hwnd,TEXT("^ ^ 你胜利了,还算不错~"),szAppName,MB_OK);
                SendMessage(hwnd,WM_CREATE,0,0);
                InvalidateRect(hwnd,NULL,FALSE);
            }
            ReleaseDC(hwnd,hdc);
            //取消鼠标左键按下状态
            MouseDown=FALSE;
        }
        //如果左键没按下，返回
        return 0;

    case WM_LBUTTONDOWN:		//鼠标左键按下
        /*
        说明：1.鼠标左键在雷区范围内按下
        			1).按在未探测的区域
        			2).按在已探测过的区域
        			3).按在已标记的区域
        		  2.鼠标左键在程序其他位置按下
        */
        //取坐标
        pt.x = LOWORD (lParam);
        pt.y = HIWORD (lParam);
        //计算整个雷区范围(Grid)
        Grid.left = SHADOW3+BORDER+SHADOW2 ;
        Grid.top = RESERVED_AREA+SHADOW3+SHADOW2*3+2*BORDER ;
        Grid.right = Grid.left+BOXSIZE_X * WIDTH ;
        Grid.bottom = Grid.top+BOXSIZE_Y * HEIGHT ;

        //坐标在雷区中
        if (PtInRect(&Grid,pt))
        {
            //计算某个雷区范围
            i=(pt.x-Grid.left)/BOXSIZE_X;
            j=(pt.y-Grid.top)/BOXSIZE_Y;

            if(Board[j][i] < -1/*标记位置*/ || Board[j][i] >0/*已探测过的区域*/)
            {
                return 0;//按下无效，返回
            }

            //拦截鼠标
            SetCapture (hwnd) ;
            //按下有效
            MouseDown=TRUE;
            //计算当前雷位置范围(Grid)
            Grid.left+=i*BOXSIZE_X;
            Grid.top+=j*BOXSIZE_Y;
            Grid.right  = Grid.left+BOXSIZE_X;
            Grid.bottom =Grid.top+BOXSIZE_Y;
            //设置按下效果
            hdc=GetDC(hwnd);
            DrawBorder(hdc, Grid.left, Grid.top, BOXSIZE_X-1, BOXSIZE_Y-1, SHADOW1, FALSE);
            ReleaseDC(hwnd,hdc);
            //标记鼠标指针在整个雷区内
            outFlag = FALSE;
        }
        //坐标不在雷区中，返回
        return 0;

    case WM_RBUTTONDOWN:
        //取坐标
        pt.x = LOWORD (lParam);
        pt.y = HIWORD (lParam);
        //计算整个雷区范围
        Grid.left=SHADOW3+BORDER+SHADOW2;
        Grid.top    = RESERVED_AREA+SHADOW3+SHADOW2*3+2*BORDER;
        Grid.right  = Grid.left+BOXSIZE_X * WIDTH ;
        Grid.bottom =Grid.top+BOXSIZE_Y * HEIGHT ;
        if (PtInRect(&Grid,pt))
        {
            i=(pt.x-Grid.left)/BOXSIZE_X;
            j=(pt.y-Grid.top)/BOXSIZE_Y;

            if(Board[j][i] > 0)	return 0;
            if(Board[j][i] < -1) //复原
            {
                if(Board[j][i] == -2)
                    Board[j][i] = 0;
                if(Board[j][i] == -3)
                    Board[j][i] = -1;

                //定位范围
                Grid.left+=i*BOXSIZE_X;
                Grid.top+=j*BOXSIZE_Y;
                Grid.right  = Grid.left+BOXSIZE_X;
                Grid.bottom =Grid.top+BOXSIZE_Y;
                InvalidateRect(hwnd,&Grid,TRUE);		//重绘

                bombSign++;//标记数增加
                InvalidateRect(hwnd,&rect2,FALSE);
                return 0;
            }
            if(Board[j][i] == 0)//标注
                Board[j][i] = -2;
            if(Board[j][i] == -1)
                Board[j][i] = -3;

            //设置按下效果
            Grid.left+=i*BOXSIZE_X;
            Grid.top+=j*BOXSIZE_Y;
            Grid.right  = Grid.left+BOXSIZE_X;
            Grid.bottom =Grid.top+BOXSIZE_Y;
            hdc=GetDC(hwnd);
            //设置text的输出范围
            Grid.left+=2;
            Grid.top+=2;
            Grid.right= Grid.left+BOXSIZE_X-2;
            Grid.bottom =Grid.top+BOXSIZE_Y-2;
            //设置相关颜色
            SetTextColor(hdc,RGB(255, 0, 0));
            SetBkColor(hdc,RGB(230, 230, 230));//设置Text背后颜色与背景颜色一致

            DrawText(hdc,"危",-1,&Grid,DT_SINGLELINE|DT_CENTER|DT_VCENTER);

            ReleaseDC(hwnd,hdc);

            bombSign--;//标记数减少
            InvalidateRect(hwnd,&rect2,FALSE);
        }

        return 0;

    case WM_PAINT:
        hdc = BeginPaint (hwnd, &ps) ;

        GetClientRect (hwnd, &rect) ;
        hBrush = CreateSolidBrush (RGB(230,230,230)) ;
        SelectObject(hdc,GetStockObject(NULL_PEN));
        SelectObject(hdc,hBrush);

        Rectangle(hdc,0, 0, rect.right, rect.bottom);
        DeleteObject(SelectObject(hdc,GetStockObject(WHITE_BRUSH)));//这怎么理解?


        DrawBorder(hdc, 0, 0, rect.right, rect.bottom, SHADOW3, TRUE);
        DrawBorder(hdc, SHADOW3+BORDER, SHADOW3+BORDER, BOXSIZE_X * WIDTH + 2 * SHADOW2, RESERVED_AREA+ 2 * SHADOW2, SHADOW2, FALSE);

        DrawBorder(hdc, SHADOW3+BORDER, RESERVED_AREA+SHADOW3+SHADOW2*2+2*BORDER, BOXSIZE_X * WIDTH + 2 * SHADOW2, BOXSIZE_Y * HEIGHT+ 2 * SHADOW2, SHADOW2, FALSE);
        xStart=SHADOW3+BORDER+SHADOW2;
        yStart= RESERVED_AREA+SHADOW3+SHADOW2*3+2*BORDER;
        for (i=0; i<WIDTH; i++)
            for (j=0; j<HEIGHT; j++)
            {
                x=xStart+i*BOXSIZE_X;
                y=yStart+j*BOXSIZE_Y;
                DrawBorder(hdc, x, y, BOXSIZE_X-1, BOXSIZE_Y-1, SHADOW1, TRUE);

            }
        //左边窗口
        GetClientRect (hwnd, &rect) ;
        rect.top = rect.top + 20;
        rect.bottom = rect.top + 25;
        rect.left = rect.left + 25;
        rect.right = rect.left + 55;
        SelectObject(hdc,GetStockObject(NULL_PEN));
        hBrush = CreateSolidBrush (RGB(0,255,0)) ;
        SelectObject(hdc,hBrush);
        Rectangle(hdc,rect.left, rect.top, rect.right, rect.bottom);
        DrawBorder(hdc,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SHADOW2,TRUE);
        //输出标记记数
        rect2.top = rect.top + 2;
        rect2.bottom = rect.bottom - 2;
        rect2.left = rect.left + 2;
        rect2.right = rect.right - 2;
        SetTextColor(hdc,RGB(0,0,0));
        SetBkColor(hdc,RGB(0,255,0));
        wsprintf(buffer,TEXT("%d"),bombSign);
        DrawText(hdc,buffer,-1,&rect,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
        //右边窗口
        GetClientRect (hwnd, &rect) ;
        rect.top = rect.top + 20;
        rect.bottom = rect.top + 25;
        rect.right = rect.right - 25;
        rect.left = rect.right - 55;
        Rectangle(hdc,rect.left, rect.top, rect.right, rect.bottom);
        DrawBorder(hdc,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SHADOW2,TRUE);
        DeleteObject(SelectObject(hdc,GetStockObject(WHITE_BRUSH)));
        //输出计时
        rect1.top = rect.top + 2;
        rect1.bottom = rect.bottom - 2;
        rect1.left = rect.left + 2;
        rect1.right = rect.right - 2;
        SetTextColor(hdc,RGB(0,0,0));
        SetBkColor(hdc,RGB(0,255,0));
        wsprintf(buffer,TEXT("%d"),atime);
        DrawText(hdc,buffer,-1,&rect,DT_SINGLELINE|DT_CENTER|DT_VCENTER);

        RedrawClientArea(hdc,Board,hwnd);
        EndPaint (hwnd, &ps) ;
        //RedrawClientArea(Board,hwnd);
        return 0 ;

    case WM_TIMER:
        atime++;//增加时间
        InvalidateRect(hwnd,&rect1,FALSE);
        return 0;

    case WM_COMMAND:
        switch(LOWORD (wParam))
        {
            //case ID_ZIDINGYI:
            //return 0;

        case ID_ADD5:
            KillTimer(hwnd,ID_TIMER);//停止时间
            //atime = 0;
            bombs = bombs + 5;
            bombs = min(bombs, WIDTH * HEIGHT);
            SendMessage(hwnd,WM_CREATE,0,0);
            InvalidateRect(hwnd,NULL,FALSE);
            return 0;

        case ID_MINUS5:
            KillTimer(hwnd,ID_TIMER);//停止时间
            //atime = 0;
            bombs = bombs - 5;
            bombs = max(bombs, 5);
            SendMessage(hwnd,WM_CREATE,0,0);
            InvalidateRect(hwnd,NULL,FALSE);
            return 0;

        case ID_TUICHU:
            exit(0);
        }

    case WM_DESTROY:
        KillTimer(hwnd,ID_TIMER);
        PostQuitMessage (0) ;
        return 0 ;
    }
    return DefWindowProc (hwnd, message, wParam, lParam) ;
}

void DrawBorder(HDC hdc, int x, int y, int Width, int Height, int BorderWidth, int WTop)
{
    int i;
    HPEN  hpen;
    hpen=CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    if (WTop)
        SelectObject(hdc,GetStockObject(WHITE_PEN));
    else
        SelectObject(hdc,hpen);
    for (i=0; i<BorderWidth; i++)//先画上面和左面
    {
        MoveToEx(hdc,x+i, y+i,NULL);
        LineTo(hdc,x+Width-i, y+i);
        MoveToEx(hdc,x+i, y+i,NULL);
        LineTo(hdc,x+i, y+Height-i);
    }

    if (WTop)
        SelectObject(hdc,hpen);
    else
        SelectObject(hdc,GetStockObject(WHITE_PEN));
    for (i=0; i<BorderWidth; i++)//再画右面和下面
    {
        MoveToEx(hdc,x+Width-i, y+Height-i,NULL);
        LineTo(hdc,x+Width-i, y+i);
        MoveToEx(hdc,x+Width-i, y+Height-i,NULL);
        LineTo(hdc,x+i, y+Height-i);
    }
    DeleteObject(hpen);
}

void ExpandSearch(int Board[HEIGHT][WIDTH],int i,int j,HWND hwnd)
{
    int d[8][2] = {{ -1, -1 },{ -1, 0 },{ -1, 1 },{ 0, -1 },{ 0, 1 },{ 1, -1 },{ 1, 0 },{ 1, 1 }};
    int ii,jj;
    int bi,bj;
    int bombNum;
    int m,k;
    TCHAR mybuffer[2];
    RECT rect;
    HDC hdc;

    Board[j][i] = 9;

//整个雷区范围
    rect.left=SHADOW3+BORDER+SHADOW2;
    rect.top    = RESERVED_AREA+SHADOW3+SHADOW2*3+2*BORDER;
    rect.right  = rect.left+BOXSIZE_X * WIDTH ;
    rect.bottom =rect.top+BOXSIZE_Y * HEIGHT ;
//某个雷区范围
    rect.left+=i*BOXSIZE_X;
    rect.top+=j*BOXSIZE_Y;
    rect.right  = rect.left+BOXSIZE_X;
    rect.bottom =rect.top+BOXSIZE_Y;
    hdc = GetDC(hwnd);
    DrawBorder(hdc, rect.left, rect.top, BOXSIZE_X-1, BOXSIZE_Y-1, SHADOW1, FALSE);
    ReleaseDC(hwnd,hdc);
    /* 需要将它本身输出(输出0吗？) */

    for(m=0; m<8; m++)
    {
        ii = i+d[m][0];
        if(ii < 0 || ii == WIDTH) continue;
        jj = j+d[m][1];
        if(jj < 0 || jj == HEIGHT)	continue;
        if(Board[jj][ii] == 9 || Board[jj][ii] < -1)	continue;
        bombNum = 0;//复位
        for (k = 0; k < 8; k++)
        {
            bi = ii + d[k][0];
            if(bi < 0 || bi == WIDTH) continue;
            bj = jj + d[k][1];
            if(bj < 0 || bj == HEIGHT) continue;
            //if(Board[bj][bi] == -2)	continue;
            if(Board[bj][bi] == -1 || Board[bj][bi] == -3) bombNum++;
        }
        if(bombNum != 0)
        {
            //wsprintf(mybuffer,TEXT("%d"),bombNum);
            /* 需要找到位置，绘出按下效果，输出雷数 */
            //整个雷区范围
            rect.left=SHADOW3+BORDER+SHADOW2;
            rect.top    = RESERVED_AREA+SHADOW3+SHADOW2*3+2*BORDER;
            rect.right  = rect.left+BOXSIZE_X * WIDTH ;
            rect.bottom =rect.top+BOXSIZE_Y * HEIGHT ;
            //某个雷区范围
            rect.left+=ii*BOXSIZE_X;
            rect.top+=jj*BOXSIZE_Y;
            rect.right  = rect.left+BOXSIZE_X;
            rect.bottom =rect.top+BOXSIZE_Y;

            hdc = GetDC(hwnd);
            DrawBorder(hdc, rect.left, rect.top, BOXSIZE_X-1, BOXSIZE_Y-1, SHADOW1, FALSE);

            Board[jj][ii] = bombNum;


            //设置text的输出范围
            rect.left+=2;
            rect.top+=2;
            rect.right= rect.left+BOXSIZE_X-2;
            rect.bottom =rect.top+BOXSIZE_Y-2;
            wsprintf(mybuffer,TEXT("%d"),bombNum);
            //设置相关颜色
            switch(bombNum)
            {
            case 1:
                SetTextColor(hdc,RGB(225, 55, 98));
                break;
            case 2:
                SetTextColor(hdc,RGB(0, 255, 0));
                break;
            case 3:
                SetTextColor(hdc,RGB(0, 0, 255));
                break;
            case 4:
                SetTextColor(hdc,RGB(255, 255, 0));
                break;
            case 5:
                SetTextColor(hdc,RGB(255, 0, 255));
                break;
            case 6:
                SetTextColor(hdc,RGB(0, 255, 255));
                break;
            case 7:
                SetTextColor(hdc,RGB(255, 255, 255));
                break;
            case 8:
                SetTextColor(hdc,RGB(150, 30, 210));
                break;
            }
            SetBkColor(hdc,RGB(230, 230, 230));//设置Text背后颜色与背景颜色一致
            //输出text
            DrawText(hdc,mybuffer,-1,&rect,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
            ReleaseDC(hwnd,hdc);

        }
        else
        {
            ExpandSearch(Board,ii,jj,hwnd);
        }
    }
}

void RedrawClientArea(HDC hdc,int Board[HEIGHT][WIDTH],HWND hwnd)
{
    int i,j;
    RECT rect;
    RECT rect0;
    //HDC hdc;
    TCHAR mybuffer[2];

    rect.left = SHADOW3+BORDER+SHADOW2;
    rect.top = RESERVED_AREA+SHADOW3+SHADOW2*3+2*BORDER;
    rect.right = rect.left+BOXSIZE_X * WIDTH ;
    rect.bottom = rect.top+BOXSIZE_Y * HEIGHT ;

    //hdc=GetDC(hwnd);
    for(j=0; j<HEIGHT; j++)
        for(i=0; i<WIDTH; i++)
        {
            if(Board[j][i] == -1 || Board[j][i] == 0)	continue;
            rect0.left = rect.left + i*BOXSIZE_X;
            rect0.top = rect.top + j*BOXSIZE_Y;
            rect0.right  = rect0.left+BOXSIZE_X;
            rect0.bottom =rect0.top+BOXSIZE_Y;
            if(Board[j][i] < -1)
            {
                //Text范围
                rect0.left+=2;
                rect0.top+=2;
                rect0.right= rect0.left+BOXSIZE_X-2;
                rect0.bottom =rect0.top+BOXSIZE_Y-2;

                //设置相关颜色
                SetTextColor(hdc,RGB(255, 0, 0));
                SetBkColor(hdc,RGB(230, 230, 230));//设置Text背后颜色与背景颜色一致
                DrawText(hdc,"危",-1,&rect0,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
                //ReleaseDC(hwnd,hdc);
            }
            else
            {
                //hdc=GetDC(hwnd);
                //按下效果
                DrawBorder(hdc, rect0.left, rect0.top, BOXSIZE_X-1, BOXSIZE_Y-1, SHADOW1, FALSE);
                if(Board[j][i] == 9)	continue;
                //Text范围
                rect0.left+=2;
                rect0.top+=2;
                rect0.right= rect0.left+BOXSIZE_X-2;
                rect0.bottom =rect0.top+BOXSIZE_Y-2;
                wsprintf(mybuffer,TEXT("%d"),Board[j][i]);
                //输出text
                switch(Board[j][i])
                {
                case 1:
                    SetTextColor(hdc,RGB(225, 55, 98));
                    break;
                case 2:
                    SetTextColor(hdc,RGB(0, 255, 0));
                    break;
                case 3:
                    SetTextColor(hdc,RGB(0, 0, 255));
                    break;
                case 4:
                    SetTextColor(hdc,RGB(255, 255, 0));
                    break;
                case 5:
                    SetTextColor(hdc,RGB(255, 0, 255));
                    break;
                case 6:
                    SetTextColor(hdc,RGB(0, 255, 255));
                    break;
                case 7:
                    SetTextColor(hdc,RGB(255, 255, 255));
                    break;
                case 8:
                    SetTextColor(hdc,RGB(150, 30, 210));
                    break;
                }
                SetBkColor(hdc,RGB(230, 230, 230));//设置Text背后颜色与背景颜色一致
                DrawText(hdc,mybuffer,-1,&rect0,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
            }
        }
    //ReleaseDC(hwnd,hdc);
}
