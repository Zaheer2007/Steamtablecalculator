#include <windows.h>
#include <string>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hEdit;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "SimpleGUIWindowClass";

    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                             
        CLASS_NAME,                   
        "Name Input GUI",             
        WS_OVERLAPPEDWINDOW,         

        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,

        NULL, NULL, hInstance, NULL  
    );

    if (hwnd == NULL)
        return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            CreateWindow("STATIC", "Enter your name:",
                WS_VISIBLE | WS_CHILD,
                20, 20, 200, 20,
                hwnd, NULL, NULL, NULL);

            hEdit = CreateWindow("EDIT", "",
                WS_VISIBLE | WS_CHILD | WS_BORDER,
                20, 50, 240, 25,
                hwnd, NULL, NULL, NULL);

            CreateWindow("BUTTON", "Say Hello",
                WS_VISIBLE | WS_CHILD,
                90, 90, 100, 30,
                hwnd, (HMENU) 1, NULL, NULL);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                char name[100];
                GetWindowText(hEdit, name, 100);

                std::string greeting = "Hello, ";
                greeting += name;
                greeting += "!";

                MessageBox(hwnd, greeting.c_str(), "Greeting", MB_OK);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}