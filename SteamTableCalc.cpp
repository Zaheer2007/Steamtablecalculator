#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <regex>
#include <cstdio>  // for snprintf

#define ID_COMBO_INPUT_TYPE 101
#define ID_EDIT_INPUT_VALUE 102
#define ID_EDIT_X_VALUE     105
#define ID_BUTTON_CALCULATE 103
#define ID_STATIC_RESULT    104

// Global control handles
HWND g_hComboInputType;
HWND g_hEditInputValue;
HWND g_hEditXValue;
HWND g_hButtonCalculate;
HWND g_hStaticResult;

// File paths
const char* FILE_TEMP = "A2.csv";
const char* FILE_PRESS = "A3.csv";

// Data structure optimized for fast lookup
struct DataEntry {
    float hf;
    float hg;
};

// Use hash maps for O(1) lookup instead of linear search
static std::unordered_map<float, DataEntry> g_tempDataMap;
static std::unordered_map<float, DataEntry> g_presDataMap;

// Pre-compiled regex for number validation (compile once, use many times)
static const std::regex g_floatRegex(R"([-+]?[0-9]*\.?[0-9]+)");

// Efficient string trimming function
inline std::string trim(const std::string& str) {
    const char* whitespace = " \t\r\n";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

bool loadData(const char* filename, std::unordered_map<float, DataEntry>& container) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    // Skip header line
    std::getline(file, line);
    
    // Reserve space to avoid rehashing
    container.reserve(100);  // Assume about 100 entries
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        // Avoid stringstream which has overhead
        size_t pos = 0, nextPos = 0;
        std::vector<std::string> row;
        row.reserve(8);  // We know we need 8 columns
        
        // Manual split is faster than getline with stringstream
        while ((nextPos = line.find(',', pos)) != std::string::npos) {
            row.push_back(trim(line.substr(pos, nextPos - pos)));
            pos = nextPos + 1;
        }
        row.push_back(trim(line.substr(pos)));
        
        if (row.size() >= 8) {
            try {
                float key = std::stof(row[0]);
                float hf = std::stof(row[6]);
                float hg = std::stof(row[7]);
                container[key] = {hf, hg};  // O(1) insertion
            } catch (...) {
                // ignore malformed lines
            }
        }
    }
    return true;
}

float calculateH(const std::unordered_map<float, DataEntry>& data, float inputValue, float x) {
    auto it = data.find(inputValue);  // O(1) lookup
    if (it != data.end()) {
        return it->second.hf + x * (it->second.hg - it->second.hf);
    }
    return -1.0f;
}

bool validateInput(const char* input, float& value) {
    std::string str = trim(input);
    if (!std::regex_match(str, g_floatRegex)) {
        return false;
    }
    value = std::stof(str);
    return true;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Create UI controls
            CreateWindow("STATIC", "Input Type:", WS_VISIBLE | WS_CHILD,
                20, 10, 120, 20, hwnd, NULL, NULL, NULL);

            g_hComboInputType = CreateWindow("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
                150, 10, 160, 120, hwnd, (HMENU)ID_COMBO_INPUT_TYPE, NULL, NULL);
            SendMessage(g_hComboInputType, CB_ADDSTRING, 0, (LPARAM)"Temperature (°C)");
            SendMessage(g_hComboInputType, CB_ADDSTRING, 0, (LPARAM)"Pressure (bar)");
            SendMessage(g_hComboInputType, CB_SETCURSEL, 0, 0);

            CreateWindow("STATIC", "Enter Value:", WS_VISIBLE | WS_CHILD,
                20, 45, 120, 20, hwnd, NULL, NULL, NULL);
            g_hEditInputValue = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                150, 45, 160, 25, hwnd, (HMENU)ID_EDIT_INPUT_VALUE, NULL, NULL);

            CreateWindow("STATIC", "Enter x (0-1):", WS_VISIBLE | WS_CHILD,
                20, 80, 120, 20, hwnd, NULL, NULL, NULL);
            g_hEditXValue = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                150, 80, 160, 25, hwnd, (HMENU)ID_EDIT_X_VALUE, NULL, NULL);

            g_hButtonCalculate = CreateWindow("BUTTON", "Calculate", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                20, 120, 290, 30, hwnd, (HMENU)ID_BUTTON_CALCULATE, NULL, NULL);

            g_hStaticResult = CreateWindow("STATIC", "Result: ", WS_VISIBLE | WS_CHILD,
                20, 165, 290, 25, hwnd, (HMENU)ID_STATIC_RESULT, NULL, NULL);

            // Load data files
            if (!loadData(FILE_TEMP, g_tempDataMap)) {
                MessageBox(hwnd, "Failed to load temperature data.", "Error", MB_ICONERROR);
            }
            if (!loadData(FILE_PRESS, g_presDataMap)) {
                MessageBox(hwnd, "Failed to load pressure data.", "Error", MB_ICONERROR);
            }
            break;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BUTTON_CALCULATE) {
                char bufVal[64];
                GetWindowText(g_hEditInputValue, bufVal, sizeof(bufVal));
                
                float inputVal;
                if (!validateInput(bufVal, inputVal)) {
                    SetWindowText(g_hStaticResult, "Result: Invalid input value");
                    return 0;
                }

                char bufX[64];
                GetWindowText(g_hEditXValue, bufX, sizeof(bufX));
                
                float x;
                if (!validateInput(bufX, x)) {
                    SetWindowText(g_hStaticResult, "Result: Invalid x value");
                    return 0;
                }
                
                if (x < 0.0f || x > 1.0f) {
                    SetWindowText(g_hStaticResult, "Result: x must be between 0 and 1");
                    return 0;
                }

                int idx = SendMessage(g_hComboInputType, CB_GETCURSEL, 0, 0);
                const auto& data = (idx == 0) ? g_tempDataMap : g_presDataMap;

                float h = calculateH(data, inputVal, x);
                if (h < 0.0f) {
                    SetWindowText(g_hStaticResult, "Result: Calculation failed");
                } else {
                    char resBuf[64];
                    snprintf(resBuf, sizeof(resBuf), "Result: h = %.3f", h);
                    SetWindowText(g_hStaticResult, resBuf);
                }
            }
            break;
        }

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "SteamCalculatorWindow";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wc)) return 0;

    HWND hwnd = CreateWindow(
        CLASS_NAME, 
        "Steam Table Calculator", 
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 250, 
        NULL, NULL, hInstance, NULL
    );
    
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