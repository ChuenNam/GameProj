#include "framework.h"
#include "GameProj.h"
#include <time.h>

#define MAX_LOADSTRING 100
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 500
#define TIMER_ID 1
#define PLAYER_SIZE 40
#define OBSTACLE_WIDTH 30
#define MAX_OBSTACLES 1
#define PLAYER_JUMP_HEIGHT 50
#define PLAYER_SPEED 8
#define OBSTACLE_SPEED 5


// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HBITMAP BirdPicture;
HBITMAP BackgroundPicture;

// 全局游戏变量
HWND hwnd;
HBRUSH playerBrush, obstacleBrush, backgroundBrush;
int playerX, playerY;
int obstacles[MAX_OBSTACLES];
int score = 0;
int fallTime = 0;
BOOL gameRunning = FALSE;
BOOL playerJumping = FALSE;
int jumpCounter = 0;
int OBSTACLE_HEIGHT = 100;
int upper = 300;
int lower = 80;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// 游戏函数声明
void InitializeGame();
void UpdateGame();
void DrawGame(HDC hdc);
void EndGame();
void ResetGame();
void GenerateObstacle();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GAMEPROJ, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAMEPROJ));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAMEPROJ));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GAMEPROJ);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    hwnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, WINDOW_WIDTH, WINDOW_HEIGHT + 60, nullptr, nullptr, hInstance, nullptr);

    if (!hwnd)
    {
        return FALSE;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 初始化游戏
    BirdPicture = (HBITMAP)LoadImage(NULL, L"IMG/Bird.bmp", IMAGE_BITMAP,
        0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
    BackgroundPicture = (HBITMAP)LoadImage(NULL, L"IMG/Background.bmp", IMAGE_BITMAP,
        0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
    InitializeGame();

    return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 分析菜单选择:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_KEYDOWN:
        if (wParam == VK_SPACE && gameRunning && !playerJumping) {
            playerJumping = TRUE;
            jumpCounter = PLAYER_JUMP_HEIGHT / PLAYER_SPEED;
            fallTime = 0;
        }
        break;
    case WM_ERASEBKGND:
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        HDC memHDC = CreateCompatibleDC(hdc);
        RECT rectClient;
        GetClientRect(hwnd, &rectClient);
        HBITMAP bmpBuff = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
        HBITMAP pOldBMP = (HBITMAP)SelectObject(memHDC, bmpBuff);
        DrawGame(memHDC);
		BOOL tt = BitBlt(hdc, rectClient.left, rectClient.top, WINDOW_WIDTH, WINDOW_HEIGHT, memHDC, rectClient.left, rectClient.top, SRCCOPY);
        SelectObject(memHDC, pOldBMP);
        DeleteObject(bmpBuff);
        DeleteDC(memHDC);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_TIMER:
        if (wParam == TIMER_ID) {
            UpdateGame();
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void DrewImage(HDC hdc, int x, int y, int sideLength, HBITMAP image)
{
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP bmp = image;
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, bmp);
    BITMAP bm;
    GetObject(bmp, sizeof(bm), &bm);
    SetStretchBltMode(hdc, STRETCH_HALFTONE);
    TransparentBlt(hdc, x, y, sideLength, sideLength, hdcMem, 0, 0, sideLength, sideLength, RGB(0, 0, 0));
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hdcMem);
}



// 初始化游戏变量和资源
void InitializeGame() {
    srand((unsigned int)time(NULL));
    playerX = WINDOW_WIDTH / 4;
    playerY = WINDOW_HEIGHT/2 - PLAYER_SIZE;

    obstacleBrush = CreateSolidBrush(RGB(0, 0, 255));
    playerBrush = CreateSolidBrush(RGB(255, 0, 0));
    backgroundBrush = CreateSolidBrush(RGB(255, 255, 255));

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i] = -1;
    }

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        GenerateObstacle();
    }

    if (!SetTimer(hwnd, TIMER_ID, 30, NULL)) {
        MessageBox(hwnd, L"Failed to create timer", L"Error", MB_OK | MB_ICONERROR);
    }

    gameRunning = TRUE;
}

// 更新游戏状态
void UpdateGame() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i] != -1) {
            obstacles[i] -= OBSTACLE_SPEED;
            if (obstacles[i] < -OBSTACLE_WIDTH) {
                int tmp = OBSTACLE_HEIGHT;
                do
                {
                    OBSTACLE_HEIGHT = lower + rand() % (upper - lower + 1);
                } while (abs(tmp - OBSTACLE_HEIGHT) < 30);

                obstacles[i] = WINDOW_WIDTH + rand() % (WINDOW_WIDTH / 2);
            }

            if (playerX < obstacles[i] + OBSTACLE_WIDTH &&
                playerX + PLAYER_SIZE > obstacles[i] &&
                playerY + PLAYER_SIZE > WINDOW_HEIGHT - OBSTACLE_HEIGHT ||
                (playerY <= 0) || (playerY + PLAYER_SIZE >= WINDOW_HEIGHT)
                ) {
                EndGame();
                return;
            }

        }
    }

    if (playerJumping) {
		playerY -= (PLAYER_SPEED - (PLAYER_SPEED * fallTime) / (PLAYER_JUMP_HEIGHT / PLAYER_SPEED));
        jumpCounter--;

        if (jumpCounter <= 0) {
            playerJumping = FALSE;
            fallTime = 0;
        }
    }
    else if (playerY < WINDOW_HEIGHT - PLAYER_SIZE) {
        playerY += 0.05 * fallTime * fallTime;
    }

    score++;
    fallTime++;

    if (rand() % 100 < 100) {
        GenerateObstacle();
    }

    InvalidateRect(hwnd, NULL, TRUE);
}

// 绘制游戏
void DrawGame(HDC hdc) {
    //RECT clientRect;
    //GetClientRect(hwnd, &clientRect);
    //FillRect(hdc, &clientRect, backgroundBrush);
    DrewImage(hdc, 0, 0, 521, BackgroundPicture);

    //RECT playerRect = { playerX, playerY, playerX + PLAYER_SIZE, playerY + PLAYER_SIZE };
    //FillRect(hdc, &playerRect, playerBrush);
    DrewImage(hdc, playerX, playerY, 64, BirdPicture);


    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i] != -1) {
            RECT obstacleRect = { obstacles[i], WINDOW_HEIGHT - OBSTACLE_HEIGHT, obstacles[i] + OBSTACLE_WIDTH, WINDOW_HEIGHT};
            FillRect(hdc, &obstacleRect, obstacleBrush);
        }
    }

    WCHAR scoreText[50];
    swprintf_s(scoreText, L"Time: %d s", score/30);
    TextOutW(hdc, 10, 10, scoreText, wcslen(scoreText));
}

// 结束游戏
void EndGame() {
    gameRunning = FALSE;
    KillTimer(hwnd, TIMER_ID);

    MessageBox(hwnd, L"   游戏结束!\n点击重新开始", L"Game Over", MB_OK | MB_ICONINFORMATION);

    ResetGame();
}

// 重置游戏状态
void ResetGame() {
    playerX = WINDOW_WIDTH / 4;
    playerY = WINDOW_HEIGHT/2 - PLAYER_SIZE;
    score = 0;
    fallTime = 0;

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i] = -1;
    }

    SetTimer(hwnd, TIMER_ID, 30, NULL);

    gameRunning = TRUE;
}

 //生成新障碍物
void GenerateObstacle() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i] == -1) {
            obstacles[i] = WINDOW_WIDTH + rand() % (WINDOW_WIDTH / 2);
            break;
        }
    }
}
