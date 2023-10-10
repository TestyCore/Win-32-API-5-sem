#include <windows.h>
#include <cstdlib>

#define ID_FILE_WRITE 100
#define ID_FILE_READ 101

HMENU hMenu;
HWND hResult, hSource;
wchar_t data[100];
int numBytesToWrite;
wchar_t res[100];
HANDLE writeCompleteEvent;


void AddControls(HWND hWnd);
void AddMenus(HWND hWnd);
void WriteToFileAsync(HWND hWnd);
void ReadFromFileAsync(HWND hWnd);
DWORD WINAPI WriteAsync(LPVOID filePointer);
DWORD WINAPI ReadAsync(LPVOID filePointer);
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

	CreateWindowW(L"Class", L"Lab_3", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 20, 20, 500, 200, NULL, NULL, NULL, NULL);


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
		case ID_FILE_WRITE: {
            WriteToFileAsync(hWnd);
			break;
		}
        case ID_FILE_READ: {
            ReadFromFileAsync(hWnd);
            break;
        }
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
    hSource = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 40, 40, 280, 20, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"Button", L"Write", WS_VISIBLE | WS_CHILD, 340, 40, 100, 30, hWnd, (HMENU)ID_FILE_WRITE, NULL, NULL);

    hResult = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 40, 100, 280, 20, hWnd, NULL, NULL, NULL);
    CreateWindowW(L"Button", L"Read", WS_VISIBLE | WS_CHILD, 340, 100, 100, 30, hWnd, (HMENU)ID_FILE_READ, NULL, NULL);
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
    // Установка размера файла
    SetFilePointer(fileHandle, fileSize, NULL, FILE_BEGIN);
    SetEndOfFile(fileHandle);

    // Отображение файла в память
    HANDLE fileMapping = CreateFileMapping(
        fileHandle,
        NULL,
        PAGE_READWRITE,
        0,
        fileSize,
        NULL
    );

    // Получаем указатель на отображенный файл
    LPVOID filePointer = MapViewOfFile(fileMapping, FILE_MAP_WRITE, 0, 0, 0);

    HANDLE writeThread = CreateThread(NULL, 0, WriteAsync, filePointer,  0, NULL);

    WaitForSingleObject(writeThread, INFINITE);

    CloseHandle(writeThread);
   
    // Освобождаем ресурсы
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
    // Установка размера файла
    SetFilePointer(fileHandle, fileSize, NULL, FILE_BEGIN);
    SetEndOfFile(fileHandle);

    // Отображение файла в память
    HANDLE fileMapping = CreateFileMapping(
        fileHandle,
        NULL,
        PAGE_READWRITE,
        0,
        fileSize,
        NULL
    );

    // Получаем указатель на отображенный файл
    LPVOID filePointer = MapViewOfFile(fileMapping, FILE_MAP_WRITE, 0, 0, 0);

    HANDLE readThread = CreateThread(NULL, 0, ReadAsync, filePointer, 0, NULL);

    WaitForSingleObject(readThread, INFINITE);

    CloseHandle(readThread);

    SetWindowText(hResult, res);

    // Освобождаем ресурсы
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