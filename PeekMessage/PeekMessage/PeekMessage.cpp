// PeekMessage.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "PeekMessage.h"
#include<list>

using namespace std;

#define MAX_LOADSTRING 100

typedef struct _tagRectangle
{
	float left, top, right, bottom;
}RECTANGLE;

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
HDC g_hDC;
HWND g_hWnd;
bool g_bLoop = true;
RECTANGLE g_tPlayerRC = { 100, 100, 200, 200 };

typedef struct _tagBullet
{
	RECTANGLE rc;
	float fDist;
	float fLimitDist;

}BULLET, *PBULLET;

// 플레이어 총알
list<BULLET> g_PlayerBulletList;


// 시간을 구하기 위한 변수들
LARGE_INTEGER g_tSecond;
LARGE_INTEGER g_tTime;
float g_fDeltaTime;

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void Run();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PEEKMESSAGE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 응용 프로그램 초기화를 수행합니다.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	//화면용 DC 생성
	g_hDC = GetDC(g_hWnd);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PEEKMESSAGE));

    MSG msg;

	QueryPerformanceFrequency(&g_tSecond);
	QueryPerformanceCounter(&g_tTime);

    // 기본 메시지 루프입니다.
    while (g_bLoop)
    {
		//PeekMessage는 메시지가 메시지큐에 없어도 바로 빠져나온다.
		//메시지가 있을 경우 true, 없을 경우 false
		//메시지가 없는 시간이 윈도의 데드타임
		//PM_REMOVE는 메시지가 있을 경우 가져오면서, 그 메시지를 지워버림
        if (PeekMessage(&msg, nullptr, 0, 0,PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		//윈도우 데드타임인 경우
		else
		{
			//이 부분에서 게임을 구현함
			//윈도우의 이벤트가 발생하지 않는 타임

			Run();
		}
    }

	ReleaseDC(g_hWnd, g_hDC);

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PEEKMESSAGE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_PEEKMESSAGE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWnd = hWnd;

   // 실제 윈도우 타이틀바나 메뉴를 포함한 윈도의 크기를 구해준다.
   
   RECT rc = { 0,0,800,600 };
   AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

   // 위에서 구해준 크기로 윈도우 클라이언트 영역의 크기를
   // 원하는 크기로 맞춰줘야 한다. (윈도우 창의 위치 등등..)
   SetWindowPos(hWnd, HWND_TOPMOST, 100, 100, rc.right - rc.left, 
	   rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다.
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
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다.
            EndPaint(hWnd, &ps);
        }
        break;
		
		//윈도우 종료될 때 들어오는 메시지
    case WM_DESTROY:
		g_bLoop = false;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
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

void Run()
{
	//이벤트가 없을때만 들어옴

	// DeltaTime 구하기
	LARGE_INTEGER tTime;
	QueryPerformanceCounter(&tTime);
	g_fDeltaTime = (tTime.QuadPart - g_tTime.QuadPart) / (float)g_tSecond.QuadPart;
	g_tTime = tTime;
	//

	//Time Scale
	static float fTimeScale = 1.0f;
	if (GetAsyncKeyState(VK_F1))
	{
		fTimeScale -= g_fDeltaTime;

		if (fTimeScale < 0.f)
		{
			fTimeScale = 0.f;
		}
	}
	//

	// 플레이어 초당 이동속도 : 300
	float fSpeed = 300.0f * g_fDeltaTime * fTimeScale;
	// 총알 초당 이동속도 : 600
	float bulletSpeed = 600.0f * g_fDeltaTime * fTimeScale;

	if (GetAsyncKeyState(VK_SPACE))
	{
		//총알 발사
		BULLET tBullet;

		tBullet.rc.left = g_tPlayerRC.right;
		tBullet.rc.right = g_tPlayerRC.right + 50.0f;
		tBullet.rc.top = (g_tPlayerRC.top + g_tPlayerRC.bottom) / 2.0f - 25.0f;
		tBullet.rc.bottom = tBullet.rc.top + 50.0f;
		tBullet.fDist = 0.f;
		tBullet.fLimitDist = 100.f;

		g_PlayerBulletList.push_back(tBullet);
	}


	// 클라이언트창의 영역만 계산하여 가져온다.
	// 현재 클라이언트의 창을 가져온다.
	RECT rcWindow;
	GetClientRect(g_hWnd, &rcWindow);
	SetRect(&rcWindow, 0, 0, 800, 600);

	if (GetAsyncKeyState('D'))
	{
		if (g_tPlayerRC.right <= rcWindow.right)
		{
			g_tPlayerRC.left += fSpeed;
			g_tPlayerRC.right += fSpeed;
		}
	}

	if (GetAsyncKeyState('A'))
	{	
		if (g_tPlayerRC.left >= rcWindow.left)
		{
			g_tPlayerRC.left -= fSpeed;
			g_tPlayerRC.right -= fSpeed;
		}
	}

	if (GetAsyncKeyState('S'))
	{
		if (g_tPlayerRC.bottom <= rcWindow.bottom)
		{
			g_tPlayerRC.top += fSpeed;
			g_tPlayerRC.bottom += fSpeed;
		}
	}

	if (GetAsyncKeyState('W'))
	{
		if (g_tPlayerRC.top >= rcWindow.top)
		{
			g_tPlayerRC.top -= fSpeed;
			g_tPlayerRC.bottom -= fSpeed;
		}
	}

	//플레이어의 총알 이동
	list<BULLET>::iterator iter; //총알 발사 list의 반복자(iterator)
	list<BULLET>::iterator iterEnd = g_PlayerBulletList.end(); //list의 마지막

	//총알 속도 부여
	for (iter = g_PlayerBulletList.begin(); iter != iterEnd; )
	{
		(*iter).rc.left += bulletSpeed;
		(*iter).rc.right += bulletSpeed;

		(*iter).fDist += fSpeed;
		

		if ((*iter).rc.left >= 800)
		{
			iter = g_PlayerBulletList.erase(iter);
			iterEnd = g_PlayerBulletList.end();
		}
		else if ((*iter).fDist >= (*iter).fLimitDist)
		{
			iter = g_PlayerBulletList.erase(iter);
			iterEnd = g_PlayerBulletList.end();
		}
		else
		{
			++iter;
		}
	}

	//좌표 이동한 사각형의 잔상이 남지 않도록 화면을 덮는다.
	Rectangle(g_hDC, 0.0f, 0.0f, 800.0f, 600.0f);

	//플레이어(사각형) 출력
	Rectangle(g_hDC, g_tPlayerRC.left, g_tPlayerRC.top
		, g_tPlayerRC.right, g_tPlayerRC.bottom);

	for (iter = g_PlayerBulletList.begin(); iter != iterEnd; ++iter)
	{
		//총알 출력
		Rectangle(g_hDC, (*iter).rc.left, (*iter).rc.top, (*iter).rc.right, (*iter).rc.bottom);
	}
}
