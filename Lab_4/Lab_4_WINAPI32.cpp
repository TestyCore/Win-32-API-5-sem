#include <windows.h>
#include <cstdlib>
#include <string>
#include <CommCtrl.h>


#define ID_FILE_WRITE 100
#define ID_FILE_READ 101
#define ID_CALCULATE 102
#define ID_PROGRESS_STATE 103
#define ID_PROGRESS_BAR 104
#define ID_TERMINATE 105

HMENU hMenu;
HWND hWnd, hResult, hSource, hProgress, hProgressBar;
wchar_t data[100];
int numBytesToWrite;
wchar_t res[100];
HANDLE writeCompleteEvent;
bool isInProgress;
HBRUSH hProgressBarBrush;
RECT progressBarRect;


void AddControls(HWND hWnd);
void AddMenus(HWND hWnd);
void WriteToFileAsync(HWND hWnd);
void ReadFromFileAsync(HWND hWnd);
void CalculateAsync(WPARAM wp);
void Terminate();
void UpdateProgressState(WPARAM wp);
void UpdateProgressBar(WPARAM wp);
DWORD WINAPI WriteAsync(LPVOID filePointer);
DWORD WINAPI ReadAsync(LPVOID filePointer);
DWORD WINAPI ProgressAsync(LPVOID lpParam);
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR arts, int ncmdshow) {


	WNDCLASSW wc = { 0 };

	wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInst;
	wc.lpszClassName = L"Class";
	wc.lpfnWndProc = WindowProcedure;

	if (!RegisterClassW(&wc))
		return -1;

    isInProgress = false;
	hWnd = CreateWindowW(L"Class", L"Lab_4", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 20, 20, 500, 300, NULL, NULL, NULL, NULL);


	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

DWORD WINAPI ProgressAsync(LPVOID lpParam) {
    for (int i = 1; i <= 100; i++) {
        for (int j = 0; j < 70000000; j++) {
            if (isInProgress == false) {
                SendMessage(hWnd, ID_PROGRESS_STATE, 0, 0);
                SendMessage(GetDlgItem(hWnd, ID_PROGRESS_BAR), PBM_SETPOS, 0, 0);
                return 0;
            }
            int a = 6;
            int b = 7;
            int c = a + b;
        }
        int progress = i;
        SendMessage(hWnd, ID_PROGRESS_STATE, *reinterpret_cast<WPARAM*>(&progress), 0);
        SendMessage(GetDlgItem(hWnd, ID_PROGRESS_BAR), PBM_SETPOS, progress, 0);

    }

    isInProgress = false;
    return 0;
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg)
	{
	case WM_COMMAND:
		switch (wp)
		{
		case ID_FILE_WRITE: {
            WriteToFileAsync(hWnd);
			break;
		}
        case ID_FILE_READ: {
            ReadFromFileAsync(hWnd);
            break;
        }
        case ID_CALCULATE: {
            CalculateAsync(wp);
            break;
        }
        case ID_TERMINATE: {
            Terminate();
            break;
        }
		}
        break;

    case ID_PROGRESS_STATE: {
        UpdateProgressState(wp);
        break;
    }


    case WM_PAINT:
    {
        UpdateProgressBar(wp);
        break;
    }
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
    hSource = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 40, 40, 280, 30, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"Button", L"Write", WS_VISIBLE | WS_CHILD, 340, 40, 100, 30, hWnd, (HMENU)ID_FILE_WRITE, NULL, NULL);

    hResult = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 40, 100, 280, 30, hWnd, NULL, NULL, NULL);
    CreateWindowW(L"Button", L"Read", WS_VISIBLE | WS_CHILD, 340, 100, 100, 30, hWnd, (HMENU)ID_FILE_READ, NULL, NULL);


    hProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE, 40, 177, 245, 30, hWnd, (HMENU)ID_PROGRESS_BAR, NULL, NULL);
    hProgress = CreateWindowW(L"Static", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 290, 177, 30, 30, hWnd, NULL, NULL, NULL);
    CreateWindowW(L"Button", L"Start", WS_VISIBLE | WS_CHILD, 340, 160, 100, 30, hWnd, (HMENU)ID_CALCULATE, NULL, NULL);
    CreateWindowW(L"Button", L"Terminate", WS_VISIBLE | WS_CHILD, 340, 200, 100, 30, hWnd, (HMENU)ID_TERMINATE, NULL, NULL);

}

void WriteToFileAsync(HWND hWnd) {

    LPCWSTR  filename = L"mapped_file.txt";

    HANDLE fileHandle = CreateFile(
        filename,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL
    );

    GetWindowText(hSource, data, 100);
    numBytesToWrite = (wcslen(data) + 1) * sizeof(wchar_t);

    DWORD fileSize = numBytesToWrite;
    SetFilePointer(fileHandle, fileSize, NULL, FILE_BEGIN);
    SetEndOfFile(fileHandle);

    HANDLE fileMapping = CreateFileMapping(
        fileHandle,
        NULL,
        PAGE_READWRITE,
        0,
        fileSize,
        NULL
    );

    LPVOID filePointer = MapViewOfFile(fileMapping, FILE_MAP_WRITE, 0, 0, 0);

    HANDLE writeThread = CreateThread(NULL, 0, WriteAsync, filePointer,  0, NULL);

    WaitForSingleObject(writeThread, INFINITE);

    CloseHandle(writeThread);

    UnmapViewOfFile(filePointer);
    CloseHandle(fileMapping);
    CloseHandle(fileHandle);
}

void ReadFromFileAsync(HWND hWnd) {

    LPCWSTR  filename = L"mapped_file.txt";

    HANDLE fileHandle = CreateFile(
        filename,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL
    );

    DWORD fileSize = numBytesToWrite;
    SetFilePointer(fileHandle, fileSize, NULL, FILE_BEGIN);
    SetEndOfFile(fileHandle);

    HANDLE fileMapping = CreateFileMapping(
        fileHandle,
        NULL,
        PAGE_READWRITE,
        0,
        fileSize,
        NULL
    );

    LPVOID filePointer = MapViewOfFile(fileMapping, FILE_MAP_WRITE, 0, 0, 0);

    HANDLE readThread = CreateThread(NULL, 0, ReadAsync, filePointer, 0, NULL);

    WaitForSingleObject(readThread, INFINITE);

    CloseHandle(readThread);

    SetWindowText(hResult, res);

    UnmapViewOfFile(filePointer);
    CloseHandle(fileMapping);
    CloseHandle(fileHandle);
}


DWORD WINAPI WriteAsync(LPVOID filePointer) {
    CopyMemory(filePointer, data, numBytesToWrite);

    SetEvent(writeCompleteEvent);
    
    return 0;
}

DWORD WINAPI ReadAsync(LPVOID filePointer) {
    WaitForSingleObject(writeCompleteEvent, INFINITE);

    wchar_t* Buffer = new wchar_t[numBytesToWrite / sizeof(wchar_t)];
    CopyMemory(Buffer, filePointer, numBytesToWrite);
    wcscpy_s(res, Buffer);
    wcscat_s(res, L"\0");

    return 0;
}

void CalculateAsync(WPARAM wp) {
    if (isInProgress) {
        return;
    }
    else
    {
        isInProgress = true;
        HANDLE CalculateThread = CreateThread(NULL, 0, ProgressAsync, NULL, 0, NULL);
        SetThreadPriority(CalculateThread, THREAD_PRIORITY_HIGHEST);
    }
}

void Terminate() {
    isInProgress = false;
}

void UpdateProgressState(WPARAM wp) {
    int progress_int = *reinterpret_cast<int*>(&wp);
    WCHAR progress_wchar[4];
    swprintf_s(progress_wchar, L"%d", progress_int);
    LPCWSTR progress = progress_wchar;

    SetWindowText(hProgress, progress);
}

void UpdateProgressBar(WPARAM wp) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    int progress = *reinterpret_cast<int*>(&wp);
    progressBarRect.right = progressBarRect.left + progress;

    hProgressBarBrush = CreateSolidBrush(RGB(0, 0, 255));
    FillRect(hdc, &progressBarRect, hProgressBarBrush);

    EndPaint(hWnd, &ps);
}