#include "growth.h"
#include <iostream>
#include <algorithm>
#include <queue>
#include <tuple>
#include <ctime>
#include <cstdlib>
using namespace std;

struct Growth{ //Struct that holds information needed for growth function

    int row, col;
    char cellType;
    int currentPop;
    int adjacentPop;

    Growth(int r, int c, char t, int pop, int adj) : row(r), col(c), currentPop(pop), adjacentPop(adj) {}

};

struct GrowthUpdate { //Struct to hold values of cells that need to be grown post-iteration
    
    int row, col;
    int popChange;
    int workerChange;
    int goodsChange;

    GrowthUpdate(int r, int c, int pop, int workers, int goods)
        : row(r), col(c), popChange(pop), workerChange(workers), goodsChange(goods) {}
};

struct PollutionCell {
    int row, col, level;
    PollutionCell(int r, int c, int l) : row(r), col(c), level(l) {}
};


int getNearbyPopulation(const vector<vector<pair<char, int>>> &region, int row, int col) {
    int nearbyPop = 0;
    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},  
        {0, -1},           {0, 1},   
        {1, -1},  {1, 0},  {1, 1}   
    };

    int numRows = region.size();
    int numCols = region[0].size();

    for (const auto& dir : directions) {
        int newRow = row + dir[0];
        int newCol = col + dir[1];
        if (newRow >= 0 && newRow < numRows && newCol >= 0 && newCol < numCols) {
            nearbyPop += region[newRow][newCol].second;
        }
    }

    return nearbyPop;
}

void spreadPollution(const vector<vector<pair<char, int>>> &region, vector<vector<int>> &pollution, int originRow, int originCol) { //Pollution spread function

    int numRows = region.size();
    int numCols = region[0].size();
    int sourcePop = region[originRow][originCol].second;
    vector<vector<bool>> visited(numRows, vector<bool>(numCols, false));
    queue<PollutionCell> q;

    q.emplace(originRow, originCol, sourcePop);
    visited[originRow][originCol] = true;

    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    while (!q.empty()) {
        PollutionCell cell = q.front();
        q.pop();

        pollution[cell.row][cell.col] += cell.level;

        if(cell.level > 1) {
            for (const auto& dir : directions) {
                int newRow = cell.row + dir[0];
                int newCol = cell.col + dir[1];
                if (newRow >= 0 && newRow < numRows && newCol >= 0 && newCol < numCols && !visited[newRow][newCol]) {
                    visited[newRow][newCol] = true;
                    q.emplace(newRow, newCol, cell.level - 1);
                }
            }
        }
    }

}

void pollutionPrint(const vector<vector<int>> &pollution) {

    for (const auto& row : pollution) {
        for (int value : row) {
            cout << value << " ";
        }
        cout << endl;
    }
}

void growthFunction(vector<vector<pair<char, int>>> &region, vector<vector<bool>> &growth, int &population, int &workers, int &goods, vector<vector<int>> &pollution, vector<pair<string, int>> &goodsVector, queue<int> &demandQueue, queue<int> &commercialQueue, vector<pair<string, int>> &supplyVector) {
    
    vector<Growth> possibleGrowth;

    for (int i = 0; i < region.size(); ++i) { //Collects all cells for possible growth into one vector
        for (int j =0; j < region[0].size(); ++j) {
            if (growth[i][j]) {
                char zoneType = region[i][j].first;
                int pop = region[i][j].second;
                int nearbyPop = getNearbyPopulation(region, i, j);
                possibleGrowth.emplace_back(i, j, zoneType, pop, nearbyPop);
            }
        }
    }

    sort(possibleGrowth.begin(), possibleGrowth.end(), [](const Growth &a, const Growth &b) { //Sorts vector of possible growth by specified rules
        if (a.cellType != b.cellType) {
            if (a.cellType == 'C') return true;
            if (b.cellType == 'C') return false;
            if (a.cellType == 'I') return true;
            return false;
        }
        if (a.currentPop != b.currentPop) return a.currentPop > b.currentPop;
        if (a.adjacentPop != b.adjacentPop) return a.adjacentPop > b.adjacentPop;
        if (a.row != b.row) return a.row < b.row;
        return a.col < b.col;
    });

    vector<GrowthUpdate> plannedAdditions; //Vector to store growth cells to execute any additions after iteration as to not create artificial growth within other cells

    for (const auto& spot : possibleGrowth) { //Iterate through each growth spot
        int i = spot.row;
        int j = spot.col;
        char& cellType = region[i][j].first;

        if (cellType == 'R' && growth[i][j]) { //Residential grow
            plannedAdditions.emplace_back(i, j, 1, 1, 0);
            population += 1;
            growth[i][j] = false;
        }
        else if (cellType == 'I' && workers >= 2 && growth[i][j]) { //Industrial grow
            plannedAdditions.emplace_back(i, j, 1, 0, 1);
            workers -= 2;
            growth[i][j] = false;
            
            if (demandQueue.empty()) { //No current demand = Random good produced

                int randomNum = rand() % 20;
                goodsVector[randomNum].second -= 1;
                commercialQueue.push(randomNum);
                supplyVector[randomNum].second += 1;

            }

            else if (!demandQueue.empty()) { //Current Demand = First demand in queue is selected;

                int tempDemand;
                tempDemand = demandQueue.front();
                demandQueue.pop();
                goodsVector[tempDemand].second -= 1;
                commercialQueue.push(tempDemand);
                supplyVector[tempDemand].second += 1;

            }
        }
        else if (cellType == 'C' && workers >= 1 && goods >= 1 && growth[i][j]) { //Commercial grow
            plannedAdditions.emplace_back(i, j, 1, 0, 0);
            workers -= 1;
            goods -= 1;
            growth[i][j] = false;

            if (!commercialQueue.empty()) { //Choose good that was least recently produced to be sold and added to official supply

                int tempCommercial;
                tempCommercial = commercialQueue.front();
                commercialQueue.pop();

            }

        }

    }

    for (const auto& update : plannedAdditions) {

        region[update.row][update.col].second += update.popChange;
        workers += update.workerChange;
        goods += update.goodsChange;

    }

    for (auto& row : pollution) { //Resets pollution map before repopulation
        fill(row.begin(), row.end(), 0);
    }

    for (int i = 0; i < region.size(); ++i) {
        for(int j = 0; j < region[0].size(); ++j) {
            if (region[i][j].first == 'I' && region[i][j].second > 0) {
                spreadPollution(region, pollution, i, j);
            }
        }
    }

}
