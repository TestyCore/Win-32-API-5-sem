#include <windows.h>
#include <stdlib.h>
#include <regex>

#define GENERATE_BUTTON 1

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

void AddMenus(HWND hWnd);
void AddControls(HWND hWnd);
void FillMonths();
bool Validate(std::wregex pattern, wchar_t string[], int size, const wchar_t err_msg[], HWND hWnd, HWND hCustom);

HMENU hMenu;
HWND hDay, hMonth, hDate, hMonthList;

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
			wchar_t day[100], month[12], date[30];
			std::wregex day_pattern(L"^(0?[1-9]|[12][0-9]|3[01])$");
			if (!Validate(day_pattern, day, 100, L"Invalid day input. Please enter a number between 1 and 31.", hWnd, hDay))
				break;
			
			int dayAsInt = _wtoi(day);
			int selectedMonthIndex = SendMessageW(hMonthList, CB_GETCURSEL, 0, 0);

			SendMessageW(hMonthList, CB_GETLBTEXT, selectedMonthIndex, (LPARAM)month);

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
	CreateWindowW(L"Static", L"Day (1 - 31):", WS_VISIBLE | WS_CHILD, 20, 40, 78, 38, hWnd, NULL, NULL, NULL);
	hDay = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 110, 40, 58, 38, hWnd, NULL, NULL, NULL);

	CreateWindowW(L"Static", L"Month:", WS_VISIBLE | WS_CHILD, 20, 90, 58, 38, hWnd, NULL, NULL, NULL);
	hMonthList = CreateWindowW(L"ComboBox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 90, 90, 110, 250, hWnd, NULL, NULL, NULL);
	FillMonths();

	CreateWindowW(L"Button", L"Generate Date", WS_VISIBLE | WS_CHILD, 20, 140, 98, 38, hWnd, (HMENU)GENERATE_BUTTON, NULL, NULL);
	hDate = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 20, 190, 180, 20, hWnd, NULL, NULL, NULL);
}

void FillMonths() {
	const wchar_t* months[] = { L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December" };
	for (int i = 0; i < 12; i++) {
		SendMessageW(hMonthList, CB_ADDSTRING, 0, (LPARAM)months[i]);
	}

	SendMessageW(hMonthList, CB_SETCURSEL, 0, 0);
}

bool Validate(std::wregex pattern, wchar_t string[], int size, const wchar_t err_msg[], HWND hWnd, HWND hCustom) {
	GetWindowText(hCustom, string, size);

	if (!std::regex_match(string, pattern)) {
		MessageBox(hWnd, err_msg, L"Error", MB_ICONERROR);
		SetWindowText(hCustom, L"");
		return false;
	}

	return true;
}