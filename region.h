#ifndef REGION_H
#define REGION_H

#include <iostream>
#include <vector>
#include <utility>
using namespace std;

void regionFill(string regionName, vector<vector<pair<char, int>>> &region);

void resetGrowth(vector<vector<bool>> &growth, int numRows, int numCols);

void pollutionFill(vector<vector<int>> &pollution, int numRows, int numCols);

void populationPrint(vector<vector<pair<char, int>>> &region);

void dataPrint(vector<vector<pair<char, int>>> &region, int population, int t, int workers, int goods);

void searchPrint(vector<vector<pair<char, int>>> &region, int topRow, int leftCol, int bottomRow, int rightCol);


#endif