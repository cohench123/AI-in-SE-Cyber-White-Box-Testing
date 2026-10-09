#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <queue>
#include "config.h"
#include "region.h"
#include "residential.h"
#include "industrial.h"
#include "growth.h"
#include "commercial.h"
#include "goods.h"
using namespace std;

int numRows;
int numCols;
int changeCount = 0;
int twoSteps;
int population = 0;
int workers = 0;
int goods = 0;
int topRow, leftCol, bottomRow, rightCol; //For search function
string difficulty;
bool difficultyValid;
bool fileValid;

int main() {

cout << "Program Started." << endl;
Config config;
srand(time(0)); //Sets random by time

string configName;
string regionName;
vector<vector<pair<char, int>>> region;  //Main vector declaration
vector<vector<int>> pollution;
vector<vector<bool>> growth;
vector<pair<string, int>> goodsVector; //Vector that holds the name and demand/surplus for each good
vector<pair<string, int>> supplyVector;
queue<int> demandQueue;
queue<int> commercialQueue;

while(!fileValid) { //Filename validity check

    cout << "Please enter configuration file name: ";
    cin >> configName;  //Input for config filename

    ifstream file(configName); //opens config file

    if(!file.is_open()) {
        cout << "Invalid filename, please try again." << endl;
        fileValid = false;
    }
    else{
        fileValid = true;
    }

}

while (!difficultyValid) { //Difficulty choice

    cout << "(Easy = 1 Demand/Timestep , Medium = 2 Demand/Timestep , Hard = 3 Demand/Timestep)" << endl;
    cout << "Please choose a difficulty level for demand (Type Difficulty Name): ";
    cin >> difficulty;

    if (difficulty != "Easy" && difficulty != "Medium" && difficulty != "Hard") {
        cout << "Invalid difficulty selection, please try again." << endl;
        difficultyValid = false;
    }
    else{
        difficultyValid = true;
    }

}

config.initiateConfig(configName, regionName);  //Initiate's the config process based on input filename

regionFill(regionName, region);  //Fills main vector region with CSV values

goodsFill(goodsVector);
supplyFill(supplyVector);

numRows = region.size();  //Sets number of rows
numCols = region[0].size();  //Sets number of columns

growth.resize(numRows, vector<bool>(numCols, false)); //Resizes the growth and pollution vectors
pollution.resize(numRows, vector<int>(numCols, 0));

cout << "Refresh Rate: " << config.refreshRate << ", Time Limit: " << config.timeLimit << endl;
cout << "Region Size: " << numRows << " rows, " << numCols << " columns" << endl;
cout << "Difficulty: " << difficulty << endl;

for (int t = 0; t < config.timeLimit; t++) {  //Beginning of main loop iterated until max timestep
    for (int i = 0; i < numRows; i++) { //Inner Loop to iterate and check for growth parameters
        for (int j = 0; j < numCols; j++) {

            pair<char, int> p = region[i][j];
            
            if (p.first == 'R') { //Residential Condition

                if(residential(i, j, region) == true) {
                    growth[i][j] = true;
                    changeCount +=1;
                }

            }

            else if (p.first == 'I') {  //Industrial Condition

                if(industrial(region, pollution, i, j, workers, goods) == true) {
                    growth[i][j] = true;
                    changeCount +=1;
                }

            }

            else if (p.first == 'C') {  //Commercial Condition

                if(commercial(region, i, j, workers, goods) == true) {
                    growth[i][j] = true;
                    changeCount += 1;
                }

            }

        }

    }

    if (changeCount == 0) { //If no change is made, then iterate twoSteps
        twoSteps +=1;
    }
    if (changeCount >= 1) { //If change is made, reset twoSteps to zero
        twoSteps = 0;
    }
    if (twoSteps >= 2) { //If twoSteps is 2 or higher, stop simultaion
        cout << "2 consecutive timesteps without a change." << endl;
        cout << "Final Region State: " << endl;
        populationPrint(region);
        return 0;
    }

    if (difficulty == "Easy") {
        generateDemand(goodsVector, demandQueue);
    }
    if (difficulty == "Medium") {
        generateDemand(goodsVector, demandQueue);
        generateDemand(goodsVector, demandQueue);
    }
    if (difficulty == "Hard") {
        generateDemand(goodsVector, demandQueue);
        generateDemand(goodsVector, demandQueue);
        generateDemand(goodsVector, demandQueue);
    }
    
    if ((t % config.refreshRate) == 0 && t < config.timeLimit) { //If needed by refreshRate, run menu for user requested functionality

        int choice;
        int addOrRemove;
        int goodsNum;
        
        do {

            cout << endl;
            cout << "Please choose a functionality to perform" << endl << endl;
            cout << "1: Output current region state" << endl;
            cout << "2: Output current pollution state" << endl;
            cout << "3: Output specified area" << endl;
            cout << "4: Output total data for region" << endl;
            cout << "5: Add or remove demand" << endl;
            cout << "6: View current demand levels" << endl;
            cout << "7: View current supply levels" << endl;
            cout << "8: View current demand list" << endl;
            cout << "9: Continue simulation" << endl;
            cout << "10: End Simulation" << endl;
            cin >> choice;

            switch(choice) {

                case 1:

                    populationPrint(region); //Outputs current region state
                    break;

                case 2:

                    cout << "Current Pollution State: " << endl;
                    pollutionPrint(pollution);
                    break;

                case 3: {

                    cout << "Enter top-left corner of search area (row then column): ";
                    cin >> topRow >> leftCol;
                    cout << "Enter bottom-right corner of search area (row then column)";
                    cin >> bottomRow >> rightCol;

                    bool bounds = //Checks bounds of input values to ensure that values are within region and in correct ordering
                        topRow >= 0 && topRow < numRows &&
                        bottomRow >= 0 && bottomRow < numRows &&
                        leftCol >= 0 && leftCol < numCols &&
                        rightCol >= 0 && rightCol < numCols &&
                        topRow <= bottomRow && leftCol <= rightCol;

                    if (!bounds) {
                        cout << "Invalid input: coordinates are either out of bounds or incorrectly ordered." << endl;
                    }
                    else {
                        searchPrint(region, topRow, leftCol, bottomRow, rightCol);
                    }
                    break;
                }

                case 4:

                    dataPrint(region, population, t, workers, goods);
                    break;

                case 5:

                    cout << "1: Add demand" << endl;
                    cout << "2: Remove demand" << endl;
                    cin >> addOrRemove;

                    if (addOrRemove == 1) { //Add Demand
                        
                        goodsPrint(goodsVector);
                        cout << "Enter cooresponding number for demand to add: ";
                        cin >> goodsNum;
                        addDemand(goodsVector, goodsNum, demandQueue);
                        break;

                    }

                    else if (addOrRemove == 2) { //Remove Demand

                        goodsPrint(goodsVector);
                        cout << "Enter cooresponding number for demand to remove (Only for goods with demand of 1 or higher): ";
                        cin >> goodsNum;
                        if (goodsVector[goodsNum].second >= 1) {
                            removeDemand(goodsVector, goodsNum);
                        }
                        else if(goodsVector[goodsNum].second <= 0) { //Invalid Choice
                            cout << "Invalid good. Curent demand level does not exceed 0." << endl;
                        }
                    }

                    else if (addOrRemove > 2 || addOrRemove < 1) { //Invalid Choice
                        
                        cout << "Invalid choice to add or remove." << endl;
                        
                    }

                        break;

                case 6:

                    goodsPrint(goodsVector);
                    break;

                case 7:

                    supplyPrint(supplyVector);
                    break;

                case 8:

                    demandPrint(demandQueue);
                    break;

                case 9:

                    cout << "Continuing Simulation..." << endl;
                    break;

                case 10:

                    cout << "Ending Simulation. Thank you for playing SimCity" << endl;
                    return 0;
                    break;

                default:

                    cout << "Invalid menu choice. Please try again." << endl;
                    break;
                
            }

        } while (choice != 9 && choice != 10);


    }

    
    growthFunction(region, growth, population, workers, goods, pollution, goodsVector, demandQueue, commercialQueue, supplyVector);

    dataPrint(region, population, t, workers, goods);

}

    cout << "Final timestep reached." << endl;
    cout << "Final Region State: " << endl;
    populationPrint(region);
    return 0;

}
