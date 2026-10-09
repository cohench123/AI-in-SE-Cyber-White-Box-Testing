#ifndef GROWTH_H
#define GROWTH_H
#include <iostream>
#include <vector>
#include <queue>
using namespace std;

void growthFunction(vector<vector<pair<char, int>>> &region, vector<vector<bool>> &growth, int &population, int &workers, int &goods, vector<vector<int>> &pollution, vector<pair<string, int>> &goodsVector, queue<int> &demandQueue, queue<int> &commercialQueue, vector<pair<string, int>> &supplyVector);

void pollutionPrint(const vector<vector<int>> &pollution);

#endif

