#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <limits>
#include <iomanip>

using namespace std;

// Function to parse the CSV file and return the data as a vector of vectors
vector<vector<float>> parseCSV(const string& filename) {
    ifstream file(filename);
    vector<vector<float>> data;
    
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        vector<float> row;
        string value;
        while (getline(ss, value, ',')) {
            try {
                row.push_back(stof(value));
            } catch (const invalid_argument& e) {
                // Handle invalid number format error
                continue;
            }
        }
        if (!row.empty()) {
            data.push_back(row);
        }
    }
    return data;
}

// Function to calculate h = Hf + Hfg based on the provided pressure value
float calculateH(const vector<vector<float>>& data, float pressure) {
    for (const auto& row : data) {
        if (row.size() >= 11) {
            if (row[0] == pressure) {
                float Hf = row[5];
                float Hfg = row[9];
                return Hf + Hfg;
            }
        }
    }
    return numeric_limits<float>::quiet_NaN();  // Return NaN if no matching pressure is found
}

int main() {
    // Load the CSV data
    string fileName = "A3.csv";
    vector<vector<float>> data = parseCSV(fileName);

    // Choose input type (temperature or pressure)
    cout << "Choose input type:\n1. Temperature\n2. Pressure\n";
    int choice;
    cin >> choice;

    // Process based on user input
    if (choice == 2) {  // Pressure
        float pressure;
        cout << "Enter pressure value: ";
        cin >> pressure;

        // Calculate h for the entered pressure
        float h = calculateH(data, pressure);
        if (h != h) {  // Check for NaN
            cout << "Error: Invalid number format in file!" << endl;
        } else {
            cout << "Calculated h = " << fixed << setprecision(2) << h << endl;
        }
    }

    return 0;
}
