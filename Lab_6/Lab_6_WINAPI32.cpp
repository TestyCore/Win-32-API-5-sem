#include <windows.h>
#include <cstdlib>
#include <string>
#include <CommCtrl.h>
#include <mutex>


#define ID_FILE_WRITE 100
#define ID_FILE_READ 101
#define ID_CALCULATE 102
#define ID_CALCULATE_2 103
#define ID_PROGRESS_STATE 104
#define ID_PROGRESS_STATE_2 105
#define ID_PROGRESS_BAR 106
#define ID_PROGRESS_BAR_2 107
#define ID_TERMINATE 108
#define ID_TERMINATE_2 109
#define ID_VALUE_VALUE 110


HMENU hMenu;
HWND hWnd, hResult, hSource, hProgress, hProgressBar, hProgress_2, hProgressBar_2, hValueValue;
wchar_t data[100];
int numBytesToWrite;
wchar_t res[100];
HANDLE writeCompleteEvent;
bool isInProgress, isInProgress_2;
HBRUSH hProgressBarBrush;
RECT progressBarRect, progressBarRect_2;

double value = 0;
CRITICAL_SECTION cs;
std::mutex mtx;
HANDLE semaphore;

void AddControls(HWND hWnd);
void AddMenus(HWND hWnd);
void WriteToFileAsync(HWND hWnd);
void ReadFromFileAsync(HWND hWnd);
void CalculateAsync(WPARAM wp);
void CalculateAsync_2(WPARAM wp);
void Terminate();
void Terminate_2();
void UpdateProgressState(WPARAM wp);
void UpdateProgressState_2(WPARAM wp);
void UpdateProgressBar(WPARAM wp);
void UpdateProgressBar_2(WPARAM wp);
void UpdateValueValue(WPARAM wp);
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
    isInProgress_2 = false;
    InitializeCriticalSection(&cs);
    semaphore = CreateSemaphore(NULL, 1, 1, NULL);
	hWnd = CreateWindowW(L"Class", L"Lab_4", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 20, 20, 500, 400, NULL, NULL, NULL, NULL);
    

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    DeleteCriticalSection(&cs);

	return 0;
}

DWORD WINAPI ProgressAsync(LPVOID lpParam) {
    for (int i = 1; i <= 100; i++) {
        for (int j = 0; j < 900000; j++) {
            if (isInProgress == false) {
                SendMessage(hWnd, ID_PROGRESS_STATE, 0, 0);
                SendMessage(GetDlgItem(hWnd, ID_PROGRESS_BAR), PBM_SETPOS, 0, 0);
                return 0;
            }
            //EnterCriticalSection(&cs);
            //std::lock_guard<std::mutex> lock(mtx);
            WaitForSingleObject(semaphore, INFINITE);
            value += 0.2;
            SendMessage(hWnd, ID_VALUE_VALUE, *reinterpret_cast<WPARAM*>(&value), 0);
            ReleaseSemaphore(semaphore, 1, NULL);
            //LeaveCriticalSection(&cs);
        }
        int progress = i;
        SendMessage(hWnd, ID_PROGRESS_STATE, *reinterpret_cast<WPARAM*>(&progress), 0);
        SendMessage(GetDlgItem(hWnd, ID_PROGRESS_BAR), PBM_SETPOS, progress, 0);

    }

    isInProgress = false;
    return 0;
}

DWORD WINAPI ProgressAsync_2(LPVOID lpParam) {
    for (int i = 1; i <= 100; i++) {
        for (int j = 0; j < 400000; j++) {
            if (isInProgress_2 == false) {
                SendMessage(hWnd, ID_PROGRESS_STATE_2, 0, 0);
                SendMessage(GetDlgItem(hWnd, ID_PROGRESS_BAR_2), PBM_SETPOS, 0, 0);
                return 0;
            }
            //EnterCriticalSection(&cs);
            //std::lock_guard<std::mutex> lock(mtx);
            WaitForSingleObject(semaphore, INFINITE);
            value += 0.000000001;
            SendMessage(hWnd, ID_VALUE_VALUE, *reinterpret_cast<WPARAM*>(&value), 0);
            ReleaseSemaphore(semaphore, 1, NULL);
            //LeaveCriticalSection(&cs);
        }
        int progress = i;
        SendMessage(hWnd, ID_PROGRESS_STATE_2, *reinterpret_cast<WPARAM*>(&progress), 0);
        SendMessage(GetDlgItem(hWnd, ID_PROGRESS_BAR_2), PBM_SETPOS, progress, 0);

    }

    isInProgress_2 = false;
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
        case ID_CALCULATE_2: {
            CalculateAsync_2(wp);
            break;
        }
        case ID_TERMINATE_2: {
            Terminate_2();
            break;
        }
        default: {
            break;
        }
		}
        break;

    case ID_PROGRESS_STATE: {
        UpdateProgressState(wp);
        break;
    }
    case ID_PROGRESS_STATE_2: {
        UpdateProgressState_2(wp);
        break;
    }
    case ID_VALUE_VALUE: {
        UpdateValueValue(wp);
        break;
    }


    case WM_PAINT:
    {
        UpdateProgressBar(wp);
        UpdateProgressBar_2(wp);
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


    hProgressBar_2 = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE, 40, 260, 245, 30, hWnd, (HMENU)ID_PROGRESS_BAR_2, NULL, NULL);
    hProgress_2 = CreateWindowW(L"Static", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 290, 260, 30, 30, hWnd, NULL, NULL, NULL);
    CreateWindowW(L"Button", L"Start", WS_VISIBLE | WS_CHILD, 340, 243, 100, 30, hWnd, (HMENU)ID_CALCULATE_2, NULL, NULL);
    CreateWindowW(L"Button", L"Terminate", WS_VISIBLE | WS_CHILD, 340, 283, 100, 30, hWnd, (HMENU)ID_TERMINATE_2, NULL, NULL);

    hValueValue = CreateWindowW(L"Static", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 290, 320, 100, 30, hWnd, NULL, NULL, NULL);

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

void CalculateAsync_2(WPARAM wp) {
    if (isInProgress_2) {
        return;
    }
    else
    {
        isInProgress_2 = true;
        HANDLE CalculateThread_2 = CreateThread(NULL, 0, ProgressAsync_2, NULL, 0, NULL);
        SetThreadPriority(CalculateThread_2, THREAD_PRIORITY_HIGHEST);
    }
}

void Terminate_2() {
    isInProgress_2 = false;
}

void UpdateProgressState(WPARAM wp) {
    int progress_int = *reinterpret_cast<int*>(&wp);
    WCHAR progress_wchar[4];
    swprintf_s(progress_wchar, L"%d", progress_int);
    LPCWSTR progress = progress_wchar;

    SetWindowText(hProgress, progress);
}

void UpdateProgressState_2(WPARAM wp) {
    int progress_int = *reinterpret_cast<int*>(&wp);
    WCHAR progress_wchar[4];
    swprintf_s(progress_wchar, L"%d", progress_int);
    LPCWSTR progress = progress_wchar;

    SetWindowText(hProgress_2, progress);
}

void UpdateValueValue(WPARAM wp) {
    double value_value = *reinterpret_cast<double*>(&wp);
    WCHAR value_value_wchar[20];
    swprintf_s(value_value_wchar, L"%f", value_value);
    LPCWSTR value = value_value_wchar;

    SetWindowText(hValueValue, value);
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

void UpdateProgressBar_2(WPARAM wp) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    int progress = *reinterpret_cast<int*>(&wp);
    progressBarRect_2.right = progressBarRect_2.left + progress;

    hProgressBarBrush = CreateSolidBrush(RGB(0, 0, 255));
    FillRect(hdc, &progressBarRect_2, hProgressBarBrush);

    EndPaint(hWnd, &ps);
}