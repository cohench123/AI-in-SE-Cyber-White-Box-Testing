#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <queue>
using namespace std;



void goodsFill(vector<pair<string, int>> &goodsVector) { //Populates the goods vector with names of goods and a starting demand/surplus that is even at 0. Positive numbers will indicate a demand for the good and negative numbers will indicate a surplus of the good

    goodsVector.push_back({"Toys", 0});
    goodsVector.push_back({"Clothing", 0});
    goodsVector.push_back({"Souvenirs", 0});
    goodsVector.push_back({"Food", 0});
    goodsVector.push_back({"Drink", 0});
    goodsVector.push_back({"Metalwork", 0});
    goodsVector.push_back({"Woodwork", 0});
    goodsVector.push_back({"Jewelry", 0});
    goodsVector.push_back({"Technology", 0});
    goodsVector.push_back({"Stationery", 0});
    goodsVector.push_back({"Vehicles", 0});
    goodsVector.push_back({"Weaponry", 0});
    goodsVector.push_back({"Tools", 0});
    goodsVector.push_back({"Decorations", 0});
    goodsVector.push_back({"Furniture", 0});
    goodsVector.push_back({"Plastic", 0});
    goodsVector.push_back({"Rubber", 0});
    goodsVector.push_back({"Medical Supplies", 0});
    goodsVector.push_back({"Chemicals", 0});
    goodsVector.push_back({"Fabrics", 0});

}

void supplyFill(vector<pair<string, int>> &supplyVector) {

    supplyVector.push_back({"Toys", 0});
    supplyVector.push_back({"Clothing", 0});
    supplyVector.push_back({"Souvenirs", 0});
    supplyVector.push_back({"Food", 0});
    supplyVector.push_back({"Drink", 0});
    supplyVector.push_back({"Metalwork", 0});
    supplyVector.push_back({"Woodwork", 0});
    supplyVector.push_back({"Jewelry", 0});
    supplyVector.push_back({"Technology", 0});
    supplyVector.push_back({"Stationery", 0});
    supplyVector.push_back({"Vehicles", 0});
    supplyVector.push_back({"Weaponry", 0});
    supplyVector.push_back({"Tools", 0});
    supplyVector.push_back({"Decorations", 0});
    supplyVector.push_back({"Furniture", 0});
    supplyVector.push_back({"Plastic", 0});
    supplyVector.push_back({"Rubber", 0});
    supplyVector.push_back({"Medical Supplies", 0});
    supplyVector.push_back({"Chemicals", 0});
    supplyVector.push_back({"Fabrics", 0});

}

void generateDemand(vector<pair<string, int>> &goodsVector, queue<int> &demandQueue) { //Adds demand to randomly generated good

    int randomNum = rand() % 20;
    goodsVector[randomNum].second += 1;
    demandQueue.push(randomNum);

}

void goodsPrint(vector<pair<string, int>> &goodsVector) {

    cout << "Current supply and demand levels: (Positive = Unmet Demand, Negative = Surplus, 0 = Current Demand Met)" << endl;
    for (int i = 0; i < 20; i++) {

        cout << goodsVector[i].first << ": " << i << ", Current demand level: " << goodsVector[i].second << endl;

    }

}

void addDemand(vector<pair<string, int>> &goodsVector, int goodsNum, queue<int> &demandQueue) {

    goodsVector[goodsNum].second += 1;
    demandQueue.push(goodsNum);
    cout << "Added 1 demand level to " << goodsVector[goodsNum].first << ". New demand level: " << goodsVector[goodsNum].second << endl;

}

void removeDemand(vector<pair<string, int>> &goodsVector, int goodsNum) {

    goodsVector[goodsNum].second -= 1;
    cout << "Removed 1 demand level from " << goodsVector[goodsNum].first << ". New demand level: " << goodsVector[goodsNum].second << endl;

}

void demandPrint (queue<int> &demandQueue) {

    cout << "Current demand queue: " << endl;
    queue<int> temp(demandQueue);

    while (!temp.empty()) {
        cout << temp.front() << " ";
        temp.pop();
    }
    cout << endl;

}

void supplyPrint (vector<pair<string, int>> &supplyVector) {

    cout << "Current supply levels of goods being sold in commercial cells: " << endl;
    for (int i = 0; i < 20; i++) {

        cout << supplyVector[i].first << ": " << i << ", Current supply level: " << supplyVector[i].second << endl;

    }

}