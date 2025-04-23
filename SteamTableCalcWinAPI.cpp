#include <windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

using namespace std;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
float calculateH(const string& filename, float inputValue);
string to_string_precise(float val, int precision);

HWND hInputBox, hCalculateBtn, hResultLabel, hRadioTemp, hRadioPress;
bool isTemp = true;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"SteamTableWinAPI";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"Steam Table Calculator",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 250,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            CreateWindow(L"STATIC", L"Choose Input Type:",
                WS_VISIBLE | WS_CHILD, 20, 20, 150, 20, hwnd, nullptr, nullptr, nullptr);

            hRadioTemp = CreateWindow(L"BUTTON", L"Temperature",
                WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON | WS_GROUP,
                20, 45, 120, 20, hwnd, (HMENU)1, nullptr, nullptr);
            hRadioPress = CreateWindow(L"BUTTON", L"Pressure",
                WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
                150, 45, 120, 20, hwnd, (HMENU)2, nullptr, nullptr);

            SendMessage(hRadioTemp, BM_SETCHECK, BST_CHECKED, 0);

            hInputBox = CreateWindow(L"EDIT", nullptr,
                WS_VISIBLE | WS_CHILD | WS_BORDER,
                20, 80, 200, 25, hwnd, nullptr, nullptr, nullptr);

            hCalculateBtn = CreateWindow(L"BUTTON", L"Calculate",
                WS_VISIBLE | WS_CHILD,
                240, 80, 100, 25, hwnd, (HMENU)3, nullptr, nullptr);

            hResultLabel = CreateWindow(L"STATIC", L"Result will appear here",
                WS_VISIBLE | WS_CHILD,
                20, 130, 350, 25, hwnd, nullptr, nullptr, nullptr);
            break;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) isTemp = true;
            else if (LOWORD(wParam) == 2) isTemp = false;
            else if (LOWORD(wParam) == 3) {
                wchar_t buffer[256];
                GetWindowText(hInputBox, buffer, 256);
                try {
                    float inputVal = stof(wstring(buffer));
                    string filename = isTemp ? "A2.csv" : "A3.csv";
                    float h = calculateH(filename, inputVal);

                    if (h == -1)
                        SetWindowText(hResultLabel, L"Error: Value not found or file issue.");
                    else {
                        string result = "Calculated h = " + to_string_precise(h, 2);
                        wstring wresult(result.begin(), result.end());
                        SetWindowText(hResultLabel, wresult.c_str());
                    }
                } catch (...) {
                    SetWindowText(hResultLabel, L"Invalid input.");
                }
            }
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return (start == string::npos || end == string::npos) ? "" : str.substr(start, end - start + 1);
}

float calculateH(const string& filename, float inputValue) {
    ifstream file(filename);
    string line;
    bool found = false;
    float Hf = 0.0, Hfg = 0.0;

    if (!file.is_open()) return -1;

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string cell;
        vector<string> row;

        while (getline(ss, cell, ',')) row.push_back(trim(cell));

        if (row.size() >= 8) {
            try {
                float value = stof(row[0]);
                if (value == inputValue) {
                    Hf = stof(row[6]);
                    Hfg = stof(row[7]);
                    found = true;
                    break;
                }
            } catch (...) { return -1; }
        }
    }
    return found ? (Hf + Hfg) : -1;
}

string to_string_precise(float val, int precision) {
    ostringstream out;
    out.precision(precision);
    out << fixed << val;
    return out.str();
}
