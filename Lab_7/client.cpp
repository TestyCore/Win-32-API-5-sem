#pragma comment(lib, "ws2_32.lib")

#include <Windows.h>
#include <string>
#include <thread>
#include <iostream>
#include <fstream>
#include <list>

using namespace std;

#define ID_BUTTON_UPLOAD 100


HMENU hMenu;
HHOOK hKeyboardHook = NULL;
HWND hActiveWindow, hListBox;
SOCKET clientSocket;


LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void AddMenus(HWND hWnd);
void AddControls(HWND hWnd);
void SendMessage(HWND hWnd);
void ConnectToServer(const char* serverIP);


LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* pKbStruct = (KBDLLHOOKSTRUCT*)lParam;

            if (pKbStruct->scanCode == 0x1C) {
                hActiveWindow = GetForegroundWindow();

                HWND hButton = GetDlgItem(hActiveWindow, ID_BUTTON_UPLOAD);

                if (hButton != NULL) {
                    SendMessage(hButton, BM_CLICK, 0, 0);
                }
            }
        }
    }

    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR arts, int ncmdshow) {
    WNDCLASSW wc = { 0 };

    wc.hbrBackground = CreateSolidBrush(RGB(255, 150, 255));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = L"Class";
    wc.lpfnWndProc = WindowProcedure;

    if (!RegisterClassW(&wc))
        return -1;

    CreateWindowW(L"Class", L"PhotoArchieve", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 450, 250, NULL, NULL, hInst, NULL);

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
        case ID_BUTTON_UPLOAD: {
            SendMessage(hWnd);
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
    hListBox = CreateWindowEx(0, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL, 30, 90, 370, 100, hWnd, NULL, NULL, NULL);

    CreateWindowEx(0, L"BUTTON", L"Upload", WS_CHILD | WS_VISIBLE, 30, 30, 370, 30, hWnd, reinterpret_cast<HMENU>(ID_BUTTON_UPLOAD), NULL, NULL);

    ConnectToServer("127.0.0.1");
}


void HandleMessages() {
    while (true) {
        char buffer[4096];
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead <= 0) {
            MessageBox(NULL, L"Error reading messages or server disconnected.", L"Error", MB_OK | MB_ICONERROR);
            closesocket(clientSocket);
            exit(1);
        }
        buffer[bytesRead] = '\0';
        char qwe[] = "Sent";
        SendMessageA(hListBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(buffer));
    }
}

void ConnectToServer(const char* serverIP) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(12345);

    connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    thread(HandleMessages).detach();
}


void SendMessage(HWND hWnd) {
    LPCWSTR title = L"Select Image";
    LPCWSTR initialDir = NULL;
    LPCWSTR filter = L"Image Files\0*.jpg;";
    WCHAR filename[MAX_PATH];
    filename[0] = '\0';

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrTitle = title;
    ofn.lpstrInitialDir = initialDir;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (!GetOpenFileName(&ofn)) {
        return;
    }

    std::ifstream imageFile(filename, std::ios::binary);
    if (!imageFile.is_open()) {
        MessageBox(NULL, L"Image does not exist!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    char buffer[600000];
    while (!imageFile.eof()) {
        imageFile.read(buffer, sizeof(buffer));
        int bytesRead = imageFile.gcount();
        send(clientSocket, buffer, bytesRead, 0);
    }

    imageFile.close();
}
