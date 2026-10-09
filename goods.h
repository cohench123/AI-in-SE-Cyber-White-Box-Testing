#ifndef GOODS_H
#define GOODS_H
#include <iostream>
#include <vector>
#include <queue>
using namespace std;

void goodsFill(vector<pair<string, int>> &goodsVector);
void supplyFill(vector<pair<string, int>> &supplyVector);
void generateDemand(vector<pair<string, int>> &goodsVector, queue<int> &demandQueue);
void goodsPrint(vector<pair<string, int>> &goodsVector);
void addDemand(vector<pair<string, int>> &goodsVector, int goodsNum, queue<int> &demandQueue);
void removeDemand(vector<pair<string, int>> &goodsVector, int goodsNum);
void demandPrint (queue<int> &demandQueue);
void supplyPrint (vector<pair<string, int>> &supplyVector);

#endif