#pragma comment(lib, "ws2_32.lib")

#include <windows.h>
#include <regex>
#include <cstdlib>
#include <vector>
#include <string>

HMENU hMenu;
HWND hEdit;
SOCKET serverSocket;
SOCKET clientSocket;
wchar_t receivedMessage[100];

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void AddMenus(HWND hWnd);
void AddControls(HWND hWnd);

bool InitSocket();
void StartServer();
void StartClient();
void SendMessageToServer(const wchar_t* message);
void SendMessageToClient(const wchar_t* message);



int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR arts, int ncmdshow) {

	if (!InitSocket()) {
		MessageBox(NULL, L"Failed to initialize sockets.", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}

    StartServer();

	WNDCLASSW wc = { 0 };

	wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInst;
	wc.lpszClassName = L"Class";
	wc.lpfnWndProc = WindowProcedure;

	if (!RegisterClassW(&wc))
		return -1;

	CreateWindowW(L"Class", L"Lab_1_2", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 20, 20, 1800, 500, NULL, NULL, NULL, NULL);

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    closesocket(serverSocket);
    closesocket(clientSocket);

    WSACleanup();

	return 0;
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg)
	{
	case WM_COMMAND:
		switch (wp)
		{
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
}


bool InitSocket() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		MessageBox(NULL, L"Failed to initialize Winsock.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}


void StartServer() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        MessageBox(NULL, L"Failed to create server socket.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        MessageBox(NULL, L"Failed to bind server socket.", L"Error", MB_OK | MB_ICONERROR);
        closesocket(serverSocket);
        return;
    }

    listen(serverSocket, 1);

    SOCKET client = accept(serverSocket, NULL, NULL);
    if (client == INVALID_SOCKET) {
        MessageBox(NULL, L"Failed to accept client connection.", L"Error", MB_OK | MB_ICONERROR);
        closesocket(serverSocket);
        return;
    }

    SendMessageToClient(L"������, ������!");

    while (true) {
        wchar_t buffer[100];
        recv(client, (char*)buffer, sizeof(buffer), 0);
        SetWindowText(hEdit, buffer);
    }
}

void StartClient() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        MessageBox(NULL, L"Failed to create client socket.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        MessageBox(NULL, L"Failed to connect to the server.", L"Error", MB_OK | MB_ICONERROR);
        closesocket(clientSocket);
        return;
    }

    recv(clientSocket, (char*)receivedMessage, sizeof(receivedMessage), 0);
    SetWindowText(hEdit, receivedMessage);

    while (true) {
        wchar_t buffer[100];
        recv(clientSocket, (char*)buffer, sizeof(buffer), 0);
        SetWindowText(hEdit, buffer);
    }
}

void SendMessageToServer(const wchar_t* message) {
    send(clientSocket, (const char*)message, sizeof(wchar_t) * wcslen(message), 0);
}

void SendMessageToClient(const wchar_t* message) {
    send(serverSocket, (const char*)message, sizeof(wchar_t) * wcslen(message), 0);
}