#include <windows.h>
#include <cstdlib>

#define ID_FILE_WRITE 100

HMENU hMenu;
HWND hResult, hSource, hWriteRes;

int FileWrite(HWND hWnd);
void AddControls(HWND hWnd);
void AddMenus(HWND hWnd);
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);


void CALLBACK WriteCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {

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

	CreateWindowW(L"Class", L"Lab_3", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 20, 20, 500, 400, NULL, NULL, NULL, NULL);


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
			FileWrite(hWnd);
			InvalidateRect(hWnd, NULL, TRUE);
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

	CreateWindowW(L"Static", L"Sales:", WS_VISIBLE | WS_CHILD, 20, 50, 58, 38, hWnd, NULL, NULL, NULL);

	CreateWindowW(L"Button", L"Remove record", WS_VISIBLE | WS_CHILD, 100, 100, 100, 30, hWnd, (HMENU)ID_FILE_WRITE, NULL, NULL);
    hSource = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 170, 280, 20, hWnd, NULL, NULL, NULL);
    hResult = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 240, 280, 20, hWnd, NULL, NULL, NULL);

    hWriteRes = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 200, 280, 20, hWnd, NULL, NULL, NULL);
}

int FileWrite(HWND hWnd) {

    // ��� ����� ��� ������
    LPCWSTR  filename = L"mapped_file.txt";

    // ������� � ��������� ���� ��� ������ � ��������� UTF-16 (Unicode)
    HANDLE fileHandle = CreateFile(
        filename,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL
    );

    // ������ ����� � ������
    wchar_t data[100];
    GetWindowText(hSource, data, 100);
    int numBytesToWrite = (wcslen(data) + 1) * sizeof(wchar_t);

    DWORD fileSize = numBytesToWrite;
    // ��������� ������� �����
    SetFilePointer(fileHandle, fileSize, NULL, FILE_BEGIN);
    SetEndOfFile(fileHandle);

    // ����������� ����� � ������
    HANDLE fileMapping = CreateFileMapping(
        fileHandle,
        NULL,
        PAGE_READWRITE,
        0,
        fileSize,
        NULL
    );

    // �������� ��������� �� ������������ ����
    LPVOID filePointer = MapViewOfFile(fileMapping, FILE_MAP_WRITE, 0, 0, 0);


    //OVERLAPPED writeOverlapped = { 0 };
    //writeOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // ������� ������� ��� ����������� ������
    //WriteFileEx(filePointer, data, numBytesToWrite, &writeOverlapped, WriteCompletionRoutine);


   /* DWORD bytesTransferred;
    if (GetOverlappedResult(fileHandle, &writeOverlapped, &bytesTransferred, TRUE)) {
        SetWindowText(hWriteRes, L"Success!");

        return 1;
    }*/

    // ���������� ������ � ������������ ����
    CopyMemory(filePointer, data, numBytesToWrite);
   /* LPCWSTR Data = data;
    *(LPCWSTR*)(filePointer) = Data;
    LPCWSTR arr = *static_cast<LPCWSTR*>(filePointer);*/

    wchar_t* Buffer = new wchar_t[numBytesToWrite / sizeof(wchar_t)];


    //DWORD bytesRead;
    //ReadFile(filePointer, Buffer, numBytesToWrite, &bytesRead, NULL);


    CopyMemory(Buffer, filePointer, numBytesToWrite);
    wchar_t res[100];
    wcscpy_s(res, Buffer);
    wcscat_s(res, L"\0");
    SetWindowText(hResult, res);

    // ����������� �������
    UnmapViewOfFile(filePointer);
    CloseHandle(fileMapping);
    CloseHandle(fileHandle);

    return 0;
}