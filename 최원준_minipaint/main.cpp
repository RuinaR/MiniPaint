#include <windows.h>		// 윈도우 헤더파일
#include <windowsx.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <queue>
#include "resource.h"
using namespace std;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;										//인스턴스 핸들
LPCTSTR lpszClass = TEXT("최원준_22311019");				//제목 표시줄에 표시되는 내용

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam,
	int nCmdShow)
{
	HWND hWnd;											//윈도우 핸들 선언
	MSG Message;										//메세지 구조체 변수 선언
	WNDCLASS WndClass;									//Windows Class 구조체 변수 선언
	g_hInst = hInstance;								//hInstance값을 외부에서도 사용할 수 있도록 전역변수에 값을 저장

	WndClass.cbClsExtra = 0;							//예약 영역. 지금은 사용X
	WndClass.cbWndExtra = 0;							//예약 영역
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	// 윈도우의 배경 색상을 지정
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);		//윈도우의 마우스포인터 모양을 지정
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	//윈도우의 커서 아이콘 모양 지정
	WndClass.hInstance = hInstance;						//윈도우 클래스를 등록하는 프로그램 번호
	WndClass.lpfnWndProc = WndProc;						//윈도우 메세지 처리 함수 지정
	WndClass.lpszClassName = lpszClass;					//윈도우 클래스의 이름 지정
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);					//메뉴 지정
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;			//윈도우의 스타일을 정의

	RegisterClass(&WndClass);							//WNDCLASS 구조체의 번지를 전달


	hWnd = CreateWindow(lpszClass, lpszClass,			//윈도우를 생성
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT
		/* 100,100,500,500*/, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);

	//메세지 루프를 실행
	while (GetMessage(&Message, NULL, 0, 0)) {			//Queue에 있는 메세지를 읽어들인다
		TranslateMessage(&Message);						//키보드 입력 메세지를 가공하여 프로그램에서 쉽게 
														//  사용할 수 있도록
		DispatchMessage(&Message);						//메세지를 윈도우의 메세지 처리 함수 WndProc로 전달
	}
	return (int)Message.wParam;							//탈출 코드. 프로그램 종료
}


#define FONTS_MAX 6
#define FONT_SIZE_MIN 20
#define FONT_SIZE_MAX 30
#define FONT_SIZE_UNIT 2
#define STR_MAX 30

enum class SELECT
{
	NULL_SHAPE = 0,
	CIRCLE,
	RECT,
	LINE,
	FREELINE,
	STR,
	CLEAR,
	MAX
};

enum class STRWIN
{
	STR = 201,
	FONT,
	SIZE,
	SET
};

enum class CHILDWIN
{
	GROUP_S_COLOR = 0,
	S_RED,
	S_BLUE,
	S_GREEN,
	GROUP_L_COLOR,
	L_RED,
	L_BLUE,
	L_GREEN,
	CLEAR,
	MAX
};

typedef struct
{
	POINT start;
	POINT end;
}DrawRect, FreeLineUnit;

typedef struct
{
	DrawRect rect;
	COLORREF lineColor;
	COLORREF shapeColor;
	SELECT select;
}DrawInfo;

typedef struct
{
	WCHAR str[STR_MAX];
	WCHAR font[STR_MAX];
	int size;
}STRInfo;

typedef struct
{
	HWND hEdit_str;
	HWND hCombo_size;
	HWND hCombo_font;
	HWND hBtn_set;
	bool isOpen;
	bool isStrSet;
	bool isFontSet;
	bool isSizeSet;
}STRWin;

const COLORREF red = RGB(255, 0, 0);
const COLORREF green = RGB(0, 255, 0);
const COLORREF blue = RGB(0, 0, 255);
const COLORREF black = RGB(0, 0, 0);
const COLORREF white = RGB(255, 255, 255);

const WCHAR* fonts[FONTS_MAX] = { L"고딕체", L"굴림체", L"궁서체", L"휴먼옛체", L"휴먼엑스포", L"휴먼아미체" };
HDC hdcBuff;
HBITMAP hBmpBuff;
HBITMAP hBmpBuffOld;

HWND arrChildWin[(int)CHILDWIN::MAX];
HDC hdc;
PAINTSTRUCT ps;
HPEN hPen;
HPEN oldPen;
HBRUSH hBrush;
HBRUSH oldBrush;
HFONT hFont;
HFONT oldFont;
RECT clientRect;

STRInfo strInfo;
STRInfo strInfoTemp;
STRWin strWin;
DrawInfo drawInfo;
DrawRect beforeRect;
queue<FreeLineUnit> freeLineQ;
FreeLineUnit unit;
bool isClick;
bool isComplete;
bool isClickSetBTN;

void InitDrawInfoRect(void);
void InitBeforeRect(void);
void InitStrWin(void);
BOOL InRect(const RECT* rect, int mx, int my);
void ShowStrWin(void);
void HideStrWin(void);
void SetDrawData(HWND hWnd, WPARAM wParam);
bool SetStrData(HWND hWnd, WPARAM wParam);
void CreateStrWin(HWND hWnd);
void CreateChildWin(HWND hWnd);
void SetCurrentCheck(HWND hWnd);
void InitStrInfo(STRInfo* strInfo);
void SetStrInfo(STRInfo* strInfo, const STRInfo* strInfoTemp);
void Init_Create(HWND hWnd);
void LBtnDown(HWND hWnd, LPARAM lParam);
void LBtnUp(HWND hWnd);
void MouseMove(HWND hWnd, LPARAM lParam);
void Paint(HWND hWnd);
void End(void);

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage) {
	case WM_CREATE:
		Init_Create(hWnd);
		break;
	case WM_COMMAND:
		if (!SetStrData(hWnd, wParam))
			SetDrawData(hWnd, wParam);
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		LBtnDown(hWnd, lParam);
		break;
	case WM_LBUTTONUP:
		LBtnUp(hWnd);
		ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		if (isClick && !InRect(&clientRect, LOWORD(lParam), HIWORD(lParam)))
			SendMessage(hWnd, WM_LBUTTONUP, wParam, lParam);
		MouseMove(hWnd, lParam);
		break;
	case WM_PAINT:
		Paint(hWnd);
		break;
	case WM_DESTROY:
		End();
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void InitDrawInfoRect(void)
{
	drawInfo.rect.start = { 0,0 };
	drawInfo.rect.end = { 0,0 };
}
void InitBeforeRect(void)
{
	beforeRect.start = { 0,0 };
	beforeRect.end = { 0,0 };
}
void InitStrWin(void)
{
	strWin.hBtn_set = NULL;
	strWin.hCombo_font = NULL;
	strWin.hCombo_size = NULL;
	strWin.hEdit_str = NULL;
}

void ShowStrWin(void)
{
	ShowWindow(strWin.hBtn_set, SW_SHOW);
	ShowWindow(strWin.hCombo_font, SW_SHOW);
	ShowWindow(strWin.hCombo_size, SW_SHOW);
	ShowWindow(strWin.hEdit_str, SW_SHOW);
}

void HideStrWin(void)
{
	ShowWindow(strWin.hBtn_set, SW_HIDE);
	ShowWindow(strWin.hCombo_font, SW_HIDE);
	ShowWindow(strWin.hCombo_size, SW_HIDE);
	ShowWindow(strWin.hEdit_str, SW_HIDE);
}

BOOL InRect(const RECT* rect, int mx, int my)
{
	if (rect->left < mx &&
		rect->right > mx &&
		rect->top < my &&
		rect->bottom > my)
	{
		return TRUE;
	}
	return FALSE;
}

void SetCurrentCheck(HWND hWnd)
{
	CHILDWIN shape = CHILDWIN::MAX;
	CHILDWIN line = CHILDWIN::MAX;

	if (drawInfo.shapeColor == red)
		shape = CHILDWIN::S_RED;
	else if(drawInfo.shapeColor == blue)
		shape = CHILDWIN::S_BLUE;
	else if (drawInfo.shapeColor == green)
		shape = CHILDWIN::S_GREEN;

	if (drawInfo.lineColor == red)
		line = CHILDWIN::L_RED;
	else if (drawInfo.lineColor == blue)
		line = CHILDWIN::L_BLUE;
	else if (drawInfo.lineColor == green)
		line = CHILDWIN::L_GREEN;

	if (shape == CHILDWIN::MAX || line == CHILDWIN::MAX)
		return;

	CheckRadioButton(hWnd, (int)CHILDWIN::S_RED, (int)CHILDWIN::S_GREEN, (int)shape);
	CheckRadioButton(hWnd, (int)CHILDWIN::L_RED, (int)CHILDWIN::L_GREEN, (int)line);
}

void SetDrawData(HWND hWnd, WPARAM wParam)
{
	if (strWin.isOpen)
	{
		MessageBox(hWnd, L"문자열 메뉴가 완료되지 않았습니다.", L"메뉴 오류", MB_OK);
		SetCurrentCheck(hWnd);
		return;
	}
	isClick = false;
	switch (LOWORD(wParam))
	{
	case ID_CIRCLE:
		drawInfo.select = SELECT::CIRCLE;
		break;
	case ID_RECT:
		drawInfo.select = SELECT::RECT;
		break;
	case ID_LINE:
		drawInfo.select = SELECT::LINE;
		break;
	case ID_FREELINE:
		drawInfo.select = SELECT::FREELINE;
		break;
	case ID_S_RED:
	case (int)CHILDWIN::S_RED:
		drawInfo.shapeColor = red;
		CheckRadioButton(hWnd, (int)CHILDWIN::S_RED, (int)CHILDWIN::S_GREEN, (int)CHILDWIN::S_RED);
		break;
	case ID_S_BLUE:
	case (int)CHILDWIN::S_BLUE:
		drawInfo.shapeColor = blue;
		CheckRadioButton(hWnd, (int)CHILDWIN::S_RED, (int)CHILDWIN::S_GREEN, (int)CHILDWIN::S_BLUE);
		break;
	case ID_S_GREEN:
	case (int)CHILDWIN::S_GREEN:
		drawInfo.shapeColor = green;
		CheckRadioButton(hWnd, (int)CHILDWIN::S_RED, (int)CHILDWIN::S_GREEN, (int)CHILDWIN::S_GREEN);
		break;
	case ID_L_RED:
	case (int)CHILDWIN::L_RED:
		drawInfo.lineColor = red;
		CheckRadioButton(hWnd, (int)CHILDWIN::L_RED, (int)CHILDWIN::L_GREEN, (int)CHILDWIN::L_RED);
		break;
	case ID_L_BLUE:
	case (int)CHILDWIN::L_BLUE:
		drawInfo.lineColor = blue;
		CheckRadioButton(hWnd, (int)CHILDWIN::L_RED, (int)CHILDWIN::L_GREEN, (int)CHILDWIN::L_BLUE);
		break;
	case ID_L_GREEN:
	case (int)CHILDWIN::L_GREEN:
		drawInfo.lineColor = green;
		CheckRadioButton(hWnd, (int)CHILDWIN::L_RED, (int)CHILDWIN::L_GREEN, (int)CHILDWIN::L_GREEN);
		break;
	case ID_CLEAR:
	case (int)CHILDWIN::CLEAR:
		drawInfo.select = SELECT::CLEAR;
		InvalidateRect(hWnd, NULL, false);
		break;
	}
}

bool SetStrData(HWND hWnd, WPARAM wParam)
{
	bool result = true;
	switch (LOWORD(wParam))
	{
	case ID_STR:
		if (strWin.isOpen)
			break;
		strWin.isOpen = true;
		drawInfo.select = SELECT::STR;
		ShowStrWin();
		break;
	case (int)STRWIN::FONT:
		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			int i = SendMessage(strWin.hCombo_font, CB_GETCURSEL, 0, 0);
			swprintf_s(strInfoTemp.font, fonts[i]);
			strWin.isFontSet = true;
		}
		break;
	case (int)STRWIN::SIZE:
		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			strInfoTemp.size = (SendMessage(strWin.hCombo_size, CB_GETCURSEL, 0, 0) * FONT_SIZE_UNIT) + FONT_SIZE_MIN;
			strWin.isSizeSet = true;
		}
		else if (HIWORD(wParam) == CBN_EDITCHANGE)
		{
			char str[STR_MAX];
			GetWindowTextA(strWin.hCombo_size, str, STR_MAX);
			strInfoTemp.size = atoi(str);
			strWin.isSizeSet = true;
		}
		break;
	case (int)STRWIN::STR:
		if (HIWORD(wParam) == EN_CHANGE)
		{
			GetWindowTextW(strWin.hEdit_str, strInfoTemp.str, STR_MAX);
			strWin.isStrSet = true;
		}
		break;
	case (int)STRWIN::SET:
		isClickSetBTN = true;
		HideStrWin();
		//InitStrWin();
		strWin.isOpen = false;
		SetStrInfo(&strInfo, &strInfoTemp);
		break;
	case ID_EXIT:
		DestroyWindow(hWnd);
		break;
	default:
		result = false;
	}
	return result;
}

void CreateStrWin(HWND hWnd)
{
	strWin.hBtn_set =
		CreateWindow(TEXT("button"), TEXT("설정"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			220, 600, 100, 50, hWnd, (HMENU)STRWIN::SET, g_hInst, NULL);
	strWin.hEdit_str =
		CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			100, 600, 100, 50, hWnd, (HMENU)STRWIN::STR, g_hInst, NULL);
	Edit_LimitText(strWin.hEdit_str, STR_MAX);
	SetWindowTextW(strWin.hEdit_str, strInfo.str);
	strWin.hCombo_size =
		CreateWindow(TEXT("combobox"), NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWN,
			100, 500, 100, 200, hWnd, (HMENU)STRWIN::SIZE, g_hInst, NULL);
	for (int i = FONT_SIZE_MIN; i <= FONT_SIZE_MAX; i += FONT_SIZE_UNIT)
	{
		WCHAR size[10];
		wsprintfW(size, L"%d", i);
		SendMessageW(strWin.hCombo_size, CB_ADDSTRING, 0, (LPARAM)size);
	}
	ComboBox_LimitText(strWin.hCombo_size, 2);
	strWin.hCombo_font =
		CreateWindow(TEXT("combobox"), NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			220, 500, 100, 200, hWnd, (HMENU)STRWIN::FONT, g_hInst, NULL);
	for (int i = 0; i < FONTS_MAX; i++)
	{
		SendMessageW(strWin.hCombo_font, CB_ADDSTRING, 0, (LPARAM)fonts[i]);
	}

	if (strWin.isFontSet)
	{
		SendMessageW(strWin.hCombo_font, CB_SELECTSTRING, -1, (LPARAM)strInfo.font);
	}
	if (strWin.isSizeSet)
	{
		WCHAR size[10];
		wsprintfW(size, L"%d", strInfo.size);
		ComboBox_SetText(strWin.hCombo_size, size);
	}
}

void CreateChildWin(HWND hWnd)
{
	arrChildWin[(int)CHILDWIN::GROUP_S_COLOR] =
		CreateWindow(TEXT("button"), TEXT("면색 지정"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			200, 100, 300, 60, hWnd, (HMENU)CHILDWIN::GROUP_S_COLOR, g_hInst, NULL);
	arrChildWin[(int)CHILDWIN::S_RED] =
		CreateWindow(TEXT("button"), TEXT("빨강"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
			210, 130, 50, 30, hWnd, (HMENU)CHILDWIN::S_RED, g_hInst, NULL);
	arrChildWin[(int)CHILDWIN::S_BLUE] =
		CreateWindow(TEXT("button"), TEXT("파랑"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			290, 130, 50, 30, hWnd, (HMENU)CHILDWIN::S_BLUE, g_hInst, NULL);
	arrChildWin[(int)CHILDWIN::S_GREEN] =
		CreateWindow(TEXT("button"), TEXT("초록"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			370, 130, 50, 30, hWnd, (HMENU)CHILDWIN::S_GREEN, g_hInst, NULL);

	arrChildWin[(int)CHILDWIN::GROUP_L_COLOR] =
		CreateWindow(TEXT("button"), TEXT("선색 지정"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			200, 200, 300, 60, hWnd, (HMENU)CHILDWIN::GROUP_L_COLOR, g_hInst, NULL);
	arrChildWin[(int)CHILDWIN::L_RED] =
		CreateWindow(TEXT("button"), TEXT("빨강"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
			210, 220, 50, 30, hWnd, (HMENU)CHILDWIN::L_RED, g_hInst, NULL);
	arrChildWin[(int)CHILDWIN::L_BLUE] =
		CreateWindow(TEXT("button"), TEXT("파랑"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			290, 220, 50, 30, hWnd, (HMENU)CHILDWIN::L_BLUE, g_hInst, NULL);
	arrChildWin[(int)CHILDWIN::L_GREEN] =
		CreateWindow(TEXT("button"), TEXT("초록"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			370, 220, 50, 30, hWnd, (HMENU)CHILDWIN::L_GREEN, g_hInst, NULL);

	arrChildWin[(int)CHILDWIN::CLEAR] =
		CreateWindow(TEXT("button"), TEXT("지우기"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			500, 165, 100, 50, hWnd, (HMENU)CHILDWIN::CLEAR, g_hInst, NULL);
}

void InitStrInfo(STRInfo* strInfo)
{
	swprintf_s(strInfo->font, L"고딕체");
	swprintf_s(strInfo->str, L"");
	strInfo->size = FONT_SIZE_MIN;
}

void SetStrInfo(STRInfo* strInfo, const STRInfo* strInfoTemp)
{
	swprintf_s(strInfo->font, strInfoTemp->font);
	swprintf_s(strInfo->str, strInfoTemp->str);
	strInfo->size = strInfoTemp->size;
}

void Init_Create(HWND hWnd)
{
	GetClientRect(hWnd, &clientRect);
	hdc = GetDC(hWnd);
	hdcBuff = CreateCompatibleDC(hdc);
	hBmpBuff = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
	hBmpBuffOld = (HBITMAP)SelectObject(hdcBuff, hBmpBuff);
	FillRect(hdcBuff, &clientRect, (HBRUSH)COLOR_WINDOWFRAME);
	SelectObject(hdcBuff, hBmpBuffOld);
	DeleteDC(hdcBuff);
	ReleaseDC(hWnd, hdc);

	InitDrawInfoRect();
	InitBeforeRect();
	drawInfo.lineColor = black;
	drawInfo.shapeColor = black;
	drawInfo.select = SELECT::NULL_SHAPE;
	InitStrInfo(&strInfo);
	InitStrInfo(&strInfoTemp);
	InitStrWin();
	strWin.isOpen = false;
	strWin.isFontSet = false;
	strWin.isSizeSet = false;
	strWin.isStrSet = false;
	isClick = false;
	isComplete = false;
	isClickSetBTN = false;
	CreateChildWin(hWnd);
	CreateStrWin(hWnd);
	HideStrWin();
}

void LBtnDown(HWND hWnd, LPARAM lParam)
{
	if (drawInfo.select == SELECT::NULL_SHAPE)
		return;
	if (drawInfo.select == SELECT::STR && !strWin.isOpen)
	{
		drawInfo.rect.start = { LOWORD(lParam), HIWORD(lParam) };
		InvalidateRect(hWnd, NULL, false);
		return;
	}
	isClick = true;
	InitBeforeRect();
	drawInfo.rect.start = { LOWORD(lParam), HIWORD(lParam) };
	drawInfo.rect.end = drawInfo.rect.start;
	unit.start = drawInfo.rect.start;
	unit.end = drawInfo.rect.end;
}

void LBtnUp(HWND hWnd)
{
	if (drawInfo.select == SELECT::NULL_SHAPE || drawInfo.select == SELECT::STR ||!isClick)
		return;
	isClick = false;
	isComplete = true;
	InvalidateRect(hWnd, NULL, false);
}

void MouseMove(HWND hWnd, LPARAM lParam)
{
	if (!isClick || drawInfo.select == SELECT::NULL_SHAPE || drawInfo.select == SELECT::STR)
		return;
	if (drawInfo.select == SELECT::FREELINE)
	{
		unit.start = unit.end;
		unit.end = { LOWORD(lParam), HIWORD(lParam) };
		freeLineQ.push(unit);
	}
	else
	{
		drawInfo.rect.end = { LOWORD(lParam), HIWORD(lParam) };
	}
	InvalidateRect(hWnd, NULL, false);
}

void Paint(HWND hWnd)
{
	hdc = BeginPaint(hWnd, &ps);
	hdcBuff = CreateCompatibleDC(hdc);
	hBmpBuffOld = (HBITMAP)SelectObject(hdcBuff, hBmpBuff);

	//펜, 브러시 생성하고 등록
	hPen = CreatePen(PS_SOLID, 3, drawInfo.lineColor);
	hBrush = CreateSolidBrush(drawInfo.shapeColor);
	oldPen = (HPEN)SelectObject(hdcBuff, hPen);
	oldBrush = (HBRUSH)SelectObject(hdcBuff, hBrush);
	//그리기	
	if (isComplete)
	{
		SetROP2(hdcBuff, R2_COPYPEN);
		isComplete = false;
	}
	else
		SetROP2(hdcBuff, R2_NOTXORPEN);

	switch (drawInfo.select)
	{
	case SELECT::CIRCLE:
		Ellipse(hdcBuff, beforeRect.start.x, beforeRect.start.y, beforeRect.end.x, beforeRect.end.y);
		Ellipse(hdcBuff, drawInfo.rect.start.x, drawInfo.rect.start.y, drawInfo.rect.end.x, drawInfo.rect.end.y);
		beforeRect = drawInfo.rect;
		break;
	case SELECT::RECT:
		Rectangle(hdcBuff, beforeRect.start.x, beforeRect.start.y, beforeRect.end.x, beforeRect.end.y);
		Rectangle(hdcBuff, drawInfo.rect.start.x, drawInfo.rect.start.y, drawInfo.rect.end.x, drawInfo.rect.end.y);
		beforeRect = drawInfo.rect;
		break;
	case SELECT::LINE:
		MoveToEx(hdcBuff, beforeRect.start.x, beforeRect.start.y, NULL);
		LineTo(hdcBuff, beforeRect.end.x, beforeRect.end.y);
		MoveToEx(hdcBuff, drawInfo.rect.start.x, drawInfo.rect.start.y, NULL);
		LineTo(hdcBuff, drawInfo.rect.end.x, drawInfo.rect.end.y);
		beforeRect = drawInfo.rect;
		break;
	case SELECT::FREELINE:
		SetROP2(hdcBuff, R2_COPYPEN);
		while (!freeLineQ.empty())
		{
			FreeLineUnit drawUnit = freeLineQ.front();
			freeLineQ.pop();
			MoveToEx(hdcBuff, drawUnit.start.x, drawUnit.start.y, NULL);
			LineTo(hdcBuff, drawUnit.end.x, drawUnit.end.y);
		}
		break;
	case SELECT::STR:
		if (isClickSetBTN || strWin.isOpen)
		{
			isClickSetBTN = false;
			break;
		}
		hFont = CreateFontW(strInfo.size, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
			VARIABLE_PITCH | FF_ROMAN, strInfo.font);
		oldFont = (HFONT)SelectObject(hdcBuff, hFont);
		TextOutW(hdcBuff, drawInfo.rect.start.x, drawInfo.rect.start.y, strInfo.str, lstrlenW(strInfo.str));
		SelectObject(hdcBuff, oldFont);
		DeleteObject(hFont);
		break;
	case SELECT::CLEAR:
		FillRect(hdcBuff, &clientRect, (HBRUSH)COLOR_WINDOWFRAME);
		break;
	}

	//펜, 브러시 돌려놓고 삭제
	SelectObject(hdcBuff, oldPen);
	SelectObject(hdcBuff, oldBrush);
	DeleteObject(hPen);
	DeleteObject(hBrush);

	BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcBuff, 0, 0, SRCCOPY);
	SelectObject(hdcBuff, hBmpBuffOld);

	DeleteDC(hdcBuff);
	EndPaint(hWnd, &ps);
}

void End(void)
{
	DeleteObject(hBmpBuff);
}