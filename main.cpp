#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <algorithm>

using namespace std;

// Function to trim leading and trailing spaces from a string
string trim(const string &str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return (start == string::npos || end == string::npos) ? "" : str.substr(start, end - start + 1);
}

float calculateH(const string& filename, float inputValue) {
    ifstream file(filename);
    string line;
    bool found = false;
    float Hf = 0.0, Hfg = 0.0;

    if (!file.is_open()) {
        cerr << "Error: Unable to open file " << filename << endl;
        return -1;
    }

    while (getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        stringstream ss(line);
        string cell;
        vector<string> row;

        while (getline(ss, cell, ',')) {
            row.push_back(trim(cell));  // Trim spaces before storing
        }

        // Skip rows that are completely empty (only commas or spaces)
        if (all_of(row.begin(), row.end(), [](const string& s) { return s.empty(); })) {
            continue;
        }

        // Ensure at least 8 columns (index 0 to 7)
        if (row.size() >= 8) {
            try {
                float value = stof(row[0]); // Temperature or Pressure
                if (value == inputValue) {
                    Hf = stof(row[6]);   // 7th column
                    Hfg = stof(row[7]);  // 8th column
                    found = true;
                    break;
                }
            } catch (const invalid_argument& e) {
                cerr << "Error: Invalid number format in file!" << endl;
                return -1;
            } catch (const out_of_range& e) {
                cerr << "Error: Number out of range in file!" << endl;
                return -1;
            }
        }
    }

    if (!found) {
        cerr << "Value not found in file!" << endl;
        return -1;
    }

    return Hf + Hfg;
}

int main() {
    int choice;
    float inputVal;
    string filename;

    cout << "Choose input type:" << endl;
    cout << "1. Temperature\n2. Pressure" << endl;
    cin >> choice;

    if (choice == 1) {
        filename = "A2.csv";
        cout << "Enter temperature value: ";
    } else if (choice == 2) {
        filename = "A3.csv";
        cout << "Enter pressure value: ";
    } else {
        cerr << "Invalid choice!" << endl;
        return 1;
    }

    cin >> inputVal;
    float h = calculateH(filename, inputVal);

    if (h != -1)
        cout << "Calculated h = " << h << endl;

    return 0;
}
