#define ID_SHOW_CHART 101
#define ID_HIDE_CHART 102
#define ID_CLEAR_CHART 103
#define ID_REMOVE_RECORD_CHART 104
#define ID_CREATE_RECORD 105


#include <windows.h>
#include <regex>
#include <cstdlib>
#include <vector>



HMENU hMenu;
HWND hDay, hMonthList, hDate, hSales;


HHOOK hKeyboardHook = NULL;
HWND hActiveWindow = NULL;
bool showChart = false;

struct Date {
	int day;
	int month;
	int year;
};

std::vector<Date> DateArr;
std::vector<int> NumArr;


LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void AddMenus(HWND hWnd);
void AddControls(HWND hWnd);
bool dateMatches(const Date& date, int day, int month, int year);
void CreateRecord(HWND hWnd);
void RemoveRecord(HWND hWnd);
void ClearChart();
bool Validate(std::wregex pattern, wchar_t string[], int size, const wchar_t err_msg[], HWND hWnd, HWND hCustom);
void FillMonths();
void CreateGraphs(HWND hWnd);
void BuildColDiagram(HDC hdc);
void BuildPlot(HDC hdc);



LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
			KBDLLHOOKSTRUCT* pKbStruct = (KBDLLHOOKSTRUCT*)lParam;

			if (pKbStruct->scanCode == 0x1C) {
				hActiveWindow = GetForegroundWindow();

				HWND hButton = GetDlgItem(hActiveWindow, ID_CREATE_RECORD);
				HWND hButton_2 = GetDlgItem(hActiveWindow, ID_SHOW_CHART);

				if (hButton != NULL) {
					SendMessage(hButton, BM_CLICK, 0, 0);
				}

				if (hButton_2 != NULL) {
					SendMessage(hButton_2, BM_CLICK, 0, 0);
				}
			}
		}
	}

	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR arts, int ncmdshow) {


	WNDCLASSW wc = { 0 };

	wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInst;
	wc.lpszClassName = L"Class";
	wc.lpfnWndProc = WindowProcedure;

	if (!RegisterClassW(&wc))
		return -1;

	CreateWindowW(L"Class", L"Lab_1_2", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 20, 20, 1800, 500, NULL, NULL, NULL, NULL);


	HHOOK hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInst, 0);

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(hKeyboardHook);

	return 0;
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg)
	{
		case WM_COMMAND:
			switch (wp)
			{
				case ID_CREATE_RECORD: {
					CreateRecord(hWnd);
					InvalidateRect(hWnd, NULL, TRUE);
					break;
				}
				case ID_SHOW_CHART: {
					showChart = true;
					InvalidateRect(hWnd, NULL, TRUE);
					break;
				}
				case ID_HIDE_CHART: {
					showChart = false;
					InvalidateRect(hWnd, NULL, TRUE);
					break;
				}
				case ID_CLEAR_CHART: {
					showChart = false;
					ClearChart();
					InvalidateRect(hWnd, NULL, TRUE);
					break;
				}
				case ID_REMOVE_RECORD_CHART: {
					RemoveRecord(hWnd);
					break;
				}
			}
			 break;

		case WM_PAINT:
			CreateGraphs(hWnd);	
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

	CreateWindowW(L"Static", L"Sales:", WS_VISIBLE | WS_CHILD, 20, 50, 58, 38, hWnd, NULL, NULL, NULL);
	hSales = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 90, 50, 58, 38, hWnd, NULL, NULL, NULL);

	CreateWindowW(L"Static", L"Day (1 - 31):", WS_VISIBLE | WS_CHILD, 20, 90, 78, 38, hWnd, NULL, NULL, NULL);
	hDay = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 110, 90, 58, 38, hWnd, NULL, NULL, NULL);

	CreateWindowW(L"Static", L"Month:", WS_VISIBLE | WS_CHILD, 20, 140, 58, 38, hWnd, NULL, NULL, NULL);
	hMonthList = CreateWindowW(L"ComboBox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 90, 140, 110, 250, hWnd, NULL, NULL, NULL);
	FillMonths();

	CreateWindowW(L"Button", L"Generate Date", WS_VISIBLE | WS_CHILD, 20, 190, 98, 38, hWnd, (HMENU)ID_CREATE_RECORD, NULL, NULL);
	hDate = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 20, 240, 180, 20, hWnd, NULL, NULL, NULL);

	CreateWindowW(L"Button", L"Show Chart", WS_VISIBLE | WS_CHILD, 50, 400, 100, 30, hWnd, (HMENU)ID_SHOW_CHART, NULL, NULL);
	CreateWindowW(L"Button", L"Hide Chart", WS_VISIBLE | WS_CHILD, 200, 400, 100, 30, hWnd, (HMENU)ID_HIDE_CHART, NULL, NULL);
	CreateWindowW(L"Button", L"Clear Chart", WS_VISIBLE | WS_CHILD, 350, 400, 100, 30, hWnd, (HMENU)ID_CLEAR_CHART, NULL, NULL);
	CreateWindowW(L"Button", L"Remove record", WS_VISIBLE | WS_CHILD, 500, 400, 100, 30, hWnd, (HMENU)ID_REMOVE_RECORD_CHART, NULL, NULL);
}

bool DateMatches(const Date& date, int day, int month, int year) {
	return (date.day == day && date.month == month && date.year == year);
}

void CreateRecord(HWND hWnd) {
	wchar_t day[3], month[12], date[30], sales[4];

	std::wregex day_pattern(L"^(0?[1-9]|[12][0-9]|3[01])$");
	if (!Validate(day_pattern, day, 3, L"Invalid day input. Please enter a number between 1 and 31.", hWnd, hDay))
		return;

	std::wregex sales_pattern(L"^(1000|([0-9]|[1-9][0-9]{0,2}))$");
	if (!Validate(sales_pattern, sales, 4, L"Invalid sales input. Please enter a number between 0 and 999.", hWnd, hSales))
		return;

	int dayAsInt = _wtoi(day);
	int selectedMonthIndex = SendMessageW(hMonthList, CB_GETCURSEL, 0, 0);
	int salesAsInt = _wtoi(sales);

	if (DateArr.size() < 9) {
		DateArr.push_back(Date{ dayAsInt, selectedMonthIndex + 1, 2023 });
		NumArr.push_back(salesAsInt);
		
	}

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

}

void RemoveRecord(HWND hWnd) {
	if (DateArr.size() < 0) {
		return;
	}

	wchar_t day[3], month[12], date[30];

	std::wregex day_pattern(L"^(0?[1-9]|[12][0-9]|3[01])$");
	if (!Validate(day_pattern, day, 3, L"Invalid day input. Please enter a number between 1 and 31.", hWnd, hDay))
		return;

	int dayAsInt = _wtoi(day);
	int selectedMonthIndex = SendMessageW(hMonthList, CB_GETCURSEL, 0, 0);

	auto it = std::find_if(DateArr.begin(), DateArr.end(),
		[&](const Date& date) {
			return DateMatches(date, dayAsInt, selectedMonthIndex + 1, 2023);
		});

	if (it != DateArr.end()) {
		int index = std::distance(DateArr.begin(), it);
		DateArr.erase(DateArr.begin() + index);

		NumArr.erase(NumArr.begin() + index);
	}
	InvalidateRect(hWnd, NULL, TRUE);
}

void CreateGraphs(HWND hWnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);

	if (showChart == false || DateArr.size() == 0)
		return;

	BuildColDiagram(hdc);
	BuildPlot(hdc);

	EndPaint(hWnd, &ps);
}

void BuildColDiagram(HDC hdc) {
	int MarginLeft = 280;
	int MarginTop = 50;
	int Width = 350;
	int Height = 300;
	int TotalWidth = Width + MarginLeft;
	int TotalHeight = Height + MarginTop;
	int ColumnSpacing = 30;

	int BarWidth = (Width - (DateArr.size() - 1) * ColumnSpacing) / DateArr.size();
	int MaxValue = *std::max_element(NumArr.begin(), NumArr.end());

	for (int i = 0; i < DateArr.size(); ++i) {
		int barHeight = (NumArr[i] * Height) / MaxValue;
		int x = MarginLeft + (BarWidth + ColumnSpacing) * i;
		int y = MarginTop + Height - barHeight;
		int barColor = RGB(0, 0, 255);

		RECT rect = { x, y, x + BarWidth, MarginTop + Height };
		HBRUSH hBrush = CreateSolidBrush(barColor);
		FillRect(hdc, &rect, hBrush);
		DeleteObject(hBrush);

		WCHAR dateText[64];
		swprintf_s(dateText, _countof(dateText), L"%02d/%02d", DateArr[i].day, DateArr[i].month);
		TextOut(hdc, x, MarginTop + Height + 5, dateText, lstrlen(dateText));
	}

	for (int i = 0; i <= 10; ++i) {
		int yLabel = MarginTop + Height - (i * Height / 10);
		WCHAR label[16];
		swprintf_s(label, _countof(label), L"%d", i * MaxValue / 10);
		TextOut(hdc, MarginLeft - 30, yLabel - 10, label, lstrlen(label));

		MoveToEx(hdc, MarginLeft, yLabel, nullptr);
		LineTo(hdc, MarginLeft + Width, yLabel);
	}
}

void BuildPlot(HDC hdc) {
	int MarginLeft = 720;
	int MarginTop = 50;
	int Width = 350;
	int Height = 300;
	int TotalWidth = Width + MarginLeft;
	int TotalHeight = Height + MarginTop;
	int ColumnSpacing = 30;

	int MaxValue = *std::max_element(NumArr.begin(), NumArr.end());

	float stepX = 0;
	if (DateArr.size() == 1) {
		stepX = Width;
	}
	else {
		stepX = static_cast<float>(Width / DateArr.size());
	}
	float scaleY = static_cast<float>(Height) / MaxValue;

	int graphX = MarginLeft;
	int graphY = MarginTop;

	HPEN grayPen = CreatePen(PS_SOLID, 1, RGB(192, 192, 192));
	SelectObject(hdc, grayPen);

	for (int i = 0; i <= 10; ++i) {
		int yGridLine = graphY + Height - (i * Height / 10);
		MoveToEx(hdc, graphX, yGridLine, nullptr);
		LineTo(hdc, graphX + Width, yGridLine);

		WCHAR yAxisLabel[16];
		swprintf_s(yAxisLabel, _countof(yAxisLabel), L"%d", i * MaxValue / 10);
		TextOut(hdc, graphX - 40, yGridLine - 10, yAxisLabel, lstrlen(yAxisLabel));
	}


	for (int i = 0; i < DateArr.size(); i++) {
		int xLabel = static_cast<int>(graphX + i * stepX);
		WCHAR label[16];
		swprintf_s(label, _countof(label), L"%02d/%02d", DateArr[i].day, DateArr[i].month);
		TextOut(hdc, xLabel - 20, graphY + Height + 10, label, lstrlen(label));
	}

	HPEN bluePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
	SelectObject(hdc, bluePen);
	MoveToEx(hdc, graphX, graphY + Height - NumArr[0] * scaleY, NULL);
	for (int i = 0; i < DateArr.size(); i++) {
		int x = static_cast<int>(graphX + (i) * stepX);
		int y = graphY + Height - NumArr[i] * scaleY;
		LineTo(hdc, x, y);
	}

	DeleteObject(bluePen);
	DeleteObject(grayPen);
}

void ClearChart() {
	DateArr.clear();
	NumArr.clear();
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

void FillMonths() {
	const wchar_t* months[] = { L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December" };
	for (int i = 0; i < 12; i++) {
		SendMessageW(hMonthList, CB_ADDSTRING, 0, (LPARAM)months[i]);
	}

	SendMessageW(hMonthList, CB_SETCURSEL, 0, 0);
}