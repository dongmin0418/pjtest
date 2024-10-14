#include <windows.h>
#include <stdbool.h>
#include <time.h>
#include <gdiplus.h> // GDI+ ��� �߰�
#pragma comment(lib, "Gdiplus.lib") // GDI+ ���̺귯�� �߰� 

// ȭ�� ũ�� �� ���� ��� ����
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GRAVITY 5
#define JUMP_STRENGTH -20
#define GROUND_HEIGHT 200
#define GROUND_Y (SCREEN_HEIGHT - GROUND_HEIGHT)

// �÷��̾� ����ü ����
typedef struct {
    int x, y;         // ��ġ
    int width, height; // ũ��
    int velocityY;    // Y�� �ӵ�
    bool isJumping;   // ���� ����
    bool isFacingLeft; // �¿� ����
} Player;

// �� ����ü ����
typedef struct {
    int x, y;         // ��ġ
    int width, height; // ũ��
    bool isRed;       // ���� ����
} Door;

// �� ����ü ����
typedef struct {
    int x, y;         // ��ġ
    int width, height; // ũ��
    bool isDeleted;   // ���� ����
} Ground;

// ��ֹ� ����ü ����
typedef struct {
    int x, y;         // ��ġ
    int width, height; // ũ��
} Obstacle;

// GDI+ �ʱ�ȭ �ڵ�
void InitializeGDIPlus(ULONG_PTR* gdiplusToken) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(gdiplusToken, &gdiplusStartupInput, NULL);
}

// GDI+ ���� �ڵ�
void ShutdownGDIPlus(ULONG_PTR gdiplusToken) {
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

// �̹��� ���� ����
Gdiplus::Bitmap* playerImageRight; // �������� ���� �̹���
Gdiplus::Bitmap* playerImageLeft;  // ������ ���� �̹���
Gdiplus::Bitmap* doorImage;         // �� �̹���
Gdiplus::Bitmap* obstacleImage;     // ��ֹ� �̹���

// ������ ���ν���
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// �÷��̾� �׸��� �Լ�
void DrawPlayer(HDC hdc, Player* player) {
    Gdiplus::Graphics graphics(hdc);
    // �¿� ���⿡ ���� �̹��� ����
    if (player->isFacingLeft) {
        graphics.DrawImage(playerImageLeft, player->x, player->y, player->width, player->height);
    }
    else {
        graphics.DrawImage(playerImageRight, player->x, player->y, player->width, player->height);
    }
}

// �� �׸��� �Լ�
void DrawDoor(HDC hdc, Door* door) {
    Gdiplus::Graphics graphics(hdc);
    graphics.DrawImage(doorImage, door->x, door->y, door->width, door->height);
}

// �浹 �˻� �Լ�
bool CheckCollision(Player* player, Door* door) {
    return (player->x < door->x + door->width &&
        player->x + player->width > door->x &&
        player->y < door->y + door->height &&
        player->y + player->height > door->y);
}

// �� �׸��� �Լ�
void DrawGround(HDC hdc, Ground* ground) {
    Gdiplus::Graphics graphics(hdc);
    Gdiplus::SolidBrush brush(Gdiplus::Color(255, 139, 69, 19)); // ���� ����

    if (!ground->isDeleted) {
        graphics.FillRectangle(&brush, ground->x, ground->y, ground->width, ground->height);
    }
    else if (ground->x == 400) {
        // 400������ ���� �׸�
        graphics.FillRectangle(&brush, ground->x, ground->y, 400, ground->height);
    }
    else if (ground->x == 420) {
        // 420 ������ ���� �׸�
        graphics.FillRectangle(&brush, ground->x + 20, ground->y, ground->width - 20, ground->height);
    }
}

// ���� ���� �޽��� �׸��� �Լ�
void DrawGameOver(HDC hdc) {
    Gdiplus::Graphics graphics(hdc);
    Gdiplus::FontFamily fontFamily(L"Arial");
    Gdiplus::Font font(&fontFamily, 48, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush brush(Gdiplus::Color(255, 255, 0, 0)); // ������
    Gdiplus::PointF point(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50);
    graphics.DrawString(L"You died", -1, &font, point, NULL, &brush);
}

// ��ֹ� �׸��� �Լ�
void DrawObstacle(HDC hdc, Obstacle* obstacle) {
    Gdiplus::Graphics graphics(hdc);
    graphics.DrawImage(obstacleImage, obstacle->x, obstacle->y, obstacle->width, obstacle->height);
}

// ���� �Լ�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    ULONG_PTR gdiplusToken;
    InitializeGDIPlus(&gdiplusToken); // GDI+ �ʱ�ȭ

    const wchar_t CLASS_NAME[] = L"GameWindowClass";

    // ������ Ŭ���� ���
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // ������ ����
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
        ShutdownGDIPlus(gdiplusToken); // GDI+ ����
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // �̹��� �ε�
    playerImageRight = Gdiplus::Bitmap::FromFile(L"Rface.png");  // ������ �̹���
    playerImageLeft = Gdiplus::Bitmap::FromFile(L"Lface.png");   // ���� �̹���
    doorImage = Gdiplus::Bitmap::FromFile(L"door.png"); // �� �̹���
    obstacleImage = Gdiplus::Bitmap::FromFile(L"pike.png"); // ��ֹ� �̹���

    // �̹��� �ε� Ȯ��
    if (doorImage == NULL || doorImage->GetLastStatus() != Gdiplus::Ok ||
        playerImageRight == NULL || playerImageRight->GetLastStatus() != Gdiplus::Ok ||
        playerImageLeft == NULL || playerImageLeft->GetLastStatus() != Gdiplus::Ok) {
        MessageBox(NULL, L"Failed to load images!", L"Error", MB_OK);
        ShutdownGDIPlus(gdiplusToken);
        return 0;
    }

    // �ʱ� ��ü ����
    Player player = { 100, SCREEN_HEIGHT - 30, 15, 30, 0, false, false }; // �ʱ� ����: ������
    Door door = { 500, SCREEN_HEIGHT - 265, 50, 70, false };
    Obstacle obstacle = { 430, GROUND_Y - 10, 10, 10 }; // ��ֹ�

    Ground ground1 = { 0, GROUND_Y, 400, GROUND_HEIGHT, false }; // 0 ~ 400: �� ����
    Ground ground2 = { 420, GROUND_Y, 380, GROUND_HEIGHT, false }; // 420 ~ 800: �� ����


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

        // Ű �Է� ó��
        if (GetAsyncKeyState('A') & 0x8000) {
            player.x -= 5;
            player.isFacingLeft = true; // ���� ����
        }
        if (GetAsyncKeyState('D') & 0x8000) {
            player.x += 5;
            player.isFacingLeft = false; // ������ ����
        }
        if (GetAsyncKeyState(VK_SPACE) & 0x8000 && !player.isJumping) {
            player.velocityY = JUMP_STRENGTH; // ����
            player.isJumping = true;
        }

        // �߷� ���� �� ���� ó��
        player.velocityY += GRAVITY;
        player.y += player.velocityY;

        // Ư�� X ��ǥ���� ���� �Ϻθ� ���� (���� ����)
        if (player.x > 350 && player.x < 380) {
            ground2.isDeleted = true;
        }
        else {
            ground2.isDeleted = false; // �ٽ� �ʱ�ȭ
        }

        // ������ �浹 ó��
        if (player.y >= GROUND_Y - player.height) {
            player.y = GROUND_Y - player.height; // �÷��̾ �� ���� ��ġ��Ŵ
            player.isJumping = false; // ���� ���� ����
            player.velocityY = 0; // �ӵ��� �ʱ�ȭ
        }

        // �÷��̾ ���ۿ� �������� �� ���� ���� ó��
        if (ground2.isDeleted && player.x >= 350 && player.x <= 380 && player.y >= GROUND_Y) {
            MessageBox(hwnd, L"You fell!", L"Game Over", MB_OK);
            running = false; // ���� ����
        }

        // ������ �浹 �˻�
        if (CheckCollision(&player, &door)) {
            // �浹 �� ���� ���� ó��
            MessageBox(hwnd, L"Clear!", L"Game Over", MB_OK);
            running = false; // ���� ����
        }

        // ��ֹ����� �浹 �˻�
        if (CheckCollision(&player, (Door*)&obstacle)) {
            // �浹 �� ���� ���� ó��
            MessageBox(hwnd, L"Game Over!", L"Game Over", MB_OK);
            running = false; // ���� ����
        }

        // ������ �׸���
        HDC hdc = GetDC(hwnd);
        // ��� �׸��� (���⼭�� ������ ������ ����)
        Gdiplus::Graphics graphics(hdc);
        graphics.Clear(Gdiplus::Color(255, 255, 255, 255)); // ��� ���

        // ��ü �׸���
        DrawGround(hdc, &ground1);
        DrawGround(hdc, &ground2);
        DrawPlayer(hdc, &player);
        DrawDoor(hdc, &door);
        DrawObstacle(hdc, &obstacle);

        // ���� ���� �� �޽��� �׸���
        if (!running) {
            DrawGameOver(hdc);
        }

        ReleaseDC(hwnd, hdc);
        Sleep(10); // ������ ����Ʈ ����
    }

    // ���� ���� �� �̹��� �޸� ����
    delete playerImageRight;
    delete playerImageLeft;
    delete doorImage;
    delete obstacleImage;

    ShutdownGDIPlus(gdiplusToken); // GDI+ ����
    return 0; // ���α׷� ����
}
