#include <windows.h>
#include <stdlib.h>

#define GENERATE_BUTTON 1

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

void AddMenus(HWND hWnd);
void AddControls(HWND hWnd);

HMENU hMenu;
HWND hDay, hMonth, hDate;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR arts, int ncmdshow) {

	WNDCLASSW wc = { 0 };

	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInst;
	wc.lpszClassName = L"Class";
	wc.lpfnWndProc = WindowProcedure;

	if (!RegisterClassW(&wc))
		return -1;

	CreateWindowW(L"Class", L"Lab_1", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 500, 500, NULL, NULL, NULL, NULL);

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg)
	{
	case WM_COMMAND:
		switch (wp)
		{
		case GENERATE_BUTTON:
			wchar_t day[3], month[12], date[30];
			GetWindowText(hDay, day, 3);
			GetWindowText(hMonth, month, 12);

			if (wcslen(day) > 0 && wcslen(month) > 0) {
				wcscpy_s(date, L"Today is ");
				wcscat_s(date, day);
				wcscat_s(date, L" of ");
				wcscat_s(date, month);
			}
			else {
				wcscpy_s(date, L"Empty field(s) exist");
			}
			

			SetWindowText(hDate, date);

			break;
		}
		break;
	case WM_CREATE:
		AddMenus(hWnd);
		AddControls(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProcW(hWnd, msg, wp, lp);
	}
}

void AddMenus(HWND hWnd) {
	hMenu = CreateMenu();

	SetMenu(hWnd, hMenu);
}


void AddControls(HWND hWnd) {
	CreateWindowW(L"Static", L"Day (1 - 31):", WS_VISIBLE | WS_CHILD, 100, 50, 98, 38, hWnd, NULL, NULL, NULL);
	hDay = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 200, 50, 98, 38, hWnd, NULL, NULL, NULL);

	CreateWindowW(L"Static", L"Month:", WS_VISIBLE | WS_CHILD, 100, 90, 98, 38, hWnd, NULL, NULL, NULL);
	hMonth = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 200, 90, 98, 38, hWnd, NULL, NULL, NULL);

	CreateWindowW(L"Button", L"Generate Date", WS_VISIBLE | WS_CHILD, 100, 140, 98, 38, hWnd, (HMENU)GENERATE_BUTTON, NULL, NULL);

	hDate = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 200, 300, 20, hWnd, NULL, NULL, NULL);
}