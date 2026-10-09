#include <iostream>
#include "config.h"
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include "region.h"
using namespace std;

void regionFill(string regionName, vector<vector<pair<char, int>>> &region) {

    ifstream file(regionName);
    string currLine;

    region.clear();

    while (getline(file, currLine)) {
        vector<pair<char, int>> row;
        stringstream ss(currLine);
        string cell;

        while (getline(ss, cell, ',')) {
            if (!cell.empty()) {
                row.push_back(make_pair(cell[0], 0));
            }
            else {
                row.push_back(make_pair(' ', 0));
            }
        }

        region.push_back(row);
    }
    file.close();

    cout << "Initial Region State:" << endl;
    for (const auto &row : region) {
        for (const auto& p : row) {
            cout << p.first << " ";
        }
        cout << endl;
    }

}

void resetGrowth(vector<vector<bool>> &growth, int numRows, int numCols) {
    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            growth[i][j] = false;
        }
    }
}

void pollutionFill(vector<vector<int>> &pollution, int numRows, int numCols) {
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            pollution[i][j] = 0;
        }
    }
}

void populationPrint(vector<vector<pair<char, int>>> &region) {

    for (const auto &row : region) {
        for (const auto& p : row) {
            
            if (p.first == 'I' || p.first == 'C' || p.first == 'R') {
                if(p.second == 0) {
                    cout << p.first << " ";
                }
                else{
                    cout << p.second << " ";
                }
            }

            else {
                cout << p.first << " ";
            }
        }
        cout << endl;
    }

}

void dataPrint(vector<vector<pair<char, int>>> &region, int population, int t, int workers, int goods) {

    cout << "Current Region Data: " << endl;
    cout << "Current Timestep: " << t << endl;
    cout << "Current Population: " << population << endl;
    cout << "Available Workers: " << workers << endl;
    cout << "Available Goods: " << goods << endl;

}

void searchPrint(vector<vector<pair<char, int>>> &region, int topRow, int leftCol, int bottomRow, int rightCol) {

    for (int i = topRow; i <= bottomRow; i++) {
        for (int j = leftCol; j <= rightCol; j++) {
            
            if (region[i][j].first == 'I' || region[i][j].first == 'C' || region[i][j].first == 'R') {
                if (region[i][j].second > 0) {
                    cout << region[i][j].second << " ";
                }
                else {
                    cout << region[i][j].first << " ";
                }
            }
            else {
                cout << region[i][j].first << " ";
            }
        }
        cout << endl;
    }

}