#include <windows.h>
#include <stdio.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TIMER_ID 1
#define PLAYER_SIZE 40
#define OBSTACLE_WIDTH 30
#define OBSTACLE_HEIGHT 30
#define MAX_OBSTACLES 5
#define PLAYER_JUMP_HEIGHT 100
#define PLAYER_SPEED 5
#define OBSTACLE_SPEED 5

// Global variables
HWND hwnd;
HINSTANCE hInstance;
HBRUSH playerBrush, obstacleBrush, backgroundBrush;
int playerX, playerY;
int obstacles[MAX_OBSTACLES];
int obstacleSpeed = OBSTACLE_SPEED;
int score = 0;
BOOL gameRunning = FALSE;

// Function prototypes
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void InitializeGame();
void UpdateGame();
void DrawGame(HDC hdc);
void EndGame();
void ResetGame();
void GenerateObstacle();

// Entry point of the Windows application
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    hInstance = hInst;

    // Register window class
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"JumpGame";
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;

    RegisterClass(&wc);

    // Create the window
    hwnd = CreateWindowW(L"JumpGame", L"Jump and Dodge Game",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInst, NULL);

    if (hwnd == NULL) {
        MessageBoxW(NULL, L"Window Creation Failed!", L"Error", MB_ICONERROR);
        return 0;
    }

    // Show the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// Window Procedure function
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        // Initialize resources and start the game
        InitializeGame();
        break;

    case WM_KEYDOWN:
        // Handle player controls (space bar to jump)
        if (wParam == VK_SPACE && gameRunning) {
            // Simulate a jump by moving player up
            playerY -= PLAYER_JUMP_HEIGHT;
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Draw the game
        DrawGame(hdc);

        EndPaint(hwnd, &ps);
    }
    break;

    case WM_TIMER:
        // Update game logic on timer event
        UpdateGame();
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Initialize game variables and resources
void InitializeGame() {
    srand(time(NULL)); // Initialize random seed
    playerX = WINDOW_WIDTH / 4;
    playerY = WINDOW_HEIGHT - PLAYER_SIZE - 50;

    // Create brushes for drawing
    playerBrush = CreateSolidBrush(RGB(255, 0, 0)); // Red color for player
    obstacleBrush = CreateSolidBrush(RGB(0, 0, 255)); // Blue color for obstacles
    backgroundBrush = CreateSolidBrush(RGB(255, 255, 255)); // White background

    // Initialize obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i] = -1; // -1 indicates no obstacle
    }

    // Start game timer
    SetTimer(hwnd, TIMER_ID, 30, NULL); // 30 milliseconds timer interval

    gameRunning = TRUE;
}

// Update game state
void UpdateGame() {
    // Move obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i] != -1) {
            obstacles[i] -= obstacleSpeed;
            if (obstacles[i] < -OBSTACLE_WIDTH) {
                // If obstacle is off screen, generate a new one
                obstacles[i] = WINDOW_WIDTH;
            }

            // Collision detection
            if (playerX < obstacles[i] + OBSTACLE_WIDTH &&
                playerX + PLAYER_SIZE > obstacles[i] &&
                playerY < WINDOW_HEIGHT - OBSTACLE_HEIGHT) {
                // Collision detected
                EndGame();
                return;
            }
        }
    }

    // Update player position (simulate gravity)
    if (playerY < WINDOW_HEIGHT - PLAYER_SIZE) {
        playerY += PLAYER_SPEED;
    }

    // Increase score
    score++;

    // Generate new obstacles randomly
    if (rand() % 100 < 5) {
        GenerateObstacle();
    }

    // Redraw the window
    InvalidateRect(hwnd, NULL, TRUE);
}

// Draw the game
void DrawGame(HDC hdc) {
    // Draw background
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    FillRect(hdc, &clientRect, backgroundBrush);

    // Draw player
    RECT playerRect = { playerX, playerY, playerX + PLAYER_SIZE, playerY + PLAYER_SIZE };
    FillRect(hdc, &playerRect, playerBrush);

    // Draw obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i] != -1) {
            RECT obstacleRect = { obstacles[i], WINDOW_HEIGHT - OBSTACLE_HEIGHT, obstacles[i] + OBSTACLE_WIDTH, WINDOW_HEIGHT };
            FillRect(hdc, &obstacleRect, obstacleBrush);
        }
    }

    // Display score
    WCHAR scoreText[50];
    swprintf_s(scoreText, L"Score: %d", score);
    TextOutW(hdc, 10, 10, scoreText, wcslen(scoreText));
}
// End the game
void EndGame() {
    gameRunning = FALSE;
    KillTimer(hwnd, TIMER_ID);

    // Convert string literals to wide character format
    LPCWSTR gameOverText = L"Game Over! Press OK to restart.";
    LPCWSTR gameOverCaption = L"Game Over";

    MessageBox(hwnd, gameOverText, gameOverCaption, MB_OK | MB_ICONINFORMATION);

    // Reset game
    ResetGame();
}

// Reset game state
void ResetGame() {
    playerX = WINDOW_WIDTH / 4;
    playerY = WINDOW_HEIGHT - PLAYER_SIZE - 50;
    score = 0;

    // Reset obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i] = -1;
    }

    // Restart game timer
    SetTimer(hwnd, TIMER_ID, 30, NULL);

    gameRunning = TRUE;
}

// Generate a new obstacle
void GenerateObstacle() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i] == -1) {
            obstacles[i] = WINDOW_WIDTH;
            break;
        }
    }
}