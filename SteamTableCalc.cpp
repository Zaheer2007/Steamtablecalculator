#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

// Define control IDs
#define ID_COMBO_INPUT_TYPE 101
#define ID_EDIT_INPUT_VALUE 102
#define ID_BUTTON_CALCULATE 103
#define ID_STATIC_RESULT 104

// Global handles
HWND g_hComboInputType;
HWND g_hEditInputValue;
HWND g_hButtonCalculate;
HWND g_hStaticResult;

using namespace std;

// Function to trim whitespace
string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return (start == string::npos || end == string::npos) ? "" : str.substr(start, end - start + 1);
}

// Perform calculation
float calculateH(const string& filename, float inputValue) {
    ifstream file(filename);
    string line;
    bool found = false;
    float Hf = 0.0f, Hfg = 0.0f;

    if (!file.is_open()) {
        return -1;
    }

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string cell;
        vector<string> row;

        while (getline(ss, cell, ',')) {
            row.push_back(trim(cell));
        }

        if (all_of(row.begin(), row.end(), [](const string& s) { return s.empty(); })) continue;

        if (row.size() >= 8) {
            try {
                float value = stof(row[0]);
                if (value == inputValue) {
                    Hf = stof(row[6]);
                    Hfg = stof(row[7]);
                    found = true;
                    break;
                }
            } catch (...) {
                return -1;
            }
        }
    }

    return found ? Hf + Hfg : -1;
}

// Main Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            g_hComboInputType = CreateWindow("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
                20, 20, 150, 200, hwnd, (HMENU)ID_COMBO_INPUT_TYPE, NULL, NULL);
            SendMessage(g_hComboInputType, CB_ADDSTRING, 0, (LPARAM)"Temperature");
            SendMessage(g_hComboInputType, CB_ADDSTRING, 0, (LPARAM)"Pressure");
            SendMessage(g_hComboInputType, CB_SETCURSEL, 0, 0);

            g_hEditInputValue = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                20, 60, 150, 25, hwnd, (HMENU)ID_EDIT_INPUT_VALUE, NULL, NULL);

            g_hButtonCalculate = CreateWindow("BUTTON", "Calculate", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                20, 100, 150, 30, hwnd, (HMENU)ID_BUTTON_CALCULATE, NULL, NULL);

            g_hStaticResult = CreateWindow("STATIC", "Result: ", WS_VISIBLE | WS_CHILD,
                20, 150, 250, 25, hwnd, (HMENU)ID_STATIC_RESULT, NULL, NULL);

            CreateWindow("STATIC", "Input Type:", WS_VISIBLE | WS_CHILD,
                20, 5, 100, 20, hwnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Enter Value:", WS_VISIBLE | WS_CHILD,
                20, 45, 100, 20, hwnd, NULL, NULL, NULL);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BUTTON_CALCULATE) {
                int selectedIndex = SendMessage(g_hComboInputType, CB_GETCURSEL, 0, 0);
                string filename = (selectedIndex == 0) ? "A2.csv" : "A3.csv";

                char buffer[256];
                GetWindowText(g_hEditInputValue, buffer, 256);

                try {
                    float inputVal = stof(buffer);
                    float h = calculateH(filename, inputVal);

                    string result = (h != -1)
                        ? "Result: Calculated h = " + to_string(h)
                        : "Result: Calculation failed";
                    SetWindowText(g_hStaticResult, result.c_str());
                } catch (...) {
                    SetWindowText(g_hStaticResult, "Result: Invalid input");
                }
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

// Entry Point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "SteamCalculatorWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(CLASS_NAME, "Steam Property Calculator", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 250, NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
