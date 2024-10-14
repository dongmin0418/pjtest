#include <windows.h>
#include <stdbool.h>
#include <time.h>
#include <gdiplus.h> // GDI+ 헤더 추가
#pragma comment(lib, "Gdiplus.lib") // GDI+ 라이브러리 추가 

// 화면 크기 및 물리 상수 정의
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GRAVITY 5
#define JUMP_STRENGTH -20
#define GROUND_HEIGHT 200
#define GROUND_Y (SCREEN_HEIGHT - GROUND_HEIGHT)

// 플레이어 구조체 정의
typedef struct {
    int x, y;         // 위치
    int width, height; // 크기
    int velocityY;    // Y축 속도
    bool isJumping;   // 점프 상태
    bool isFacingLeft; // 좌우 방향
} Player;

// 문 구조체 정의
typedef struct {
    int x, y;         // 위치
    int width, height; // 크기
    bool isRed;       // 색상 상태
} Door;

// 땅 구조체 정의
typedef struct {
    int x, y;         // 위치
    int width, height; // 크기
    bool isDeleted;   // 삭제 상태
} Ground;

// 장애물 구조체 정의
typedef struct {
    int x, y;         // 위치
    int width, height; // 크기
} Obstacle;

// GDI+ 초기화 코드
void InitializeGDIPlus(ULONG_PTR* gdiplusToken) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(gdiplusToken, &gdiplusStartupInput, NULL);
}

// GDI+ 정리 코드
void ShutdownGDIPlus(ULONG_PTR gdiplusToken) {
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

// 이미지 변수 선언
Gdiplus::Bitmap* playerImageRight; // 오른쪽을 보는 이미지
Gdiplus::Bitmap* playerImageLeft;  // 왼쪽을 보는 이미지
Gdiplus::Bitmap* doorImage;         // 문 이미지
Gdiplus::Bitmap* obstacleImage;     // 장애물 이미지

// 윈도우 프로시저
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// 플레이어 그리기 함수
void DrawPlayer(HDC hdc, Player* player) {
    Gdiplus::Graphics graphics(hdc);
    // 좌우 방향에 따라 이미지 선택
    if (player->isFacingLeft) {
        graphics.DrawImage(playerImageLeft, player->x, player->y, player->width, player->height);
    }
    else {
        graphics.DrawImage(playerImageRight, player->x, player->y, player->width, player->height);
    }
}

// 문 그리기 함수
void DrawDoor(HDC hdc, Door* door) {
    Gdiplus::Graphics graphics(hdc);
    graphics.DrawImage(doorImage, door->x, door->y, door->width, door->height);
}

// 충돌 검사 함수
bool CheckCollision(Player* player, Door* door) {
    return (player->x < door->x + door->width &&
        player->x + player->width > door->x &&
        player->y < door->y + door->height &&
        player->y + player->height > door->y);
}

// 땅 그리기 함수
void DrawGround(HDC hdc, Ground* ground) {
    Gdiplus::Graphics graphics(hdc);
    Gdiplus::SolidBrush brush(Gdiplus::Color(255, 139, 69, 19)); // 브라운 색상

    if (!ground->isDeleted) {
        graphics.FillRectangle(&brush, ground->x, ground->y, ground->width, ground->height);
    }
    else if (ground->x == 400) {
        // 400까지의 땅을 그림
        graphics.FillRectangle(&brush, ground->x, ground->y, 400, ground->height);
    }
    else if (ground->x == 420) {
        // 420 이후의 땅을 그림
        graphics.FillRectangle(&brush, ground->x + 20, ground->y, ground->width - 20, ground->height);
    }
}

// 게임 오버 메시지 그리기 함수
void DrawGameOver(HDC hdc) {
    Gdiplus::Graphics graphics(hdc);
    Gdiplus::FontFamily fontFamily(L"Arial");
    Gdiplus::Font font(&fontFamily, 48, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush brush(Gdiplus::Color(255, 255, 0, 0)); // 빨간색
    Gdiplus::PointF point(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50);
    graphics.DrawString(L"You died", -1, &font, point, NULL, &brush);
}

// 장애물 그리기 함수
void DrawObstacle(HDC hdc, Obstacle* obstacle) {
    Gdiplus::Graphics graphics(hdc);
    graphics.DrawImage(obstacleImage, obstacle->x, obstacle->y, obstacle->width, obstacle->height);
}

// 메인 함수
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    ULONG_PTR gdiplusToken;
    InitializeGDIPlus(&gdiplusToken); // GDI+ 초기화

    const wchar_t CLASS_NAME[] = L"GameWindowClass";

    // 윈도우 클래스 등록
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // 윈도우 생성
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Simple Game",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH, SCREEN_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        MessageBox(NULL, L"Window creation failed!", L"Error", MB_OK);
        ShutdownGDIPlus(gdiplusToken); // GDI+ 정리
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // 이미지 로드
    playerImageRight = Gdiplus::Bitmap::FromFile(L"Rface.png");  // 오른쪽 이미지
    playerImageLeft = Gdiplus::Bitmap::FromFile(L"Lface.png");   // 왼쪽 이미지
    doorImage = Gdiplus::Bitmap::FromFile(L"door.png"); // 문 이미지
    obstacleImage = Gdiplus::Bitmap::FromFile(L"pike.png"); // 장애물 이미지

    // 이미지 로드 확인
    if (doorImage == NULL || doorImage->GetLastStatus() != Gdiplus::Ok ||
        playerImageRight == NULL || playerImageRight->GetLastStatus() != Gdiplus::Ok ||
        playerImageLeft == NULL || playerImageLeft->GetLastStatus() != Gdiplus::Ok) {
        MessageBox(NULL, L"Failed to load images!", L"Error", MB_OK);
        ShutdownGDIPlus(gdiplusToken);
        return 0;
    }

    // 초기 객체 설정
    Player player = { 100, SCREEN_HEIGHT - 30, 15, 30, 0, false, false }; // 초기 방향: 오른쪽
    Door door = { 500, SCREEN_HEIGHT - 265, 50, 70, false };
    Obstacle obstacle = { 430, GROUND_Y - 10, 10, 10 }; // 장애물

    Ground ground1 = { 0, GROUND_Y, 400, GROUND_HEIGHT, false }; // 0 ~ 400: 땅 존재
    Ground ground2 = { 420, GROUND_Y, 380, GROUND_HEIGHT, false }; // 420 ~ 800: 땅 존재


    MSG msg;
    bool running = true;
    clock_t lastToggleTime = clock();

    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 키 입력 처리
        if (GetAsyncKeyState('A') & 0x8000) {
            player.x -= 5;
            player.isFacingLeft = true; // 왼쪽 방향
        }
        if (GetAsyncKeyState('D') & 0x8000) {
            player.x += 5;
            player.isFacingLeft = false; // 오른쪽 방향
        }
        if (GetAsyncKeyState(VK_SPACE) & 0x8000 && !player.isJumping) {
            player.velocityY = JUMP_STRENGTH; // 점프
            player.isJumping = true;
        }

        // 중력 적용 및 점프 처리
        player.velocityY += GRAVITY;
        player.y += player.velocityY;

        // 특정 X 좌표에서 땅의 일부를 삭제 (함정 구간)
        if (player.x > 350 && player.x < 380) {
            ground2.isDeleted = true;
        }
        else {
            ground2.isDeleted = false; // 다시 초기화
        }

        // 땅과의 충돌 처리
        if (player.y >= GROUND_Y - player.height) {
            player.y = GROUND_Y - player.height; // 플레이어를 땅 위에 위치시킴
            player.isJumping = false; // 점프 상태 해제
            player.velocityY = 0; // 속도를 초기화
        }

        // 플레이어가 구멍에 떨어졌을 때 게임 오버 처리
        if (ground2.isDeleted && player.x >= 350 && player.x <= 380 && player.y >= GROUND_Y) {
            MessageBox(hwnd, L"You fell!", L"Game Over", MB_OK);
            running = false; // 게임 종료
        }

        // 문과의 충돌 검사
        if (CheckCollision(&player, &door)) {
            // 충돌 시 게임 오버 처리
            MessageBox(hwnd, L"Clear!", L"Game Over", MB_OK);
            running = false; // 게임 종료
        }

        // 장애물과의 충돌 검사
        if (CheckCollision(&player, (Door*)&obstacle)) {
            // 충돌 시 게임 오버 처리
            MessageBox(hwnd, L"Game Over!", L"Game Over", MB_OK);
            running = false; // 게임 종료
        }

        // 윈도우 그리기
        HDC hdc = GetDC(hwnd);
        // 배경 그리기 (여기서는 간단한 색으로 설정)
        Gdiplus::Graphics graphics(hdc);
        graphics.Clear(Gdiplus::Color(255, 255, 255, 255)); // 흰색 배경

        // 객체 그리기
        DrawGround(hdc, &ground1);
        DrawGround(hdc, &ground2);
        DrawPlayer(hdc, &player);
        DrawDoor(hdc, &door);
        DrawObstacle(hdc, &obstacle);

        // 게임 오버 시 메시지 그리기
        if (!running) {
            DrawGameOver(hdc);
        }

        ReleaseDC(hwnd, hdc);
        Sleep(10); // 프레임 레이트 조절
    }

    // 게임 종료 시 이미지 메모리 해제
    delete playerImageRight;
    delete playerImageLeft;
    delete doorImage;
    delete obstacleImage;

    ShutdownGDIPlus(gdiplusToken); // GDI+ 정리
    return 0; // 프로그램 종료
}
